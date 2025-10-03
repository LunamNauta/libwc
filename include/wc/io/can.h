#ifndef WC_CAN_HEADER
#define WC_CAN_HEADER

#include "wc/containers/que.h"

#include <string.h>

#include <linux/can.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>

//------------------------------------------------------------------------------------

#define WC_CAN_MAX_FRAMES 16

//------------------------------------------------------------------------------------

typedef struct wccan_frame{
    bool is_fd;
    union{
        struct canfd_frame fd;
        struct can_frame reg;
    };
} wccan_frame_t;

void wccan_frame_init(wccan_frame_t* frm, bool is_fd, bool eff, bool rtr);

void wccan_frame_set_id(wccan_frame_t* frm, canid_t id);
void wccan_frame_set_len(wccan_frame_t* frm, size_t len);

canid_t wccan_frame_id(const wccan_frame_t* frm);
size_t wccan_frame_len(const wccan_frame_t* frm);
const void* wccan_frame_data(const wccan_frame_t* frm);
bool wccan_frame_eff(const wccan_frame_t* frm);
bool wccan_frame_rtr(const wccan_frame_t* frm);
bool wccan_frame_err(const wccan_frame_t* frm);

//------------------------------------------------------------------------------------

typedef struct wccan_ctx{
    struct sockaddr_can addr;
    char* can_id;
    int socket;
    bool allow_fd;
    wcque_t received;
} wccan_ctx_t;

int wccan_ctx_init(wccan_ctx_t* ctx, const char* can_id, bool allow_fd);
void wccan_ctx_free(const wccan_ctx_t* ctx);

const char* wccan_ctx_can_id(const wccan_ctx_t* ctx);
int wccan_ctx_socket(const wccan_ctx_t* ctx);
bool wccan_ctx_allow_fd(const wccan_ctx_t* ctx);

int wccan_ctx_poll(wccan_ctx_t* ctx, wccan_frame_t* out_frm);
int wccan_ctx_pop_frame(wccan_ctx_t* ctx, wccan_frame_t* out_frm);

int wccan_ctx_read_bus(const wccan_ctx_t* ctx, wccan_frame_t* out_frame);
int wccan_ctx_write_bus(const wccan_ctx_t* ctx, const wccan_frame_t* frame);

//------------------------------------------------------------------------------------

#endif
