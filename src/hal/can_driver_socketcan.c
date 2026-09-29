#include "can_driver.h"

#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>

static int can_socket = -1;

int can_init(const char *iface_name){

    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (can_socket<0){
        perror("socket");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ - 1);

    if(ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0){
        perror("ioctl");
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind");
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    return 0;
}

int can_send(const can_frame_t *frame){
    if(can_socket < 0){
        return -1;

    }

    struct can_frame raw_frame;
    memset(&raw_frame, 0, sizeof(raw_frame));
    raw_frame.can_id = frame->id;
    raw_frame.can_dlc = frame->dlc;
    memcpy(raw_frame.data, frame->data, frame->dlc);

    ssize_t nbytes = write(can_socket, &raw_frame, sizeof(raw_frame));

    if(nbytes != sizeof(raw_frame)){
        perror("write");
        return -1;
    }

    return 0;

}

int can_receive(can_frame_t *frame){
    if(can_socket < 0){
        return -1;

    }

    struct can_frame raw_frame;
    ssize_t nbytes = read(can_socket, &raw_frame, sizeof(raw_frame));
    if(nbytes != sizeof(raw_frame)){
        perror("read");
        return -1;
    }

   
    frame->id = raw_frame.can_id;
    frame->dlc = raw_frame.can_dlc;
    memcpy(frame->data, raw_frame.data, raw_frame.can_dlc);

    return 0;

}

void can_close(void){
    if(can_socket >= 0){
        close(can_socket);
        can_socket = -1;

    }

}