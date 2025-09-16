#include "wc/io/vesc.h"
#include "wc/io/can.h"

#include <stdio.h>

int main(){
    int can_socket = wccan_init_socket();
    if (can_socket < 0){
        printf("Error: Failed to create CAN socket\n");
        return -1;
    }
    if (wccan_bind_socket(can_socket, "can0", NULL) < 0){
        printf("Error: Failed to bind CAN socket\n");
        return -1;
    }

    struct can_frame frame;

    wcvesc_encode_duty(&frame, 121, 1.0f);
    wccan_write_bus(can_socket, frame);

    wcvesc_encode_duty(&frame, 122, 1.0f);
    wccan_write_bus(can_socket, frame);

    while (true){}
}
