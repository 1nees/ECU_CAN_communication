#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} can_frame_t;

int can_init(const char *iface_name);
int can_send(const can_frame_t *frame);
int can_receive(can_frame_t *frame);

void can_close(void);

#endif