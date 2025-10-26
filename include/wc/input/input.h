#ifndef WC_INPUT_HEADER
#define WC_INPUT_HEADER

#include <stdbool.h>
#include <stddef.h>

#include "wc/container/vector.h"
#include "wc/container/deque.h"

#include <libevdev-1.0/libevdev/libevdev.h>

#define EV_DEVDROP EV_CNT
#define EV_CNT_ACT (EV_CNT+1)

#define WC_INPUT_MAX_EVENTS 64

typedef struct wcinput_device{
    struct libevdev* evdev;
    int evfd;
} wcinput_device_t;

typedef struct wcinput_event{
    const wcinput_device_t* dev;
    struct input_event ev;
} wcinput_event_t;

#define wcinput_event_dev(wcev) ((wcev).dev)
#define wcinput_event_type(wcev) ((wcev).ev.type)
#define wcinput_event_code(wcev) ((wcev).ev.code)
#define wcinput_event_value(wcev) ((wcev).ev.value)

typedef struct wcinput_event_filter{
    typeof((struct input_event){0}.type) type;
    wcvector_t codes;
} wcinput_event_filter_t;

typedef struct wcinput{
    wcvector_t devices;
    wcdeque_t events;
} wcinput_t;

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max);

int wcinput_init(wcinput_t* ctx);
void wcinput_free(wcinput_t* ctx);

const wcvector_t* wcinput_devices(const wcinput_t* ctx);
const wcdeque_t* wcinput_events(const wcinput_t* ctx);

bool wcinput_event_pending(const wcinput_t* ctx);
size_t wcinput_num_devices(const wcinput_t* ctx);
size_t wcinput_num_events(const wcinput_t* ctx);

int wcinput_scan_devices(wcinput_t* ctx, const wcvector_t* filters);
int wcinput_wait_device(wcinput_t* ctx, const wcvector_t* filters);

int wcinput_poll_events(wcinput_t* ctx, wcinput_event_t* out_ev);
bool wcinput_pop_event(wcinput_t* ctx, wcinput_event_t* out_ev);

#endif
