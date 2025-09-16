#include "wc/io/can.h"

int wccan_init_socket(){
    return socket(PF_CAN, SOCK_RAW, CAN_RAW);
}
void wccan_free_socket(int socket){
    close(socket);
}

int wccan_bind_socket(int socket, const char* can_id, struct sockaddr_can* out_addr){
    struct sockaddr_can addr = {0};
    struct ifreq if_req;

    strcpy(if_req.ifr_name, can_id);
    ioctl(socket, SIOCGIFINDEX, &if_req);
    addr.can_ifindex = if_req.ifr_ifindex;
    addr.can_family = AF_CAN;

    if (out_addr) *out_addr = addr;
    return bind(socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_can));
}

ssize_t wccan_read_bus(int socket, struct can_frame* out_frame){
    return write(socket, out_frame, sizeof(struct can_frame));
}
ssize_t wccan_write_bus(int socket, struct can_frame frame){
    return write(socket, &frame, sizeof(struct can_frame));
}
