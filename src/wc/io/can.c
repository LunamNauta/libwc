#include "wc/io/can.h"
#include "wc/containers/que.h"

#include <stdlib.h>

#include <linux/can/raw.h>
#include <poll.h>
#include <sys/poll.h>

//------------------------------------------------------------------------------------

void wccan_frame_init(wccan_frame_t* frm, bool is_fd, bool eff, bool rtr){
    memset(frm, 0, sizeof(wccan_frame_t));
    frm->is_fd = is_fd;
    if (is_fd){
        frm->fd.can_id |= (eff ? CAN_EFF_FLAG : 0);
        frm->fd.can_id |= (rtr ? CAN_RTR_FLAG : 0);
    }
    else{
        frm->reg.can_id |= (eff ? CAN_EFF_FLAG : 0);
        frm->reg.can_id |= (rtr ? CAN_RTR_FLAG : 0);
    }
}

void wccan_frame_set_id(wccan_frame_t* frm, canid_t id){
    bool has_eff = wccan_frame_eff(frm);
    bool has_rtr = wccan_frame_rtr(frm);
    if (frm->is_fd) frm->fd.can_id = (id & (has_eff ? CAN_EFF_MASK : CAN_SFF_MASK)) | (has_eff ? CAN_EFF_FLAG : 0) | (has_rtr ? CAN_RTR_FLAG : 0);
    else frm->reg.can_id = (id & (has_eff ? CAN_EFF_MASK : CAN_SFF_MASK)) | (has_eff ? CAN_EFF_FLAG : 0) | (has_rtr ? CAN_RTR_FLAG : 0);
}
void wccan_frame_set_len(wccan_frame_t* frm, size_t len){
    if (frm->is_fd) frm->fd.len = len;
    else frm->reg.len = len;
}

canid_t wccan_frame_id(const wccan_frame_t* frm){
    bool has_eff = wccan_frame_eff(frm);
    if (frm->is_fd) return frm->fd.can_id & (has_eff ? CAN_EFF_MASK : CAN_SFF_MASK);
    return frm->reg.can_id & (has_eff ? CAN_EFF_MASK : CAN_SFF_MASK);
}
size_t wccan_frame_len(const wccan_frame_t* frm){
    if (frm->is_fd) return frm->fd.len;
    return frm->reg.len;
}
const void* wccan_frame_data(const wccan_frame_t* frm){
    if (frm->is_fd) return frm->fd.data;
    return frm->reg.data;
}
bool wccan_frame_eff(const wccan_frame_t* frm){
    if (frm->is_fd) return (frm->fd.can_id & CAN_EFF_FLAG) == CAN_EFF_FLAG;
    return (frm->reg.can_id & CAN_EFF_FLAG) == CAN_EFF_FLAG;
}
bool wccan_frame_rtr(const wccan_frame_t* frm){
    if (frm->is_fd) return (frm->fd.can_id & CAN_RTR_FLAG) == CAN_RTR_FLAG;
    return (frm->reg.can_id & CAN_RTR_FLAG) == CAN_RTR_FLAG;
}
bool wccan_frame_err(const wccan_frame_t* frm){
    if (frm->is_fd) return (frm->fd.can_id & CAN_ERR_FLAG) == CAN_ERR_FLAG;
    return (frm->reg.can_id & CAN_ERR_FLAG) == CAN_ERR_FLAG;
}

//------------------------------------------------------------------------------------

int wccan_ctx_init(wccan_ctx_t* ctx, const char* can_id, bool allow_fd){
    struct ifreq if_req;

    ctx->socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (ctx->socket < 0) return -1;

    strncpy(if_req.ifr_name, can_id, IFNAMSIZ);
    ioctl(ctx->socket, SIOCGIFINDEX, &if_req);
    ctx->addr.can_ifindex = if_req.ifr_ifindex;
    ctx->addr.can_family = AF_CAN;
    if (bind(ctx->socket, (struct sockaddr*)&ctx->addr, sizeof(struct sockaddr_can)) < 0) return -1;

    if (allow_fd){
        int enable_canfd = allow_fd;
        if (setsockopt(ctx->socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(int)) < 0) return -1;
        ctx->allow_fd = allow_fd;
    }

    ctx->can_id = malloc((strlen(can_id) + 1)*sizeof(char));
    if (!ctx->can_id) return -1;
    strcpy(ctx->can_id, can_id);

    if (wcque_init_reserved(&ctx->received, sizeof(wccan_frame_t), WC_CAN_MAX_FRAMES)) return -1;

    return 0;
}
void wccan_ctx_free(const wccan_ctx_t* ctx){
    close(ctx->socket);
    free(ctx->can_id);
}

const char* wccan_ctx_can_id(const wccan_ctx_t* ctx){
    return ctx->can_id;
}
int wccan_ctx_socket(const wccan_ctx_t* ctx){
    return ctx->socket;
}
bool wccan_ctx_allow_fd(const wccan_ctx_t* ctx){
    return ctx->allow_fd;
}

int wccan_ctx_poll(wccan_ctx_t* ctx, wccan_frame_t* out_frm){
    while (true){
        wccan_frame_t frm;
        int res = wccan_ctx_read_bus(ctx, &frm);
        if (res < 0) return -1;
        if (res == 0) break;
        wcque_push_back_rot(&ctx->received, &frm);
    }
    if (out_frm) return wccan_ctx_pop_frame(ctx, out_frm);
    return 0;
}
int wccan_ctx_pop_frame(wccan_ctx_t* ctx, wccan_frame_t* out_frm){
    if (!wcque_size(&ctx->received)) return 0;
    if (out_frm) *out_frm = *(wccan_frame_t*)wcque_back(&ctx->received);
    wcque_pop_front(&ctx->received);
    return 1;
}

int wccan_ctx_read_bus(const wccan_ctx_t* ctx, wccan_frame_t* out_frame){
    struct pollfd pfd;
    pfd.fd = ctx->socket;
    pfd.events = POLLIN;
    
    int res = poll(&pfd, 1, 0);
    if (res < 0) return -1;
    if (res == 0) return 0;
    if ((pfd.revents & POLLIN) != POLLIN) return -1;

    if (ctx->allow_fd) return read(ctx->socket, &out_frame->fd, sizeof(struct canfd_frame)) == sizeof(struct canfd_frame) ? 1 : -1;
    return read(ctx->socket, &out_frame->reg, sizeof(struct can_frame)) == sizeof(struct can_frame) ? 1 : -1;
}
int wccan_ctx_write_bus(const wccan_ctx_t* ctx, const wccan_frame_t* frame){
    if (ctx->allow_fd) return write(ctx->socket, &frame->fd, sizeof(struct canfd_frame)) == sizeof(struct canfd_frame) ? 1 : -1;
    return write(ctx->socket, &frame->reg, sizeof(struct can_frame)) == sizeof(struct can_frame) ? 1 : -1;
}

//------------------------------------------------------------------------------------
