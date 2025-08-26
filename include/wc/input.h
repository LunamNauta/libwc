#include <libevdev-1.0/libevdev/libevdev.h>

#define T wcinput_dev_vec, struct libevdev*
#include "wctl/vector.h"

#define T wcinput_ev_que, struct input_event
#include "wctl/queue.h"

typedef struct wcinput_device_s{
    struct libevdev* dev;
    int fd;
} wcinput_device;

typedef struct wcinput_event_s{
    const wcinput_device* dev;
    struct input_event* ev;
} wcinput_event;

typedef struct wcinput_ctx_s{
    wcinput_dev_vec devices;
    wcinput_ev_que events;
} wcinput_ctx;

int wcinput_ctx_init(wcinput_ctx* ctx){
    if (wcinput_dev_vec_init(&ctx->devices, 4)) return -1;
    if (wcinput_ev_que_init(&ctx->events, 8)) return -1;
    return 0;
}
int wcinput_ctx_find_devices(wcinput_ctx* ctx){
    return 0;
}


