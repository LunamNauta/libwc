#ifndef WC_CAN_HEADER
#define WC_CAN_HEADER

#include <string.h>

#include <linux/can.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>

//------------------------------------------------------------------------------------

typedef struct wccan_frame{
    struct canfd_frame frame;
} wccan_frame_t;

#define wccan_frame_zero(frm) (memset(frm, 0, sizeof(wccan_frame_t)))
#define wccan_frame_set_id(frm, id) ((frm)->frame.can_id = (id & CAN_EFF_MASK) | ((id & CAN_EFF_MASK) > (id & CAN_SFF_MASK) ? CAN_EFF_FLAG : 0))
#define wccan_frame_get_id(frm) ((frm)->frame.can_id & CAN_EFF_MASK)
#define wccan_frame_get_eff(frm) ((frm)->frame.can_id & CAN_EFF_FLAG)
#define wccan_frame_get_rtr(frm) ((frm)->frame.can_id & CAN_RTR_FLAG)
#define wccan_frame_get_err(frm) ((frm)->frame.can_id & CAN_ERR_FLAG)
#define wccan_frame_get_data(frm) ((frm)->frame.data)
#define wccan_frame_get_len(frm) ((frm)->frame.len)
#define wccan_frame_set_len(frm, leni) ((frm)->frame.len = leni)

//------------------------------------------------------------------------------------

typedef struct wccan_ctx{
    int socket;
} wccan_ctx_t;

int wccan_ctx_init(wccan_ctx_t* ctx);
void wccan_ctx_free(const wccan_ctx_t* ctx);

int wccan_ctx_bind(const wccan_ctx_t* ctx, const char* can_id, struct sockaddr_can* out_addr);

ssize_t wccan_ctx_read_bus(const wccan_ctx_t* ctx, wccan_frame_t* out_frame);
ssize_t wccan_ctx_write_bus(const wccan_ctx_t* ctx, const wccan_frame_t* frame);

#endif
