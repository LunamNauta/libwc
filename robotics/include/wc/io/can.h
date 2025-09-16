#ifndef WC_CAN_HEADER
#define WC_CAN_HEADER

#include <string.h>

#include <linux/can.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>

int wccan_init_socket();
void wccan_free_socket(int socket);

int wccan_bind_socket(int socket, const char* can_id, struct sockaddr_can* out_addr);

ssize_t wccan_read_bus(int socket, struct can_frame* out_frame);
ssize_t wccan_write_bus(int socket, struct can_frame frame);

#endif
