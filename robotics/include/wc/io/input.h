#ifndef WC_INPUT_HEADER
#define WC_INPUT_HEADER

#include "wc/containers/vec.h"
#include "wc/containers/que.h"

#include <stdbool.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#define WC_INPUT_MAX_EVENTS 16

typedef struct wcinput_device{
    struct libevdev* dev;
    int fd;
} wcinput_device_t;

typedef struct wcinput_event{
    const wcinput_device_t* dev;
    struct input_event ev;
} wcinput_event_t;

typedef struct wcinput_ctx{
    wcvec_t devices;
    wcque_t events;
} wcinput_ctx_t;

typedef struct wcinput_event_filter{
    typeof((struct input_event){0}.type) type;
    wcvec_t codes;
} wcinput_event_filter_t;

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max);

int wcinput_ctx_init(wcinput_ctx_t* ctx);
void wcinput_ctx_free(wcinput_ctx_t* ctx);

const wcvec_t* wcinput_ctx_devices(const wcinput_ctx_t* ctx);
const wcque_t* wcinput_ctx_events(const wcinput_ctx_t* ctx);

int wcinput_ctx_find_devices(wcinput_ctx_t* ctx, const wcvec_t* filters);

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out);

#endif
