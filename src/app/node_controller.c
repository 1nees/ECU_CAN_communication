#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "can_driver.h"
#include "signals.h"

#define ID_HEARTBEAT   0x7FF
#define ID_CMD         0x200
#define CMD_PERIOD_MS  50
#define HB_TIMEOUT_MS  300

static long now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000L + t.tv_nsec / 1000000L;
}

static void send_cmd(uint16_t target, int enable)
{
    can_frame_t f = { .id = ID_CMD, .dlc = 4 };
    f.data[0] = target & 0xFF;
    f.data[1] = (target >> 8) & 0xFF;
    f.data[2] = enable ? 1 : 0;
    f.data[3] = f.data[0] ^ f.data[1] ^ f.data[2];   
    can_send(&f);
}

int main(void)
{
    if (can_init("vcan0") != 0) {
        printf("can_init failed\n");
        return 1;
    }

    int safe_state = 1;         
    long last_hb = 0;
    long next_cmd = now_ms();
    float temp = 25.0f;
    int last_counter = -1;
    unsigned long bad_frames = 0, lost_frames = 0;

    printf("Node B started, waiting for Node A...\n");

    while (1) {
        can_frame_t f;
        int r = can_receive_timeout(&f, 5);
        if (r < 0) {
            break;
        }

        if (r == 0) {
            if (f.id == ID_HEARTBEAT && f.dlc == 2) {
                last_hb = now_ms();
                if (safe_state) {
                    safe_state = 0;
                    printf("Node A alive -> NORMAL operation\n");
                }
                if (f.data[1] == 0) {  
                    printf("temp=%.1f C, bad=%lu, lost=%lu\n", temp, bad_frames, lost_frames);
                }
            } else if (f.id == ID_SENSOR_DATA) {
                sensor_data_t d;
                if (sensor_data_unpack(&f, &d) != 0) {
                    bad_frames++;
                    printf("Bad frame (checksum), dropped\n");
                } else {
                    if (last_counter >= 0 && d.alive_counter != ((last_counter + 1) & 0x0F)) {
                        lost_frames++;
                        printf("Alive counter jump: %d -> %d\n", last_counter, d.alive_counter);
                    }
                    last_counter = d.alive_counter;
                    temp = d.temperature;
                }
            }
        }

        long t = now_ms();

        if (!safe_state && (t - last_hb) > HB_TIMEOUT_MS) {
            safe_state = 1;
            printf("HEARTBEAT TIMEOUT (> %d ms) -> SAFE STATE\n", HB_TIMEOUT_MS);
        }

        if (t >= next_cmd) {
            if (safe_state) {
                send_cmd(0, 0);                     
            } else {
                uint16_t target = (temp > 0.0f) ? (uint16_t)(temp * 10.0f) : 0;
                send_cmd(target, temp > 25.0f);      
            }
            next_cmd += CMD_PERIOD_MS;
        }
    }

    can_close();
    return 0;
}