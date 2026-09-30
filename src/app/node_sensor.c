#include <stdio.h>
#include <math.h>
#include <time.h>
#include "can_driver.h"
#include "signals.h"

#define ID_HEARTBEAT   0x7FF
#define NODE_ID_A      1
#define PERIOD_MS      10      
#define HEARTBEAT_EVERY 10     

static void add_ms(struct timespec *t, long ms)
{
    t->tv_nsec += ms * 1000000L;
    while (t->tv_nsec >= 1000000000L) {
        t->tv_nsec -= 1000000000L;
        t->tv_sec += 1;
    }
}

int main(void)
{
    if (can_init("vcan0") != 0) {
        printf("can_init failed\n");
        return 1;
    }

    uint8_t sensor_counter = 0;
    uint8_t hb_counter = 0;
    unsigned long tick = 0;

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    printf("Node A started (Ctrl+C to stop)\n");

    while (1) {
        
        float t = (float)tick * PERIOD_MS / 1000.0f;
        sensor_data_t data = {
            .temperature = 25.0f + 5.0f * sinf(t * 0.5f),
            .alive_counter = sensor_counter
        };

        can_frame_t frame;
        sensor_data_pack(&data, &frame);
        can_send(&frame);
        sensor_counter = (sensor_counter + 1) & 0x0F;  

        
        if (tick % HEARTBEAT_EVERY == 0) {
            can_frame_t hb = { .id = ID_HEARTBEAT, .dlc = 2 };
            hb.data[0] = NODE_ID_A;
            hb.data[1] = hb_counter;
            can_send(&hb);
            hb_counter = (hb_counter + 1) & 0x0F;
        }

        tick++;

      
        add_ms(&next, PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }

    can_close();
    return 0;
}