#include <libevdev-1.0/libevdev/libevdev.h>

#define T wcinput_dev_list, struct libevdev*
#include "wctl/vector.h"

#define T wcinput_ev_list, struct input_event
#include "wctl/vector.h"

typedef struct wcinput_device_s{
    struct libevdev* dev;
    int fd;
} wcinput_device;

typedef struct wcinput_event_s{
    const wcinput_device* dev;
    struct input_event* ev;
} wcinput_event;

typedef struct wcinput_ctx_s{
    wcinput_dev_list devices;
    wcinput_ev_list events;
} wcinput_ctx;
