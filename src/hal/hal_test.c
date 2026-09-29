#include <stdio.h>
#include "can_driver.h"

int main(void) {

    if(can_init("vcan0") != 0) {
        printf("can_init failed\n");
        return 1;
    }

    can_frame_t f = { .id = 0x123, .dlc = 4, .data = {0xDE, 0xAD, 0xBE, 0xEF} };
    
    if(can_send(&f) != 0) {
        printf("can_send failed\n");
        can_close();
        return 1;
    }

    printf("Frame sent\n");
    can_close();
    return 0;
}
