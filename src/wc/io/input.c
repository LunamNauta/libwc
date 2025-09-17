#include "wc/io/input.h"

#include <string.h>
#include <errno.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <libudev.h>

//------------------------------------------------------------------------

const char* wcinput_get_name(const wcinput_device_t* dev){
    return libevdev_get_name(dev->evdev);
}
const char* wcinput_get_phys(const wcinput_device_t* dev){
    return libevdev_get_phys(dev->evdev);
}
const char* wcinput_get_uniq(const wcinput_device_t* dev){
    return libevdev_get_uniq(dev->evdev);
}

int wcinput_get_id_product(const wcinput_device_t* dev){
    return libevdev_get_id_product(dev->evdev);
}
int wcinput_get_id_vendor(const wcinput_device_t* dev){
    return libevdev_get_id_vendor(dev->evdev);
}
int wcinput_get_id_bustype(const wcinput_device_t* dev){
    return libevdev_get_id_bustype(dev->evdev);
}
int wcinput_get_id_version(const wcinput_device_t* dev){
    return libevdev_get_id_version(dev->evdev);
}
int wcinput_get_driver(const wcinput_device_t* dev){
    return libevdev_get_driver_version(dev->evdev);
}

int wcinput_has_prop(const wcinput_device_t* dev, unsigned int prop){
    return libevdev_has_property(dev->evdev, prop);
}
int wcinput_has_evtype(const wcinput_device_t* dev, unsigned int type){
    return libevdev_has_event_type(dev->evdev, type);
}
int wcinput_has_evcode(const wcinput_device_t* dev, unsigned int type, unsigned int code){
    return libevdev_has_event_code(dev->evdev, type, code);
}

int wcinput_get_abs_min(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_minimum(dev->evdev, code);
}
int wcinput_get_abs_max(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_maximum(dev->evdev, code);
}
int wcinput_get_abs_fuzz(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_fuzz(dev->evdev, code);
}
int wcinput_get_abs_flat(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_flat(dev->evdev, code);
}
int wcinput_get_abs_res(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_resolution(dev->evdev, code);
}
const struct input_absinfo* wcinput_get_abs_info(const wcinput_device_t* dev, unsigned int code){
    return libevdev_get_abs_info(dev->evdev, code);
}
int wcinput_device_get_evval(const wcinput_device_t* dev, unsigned int type, unsigned int code){
    return libevdev_get_event_value(dev->evdev, type, code);
}
int wcinput_device_fetch_evval(const wcinput_device_t* dev, unsigned int type, unsigned int code, int* val){
    return libevdev_fetch_event_value(dev->evdev, type, code, val);
}
int wcinput_device_get_repeat(const wcinput_device_t* dev, int* delay, int* period){
    return libevdev_get_repeat(dev->evdev, delay, period);
}

//------------------------------------------------------------------------

int wcinput_get_slotval(const wcinput_device_t* dev, unsigned int slot, unsigned int code){
    return libevdev_get_slot_value(dev->evdev, slot, code);
}
int wcinput_fetch_slotval(const wcinput_device_t* dev, unsigned int slot, unsigned int code, int* val){
    return libevdev_fetch_slot_value(dev->evdev, slot, code, val);
}
int wcinput_get_num_slots(const wcinput_device_t* dev){
    return libevdev_get_num_slots(dev->evdev);
}
int wcinput_get_current_slot(const wcinput_device_t* dev){
    return libevdev_get_current_slot(dev->evdev);
}

//------------------------------------------------------------------------

int wcinput_next_ev(const wcinput_device_t* dev, unsigned int flags, wcinput_event_t* ev){
    ev->dev = dev;
    return libevdev_next_event(dev->evdev, flags, &ev->ev);
}
int wcinput_has_event(const wcinput_device_t* dev){
    return libevdev_has_event_pending(dev->evdev);
}

//------------------------------------------------------------------------

const char* wcinput_event_type_get_name(wcinput_event_t event){
    return libevdev_event_type_get_name(event.ev.type);
}
const char* wcinput_event_code_get_name(wcinput_event_t event){
    return libevdev_event_code_get_name(event.ev.type, event.ev.code);
}

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max){
    if (event.ev.type == EV_KEY){
        if (event.ev.value) return out_max;
        else return out_min;
    }
    if (event.ev.type != EV_ABS) return event.ev.value;
    int flat = libevdev_get_abs_flat(event.dev->evdev, event.ev.code);
    int in_max = libevdev_get_abs_maximum(event.dev->evdev, event.ev.code) - flat;
    int in_min = libevdev_get_abs_minimum(event.dev->evdev, event.ev.code) + flat;
    int in_range = in_max - in_min;
    int out_range = out_max - out_min;
    float value = event.ev.value < 0 ? event.ev.value + flat : event.ev.value - flat;
    return (value - in_min)*out_range/in_range + out_min;
}

//------------------------------------------------------------------------

bool wcinput_passes_filters(const wcinput_device_t* dev, const wcvec_t* filters){
    bool valid = true;
    if (!filters) return true;
    for (size_t a = 0; valid && a < wcvec_size(filters); a++){
        const wcinput_event_filter_t* filter = wcvec_get(filters, a);
        if (!libevdev_has_event_type(dev->evdev, filter->type)){
            valid = false;
            break;
        }
        for (size_t b = 0; b < wcvec_size(&filter->codes); b++){
            const typeof((struct input_event){0}.code)* code = wcvec_get(&filter->codes, b);
            if (!libevdev_has_event_code(dev->evdev, filter->type, *code)){
                valid = false;
                break;
            }
        }
    }
    return valid;
}

//------------------------------------------------------------------------

int wcinput_ctx_init(wcinput_ctx_t* ctx){
    if (wcque_init_reserved(&ctx->events, sizeof(wcinput_event_t), WC_INPUT_MAX_EVENTS)) return -1;
    wcvec_init(&ctx->devices, sizeof(wcinput_device_t));
    return 0;
}
void wcinput_ctx_free(wcinput_ctx_t* ctx){
    wcvec_free(&ctx->devices);
    wcque_free(&ctx->events);
}

const wcvec_t* wcinput_ctx_devices(const wcinput_ctx_t* ctx){
    return &ctx->devices;
}
const wcque_t* wcinput_ctx_events(const wcinput_ctx_t* ctx){
    return &ctx->events;
}

size_t wcinput_ctx_num_devices(const wcinput_ctx_t* ctx){
    return wcvec_size(&ctx->devices);
}
size_t wcinput_ctx_num_events(const wcinput_ctx_t* ctx){
    return wcque_size(&ctx->events);
}

int wcinput_ctx_scan_devices(wcinput_ctx_t* ctx, const wcvec_t* filters){
    struct dirent* entry;
    wcinput_device_t dev;
    DIR* folder;
    int rc = 1;

    const char* base_path = "/dev/input/";
    size_t reserved_space = 7;
    char* file_path = malloc(strlen(base_path) + reserved_space + 1);
    if (!file_path) return -1;
    strcpy(file_path, base_path);

    folder = opendir(base_path);
    if (!folder) return -1;

    while ((entry = readdir(folder)), entry){
        if (entry->d_type != DT_CHR) continue;
        if (strlen(entry->d_name) > reserved_space){
            reserved_space = strlen(entry->d_name);
            file_path = realloc(file_path, strlen(base_path) + reserved_space + 1);
            if (!file_path) return -1;
        }
        file_path[strlen(base_path)] = 0;
        strcat(file_path, entry->d_name);

        dev.evfd = open(file_path, O_RDONLY | O_NONBLOCK);
        if (dev.evfd < 0) continue;
        if (libevdev_new_from_fd(dev.evfd, &dev.evdev) < 0) continue;
    
        if (wcinput_passes_filters(&dev, filters)){
            if (wcvec_push_back(&ctx->devices, &dev) < 0) return -1;
        }
        else{
            libevdev_free(dev.evdev);
            close(dev.evfd);
        }
    }

    closedir(folder);
    return 0;
}
int wcinput_ctx_wait_device(wcinput_ctx_t* ctx, const wcvec_t* filters){
    struct udev_monitor *monitor;
    struct udev_device *device;
    wcinput_device_t dev;
    struct udev *udev;
    int monitor_fd;
    int device_fd;

    udev = udev_new();
    if (!udev) return -1;

    monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor){
        udev_unref(udev);
        return -1;
    }

    udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", nullptr);

    udev_monitor_enable_receiving(monitor);
    monitor_fd = udev_monitor_get_fd(monitor);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(monitor_fd, &fds);

    while (true){
        if (select(monitor_fd + 1, &fds, nullptr, nullptr, nullptr) < 0) break;

        if (FD_ISSET(monitor_fd, &fds)) {
            device = udev_monitor_receive_device(monitor);
            if (device) {
                const char *action = udev_device_get_action(device);
                const char *dev_path = udev_device_get_devnode(device);
                if (action && dev_path) {
                    if (strcmp(action, "add") == 0){
                        if (strncmp("/dev/input/event", dev_path, 16) != 0) continue;

                        dev.evfd = open(dev_path, O_RDONLY | O_NONBLOCK);
                        if (dev.evfd < 0) continue;
                        if (libevdev_new_from_fd(dev.evfd, &dev.evdev) < 0) continue;
    
                        if (wcinput_passes_filters(&dev, filters)){
                            if (wcvec_push_back(&ctx->devices, &dev) < 0) return -1;
                            break;
                        }
                        else{
                            libevdev_free(dev.evdev);
                            close(dev.evfd);
                        }
                    }
                }
                udev_device_unref(device);
            }
        }
    }
    return 0;
}

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out_ev){
    struct input_event event;
    int rc;

    wcinput_event_t tmp;
    for (size_t a = 0; a < wcvec_size(&ctx->devices); a++){
        wcinput_device_t* dev = wcvec_get(&ctx->devices, a);
        do{
            rc = libevdev_next_event(dev->evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
            if (rc == -EAGAIN || rc == LIBEVDEV_READ_STATUS_SYNC) continue;
            if (rc == -ENODEV){
                libevdev_free(dev->evdev);
                close(dev->evfd);
                event.type = EV_DEVDROP;
                wcinput_event_t tmp = {dev, event};
                wcque_push_back_rot(&ctx->events, &tmp);
                wcvec_erase(&ctx->devices, a--);
                break;
            }
            if (event.type == EV_SYN) continue;
            wcinput_event_t tmp = {dev, event};
            wcque_push_back_rot(&ctx->events, &tmp);
        } while (rc != -EAGAIN && (rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == LIBEVDEV_READ_STATUS_SYNC));
    }
    if (!wcque_size(&ctx->events)) return -1;
    if (out_ev){
        *out_ev = *(wcinput_event_t*)wcque_back(&ctx->events);
        wcque_pop_front(&ctx->events);
    }
    return 0;
}

//------------------------------------------------------------------------
