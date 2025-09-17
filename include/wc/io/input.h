#ifndef WC_INPUT_HEADER
#define WC_INPUT_HEADER

#include "wc/containers/vec.h"
#include "wc/containers/que.h"

#include <libevdev-1.0/libevdev/libevdev.h>

//------------------------------------------------------------------------

#define EV_DEVDROP EV_CNT
#define EV_CNT_ACT (EV_CNT+1)

#define WC_INPUT_MAX_EVENTS 16

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

const char* wcinput_get_name(const wcinput_device_t* dev);
const char* wcinput_get_phys(const wcinput_device_t* dev);
const char* wcinput_get_uniq(const wcinput_device_t* dev);

int wcinput_get_id_product(const wcinput_device_t* dev);
int wcinput_get_id_vendor(const wcinput_device_t* dev);
int wcinput_get_id_bustype(const wcinput_device_t* dev);
int wcinput_get_id_version(const wcinput_device_t* dev);
int wcinput_get_driver(const wcinput_device_t* dev);

int wcinput_has_prop(const wcinput_device_t* dev, unsigned int prop);
int wcinput_has_evtype(const wcinput_device_t* dev, unsigned int type);
int wcinput_has_evcode(const wcinput_device_t* dev, unsigned int type, unsigned int code);

int wcinput_get_abs_min(const wcinput_device_t* dev, unsigned int code);
int wcinput_get_abs_max(const wcinput_device_t* dev, unsigned int code);
int wcinput_get_abs_fuzz(const wcinput_device_t* dev, unsigned int code);
int wcinput_get_abs_flat(const wcinput_device_t* dev, unsigned int code);
int wcinput_get_abs_res(const wcinput_device_t* dev, unsigned int code);
const struct input_absinfo* wcinput_get_abs_info(const wcinput_device_t* dev, unsigned int code);
int wcinput_device_get_evval(const wcinput_device_t* dev, unsigned int type, unsigned int code);
int wcinput_device_fetch_evval(const wcinput_device_t* dev, unsigned int type, unsigned int code, int* val);
int wcinput_device_get_repeat(const wcinput_device_t* dev, int* delay, int* period);

//------------------------------------------------------------------------

int wcinput_get_slotval(const wcinput_device_t* dev, unsigned int slot, unsigned int code);
int wcinput_fetch_slotval(const wcinput_device_t* dev, unsigned int slot, unsigned int code, int* val);
int wcinput_get_num_slots(const wcinput_device_t* dev);
int wcinput_get_current_slot(const wcinput_device_t* dev);

//------------------------------------------------------------------------

int wcinput_next_ev(const wcinput_device_t* dev, unsigned int flags, wcinput_event_t* ev);
int wcinput_has_event(const wcinput_device_t* dev);

//------------------------------------------------------------------------

const char* wcinput_event_type_get_name(wcinput_event_t event);
const char* wcinput_event_code_get_name(wcinput_event_t event);

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max);

//------------------------------------------------------------------------

bool wcinput_passes_filters(const wcinput_device_t* dev, const wcvec_t* filters);

//------------------------------------------------------------------------

int wcinput_ctx_init(wcinput_ctx_t* ctx);
void wcinput_ctx_free(wcinput_ctx_t* ctx);

const wcvec_t* wcinput_ctx_devices(const wcinput_ctx_t* ctx);
const wcque_t* wcinput_ctx_events(const wcinput_ctx_t* ctx);

size_t wcinput_ctx_num_devices(const wcinput_ctx_t* ctx);
size_t wcinput_ctx_num_events(const wcinput_ctx_t* ctx);

int wcinput_ctx_scan_devices(wcinput_ctx_t* ctx, const wcvec_t* filters);
int wcinput_ctx_wait_device(wcinput_ctx_t* ctx, const wcvec_t* filters);

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out_ev);

//------------------------------------------------------------------------


#endif
