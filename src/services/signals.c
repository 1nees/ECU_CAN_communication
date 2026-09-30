#include "signals.h"

static uint8_t calc_checksum(const uint8_t *data, int len) {
    uint8_t cs = 0;
    
    for (int i=0; i<len; i++) {
        cs ^= data[i];
    }
    return cs;
}

void sensor_data_pack(const sensor_data_t *in, can_frame_t *out) {

    float scaled = in->temperature * 10.0f;
    int16_t raw = (int16_t)(scaled >= 0 ? scaled + 0.5f : scaled - 0.5f); 

    out->id = ID_SENSOR_DATA;
    out->dlc = 4;
    out->data[0] = (uint8_t)(raw & 0xFF);
    out->data[1] = (uint8_t)((raw >> 8) & 0xFF);
    out->data[2] = in->alive_counter & 0x0F; 
    out->data[3] = calc_checksum(out->data, 3); 
}

int sensor_data_unpack(const can_frame_t *in, sensor_data_t *out){

    if(in->id != ID_SENSOR_DATA || in->dlc != 4){
        return -1;
    }

    if(calc_checksum(in->data, 3) != in->data[3]){
        return -1;
    }

    int16_t raw = (int16_t)((uint16_t)in->data[0] | ((uint16_t)in->data[1] << 8));
    out->temperature = raw/10.0f;
    out->alive_counter = in->data[2];

    return 0;
}