#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdint.h>
#include "can_driver.h"

#define ID_SENSOR_DATA 0x100

typedef struct {
    float temperature;
    uint8_t alive_counter;
} sensor_data_t;

void sensor_data_pack(const sensor_data_t *in, can_frame_t *out);
int sensor_data_unpack(const can_frame_t *in, sensor_data_t *out);

#endif