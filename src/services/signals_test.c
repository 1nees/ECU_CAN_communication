#include <stdio.h>
#include "signals.h"

int main(void){

    sensor_data_t in = { .temperature = 23.5f, .alive_counter = 3 };
    can_frame_t frame;
    sensor_data_t out;

    sensor_data_pack(&in, &frame);
    printf("Bajtovi: %02X %02X %02X %02X\n", frame.data[0], frame.data[1], frame.data[2], frame.data[3]);
    
    if(sensor_data_unpack(&frame, &out) == 0){
        printf("OK: temp=%.1f counter =%d\n", out.temperature, out.alive_counter);
    }

    frame.data[0] ^= 0x01;
    if(sensor_data_unpack(&frame, &out) != 0){
        printf("Pokvaren frejm je odbijen (cs)\n");
    }

    return 0;
}