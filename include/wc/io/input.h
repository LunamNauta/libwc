#ifndef WC_INPUT_HEADER
#define WC_INPUT_HEADER

#include "wc/containers/vec.h"
#include "wc/containers/que.h"

#include <stdbool.h>

#include <libevdev-1.0/libevdev/libevdev.h>

typedef struct wcinput_device_t{
    struct libevdev* dev;
    int fd;
} wcinput_device_t;

typedef struct wcinput_event_t{
    const wcinput_device_t* dev;
    struct input_event ev;
} wcinput_event_t;

typedef struct wcinput_ctx_t{
    wcvec_t devices;
    wcque_t events;
} wcinput_ctx_t;

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max);

int wcinput_ctx_init(wcinput_ctx_t* ctx);
void wcinput_ctx_free(wcinput_ctx_t* ctx);

int wcinput_ctx_find_devices(wcinput_ctx_t* ctx);

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out);

#endif
