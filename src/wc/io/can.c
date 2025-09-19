#include "wc/io/can.h"

#include <linux/can/raw.h>

//------------------------------------------------------------------------------------

int wccan_ctx_init(wccan_ctx_t* ctx){
    ctx->socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (ctx->socket < 0) return -1;
    return 0;
}
void wccan_ctx_free(const wccan_ctx_t* ctx){
    close(ctx->socket);
}

int wccan_ctx_bind(const wccan_ctx_t* ctx, const char* can_id, struct sockaddr_can* out_addr){
    struct sockaddr_can addr = {0};
    struct ifreq if_req;

    strncpy(if_req.ifr_name, can_id, IFNAMSIZ);
    ioctl(ctx->socket, SIOCGIFINDEX, &if_req);
    addr.can_ifindex = if_req.ifr_ifindex;
    addr.can_family = AF_CAN;

    if (out_addr) *out_addr = addr;
    if (bind(ctx->socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_can)) < 0) return -1;
    int enable_canfd = 1;
    if (setsockopt(ctx->socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(int)) < 0) return -1;
    return 0;
}

ssize_t wccan_ctx_read_bus(const wccan_ctx_t* ctx, wccan_frame_t* out_frame){
    return read(ctx->socket, &out_frame->frame, sizeof(struct can_frame));
}
ssize_t wccan_ctx_write_bus(const wccan_ctx_t* ctx, const wccan_frame_t* frame){
    return write(ctx->socket, &frame->frame, sizeof(struct can_frame));
}

//------------------------------------------------------------------------------------
