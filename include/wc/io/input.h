#ifndef WC_INPUT_HEADER
#define WC_INPUT_HEADER

#include "wc/containers/vec.h"
#include "wc/containers/que.h"

#include <libevdev-1.0/libevdev/libevdev.h>

//------------------------------------------------------------------------

#define EV_DEVDROP EV_CNT
#define EV_CNT_ACT (EV_CNT+1)

#define WC_INPUT_MAX_EVENTS 64

//------------------------------------------------------------------------

typedef struct wcinput_device{
    struct libevdev* evdev;
    int evfd;
} wcinput_device_t;

typedef struct wcinput_event{
    const wcinput_device_t* dev;
    struct input_event ev;
} wcinput_event_t;

typedef struct wcinput_event_filter{
    typeof((struct input_event){0}.type) type;
    wcvec_t codes;
} wcinput_event_filter_t;

typedef struct wcinput_ctx{
    wcvec_t devices;
    wcque_t events;
} wcinput_ctx_t;

//------------------------------------------------------------------------

bool wcinput_dev_passes_filters(const wcinput_device_t* dev, const wcvec_t* filters);
const struct libevdev* wcinput_dev_evdev(const wcinput_device_t* dev);
int wcinput_dev_evfd(const wcinput_device_t* dev);

const struct libevdev* wcinput_ev_evdev(const wcinput_event_t* ev);
const wcinput_device_t* wcinput_ev_dev(const wcinput_event_t* ev);
struct input_event wcinput_ev_ev(const wcinput_event_t* ev);
int wcinput_ev_evfd(const wcinput_event_t* ev);

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max);

//------------------------------------------------------------------------

int wcinput_ctx_init(wcinput_ctx_t* ctx);
void wcinput_ctx_free(wcinput_ctx_t* ctx);

const wcvec_t* wcinput_ctx_devices(const wcinput_ctx_t* ctx);
const wcque_t* wcinput_ctx_events(const wcinput_ctx_t* ctx);

bool wcinput_ctx_event_pending(const wcinput_ctx_t* ctx);
size_t wcinput_ctx_num_devices(const wcinput_ctx_t* ctx);
size_t wcinput_ctx_num_events(const wcinput_ctx_t* ctx);

int wcinput_ctx_scan_devices(wcinput_ctx_t* ctx, const wcvec_t* filters);
int wcinput_ctx_wait_device(wcinput_ctx_t* ctx, const wcvec_t* filters);

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out_ev);
int wcinput_ctx_pop_event(wcinput_ctx_t* ctx, wcinput_event_t* out_ev);

//------------------------------------------------------------------------


#endif
