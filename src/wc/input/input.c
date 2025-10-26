#include "wc/input/input.h"
#include "wc/container/deque.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libudev.h>

bool _wcinput_dev_passes_filters(const wcinput_device_t* dev, const wcvector_t* filters){
    bool valid = true;
    if (!filters) return true;
    for (size_t a = 0; valid && a < wcvector_size(filters); a++){
        const wcinput_event_filter_t* filter = wcvector_get(filters, a);
        if (!libevdev_has_event_type(dev->evdev, filter->type)){
            valid = false;
            break;
        }
        for (size_t b = 0; b < wcvector_size(&filter->codes); b++){
            const typeof((struct input_event){0}.code)* code = wcvector_get(&filter->codes, b);
            if (!libevdev_has_event_code(dev->evdev, filter->type, *code)){
                valid = false;
                break;
            }
        }
    }
    return valid;
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

int wcinput_init(wcinput_t* ctx){
    wcdeque_init(&ctx->events, sizeof(wcinput_event_t));
    if (wcdeque_reserve(&ctx->events, WC_INPUT_MAX_EVENTS)) return -1;
    wcvector_init(&ctx->devices, sizeof(wcinput_device_t));
    return 0;
}
void wcinput_free(wcinput_t* ctx){
    wcvector_free(&ctx->devices);
    wcdeque_free(&ctx->events);
}

const wcvector_t* wcinput_devices(const wcinput_t* ctx){
    return &ctx->devices;
}
const wcdeque_t* wcinput_events(const wcinput_t* ctx){
    return &ctx->events;
}

bool wcinput_event_pending(const wcinput_t* ctx){
    for (size_t a = 0; a < wcvector_size(&ctx->devices); a++){
        if (libevdev_has_event_pending(wcvector_get(&ctx->devices, a)) > 0) return true;
    }
    return false;
}
size_t wcinput_num_devices(const wcinput_t* ctx){
    return wcvector_size(&ctx->devices);
}
size_t wcinput_num_events(const wcinput_t* ctx){
    return wcdeque_size(&ctx->events);
}

int wcinput_scan_devices(wcinput_t* ctx, const wcvector_t* filters){
    struct dirent* entry;
    wcinput_device_t dev;
    struct stat stat1;
    struct stat stat2;
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
        if (fstat(dev.evfd, &stat1) < 0){
            close(dev.evfd);
            continue;
        }
        bool same_file = false;
        for (size_t a = 0; a < wcvector_size(&ctx->devices); a++){
            wcinput_device_t* other_dev = wcvector_get(&ctx->devices, a);
            if (fstat(other_dev->evfd, &stat2) < 0){
                struct input_event event;
                libevdev_free(other_dev->evdev);
                close(other_dev->evfd);
                event.type = EV_DEVDROP;
                wcinput_event_t tmp = {other_dev, event};
                wcdeque_push_back_rot(&ctx->events, &tmp);
                wcvector_erase(&ctx->devices, a--);
                continue;
            }
            if (stat1.st_ino == stat2.st_ino && stat1.st_dev == stat2.st_dev){
                same_file = true;
                break;
            }
        }
        if (same_file){
            close(dev.evfd);
            continue;
        }
        if (libevdev_new_from_fd(dev.evfd, &dev.evdev) < 0){
            close(dev.evfd);
            continue;
        }
    
        if (_wcinput_dev_passes_filters(&dev, filters)){
            if (wcvector_push_back(&ctx->devices, &dev) < 0) return -1;
        }
        else{
            libevdev_free(dev.evdev);
            close(dev.evfd);
        }
    }

    closedir(folder);
    return 0;
}
int wcinput_wait_device(wcinput_t* ctx, const wcvector_t* filters){
    struct udev_monitor *monitor;
    struct udev_device *device;
    wcinput_device_t dev;
    struct udev *udev;
    struct stat stat1;
    struct stat stat2;
    int monitor_fd;
    int device_fd;

    udev = udev_new();
    if (!udev) return -1;

    monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor){
        udev_unref(udev);
        return -1;
    }

    udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", NULL);

    udev_monitor_enable_receiving(monitor);
    monitor_fd = udev_monitor_get_fd(monitor);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(monitor_fd, &fds);

    while (true){
        if (select(monitor_fd + 1, &fds, NULL, NULL, NULL) < 0) break;

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
                        if (fstat(dev.evfd, &stat1) < 0){
                            close(dev.evfd);
                            continue;
                        }
                        bool same_file = false;
                        for (size_t a = 0; a < wcvector_size(&ctx->devices); a++){
                            wcinput_device_t* other_dev = wcvector_get(&ctx->devices, a);
                            if (fstat(other_dev->evfd, &stat2) < 0){
                                struct input_event event;
                                libevdev_free(other_dev->evdev);
                                close(other_dev->evfd);
                                event.type = EV_DEVDROP;
                                wcinput_event_t tmp = {other_dev, event};
                                wcdeque_push_back_rot(&ctx->events, &tmp);
                                wcvector_erase(&ctx->devices, a--);
                                continue;
                            }
                            if (stat1.st_ino == stat2.st_ino && stat1.st_dev == stat2.st_dev){
                                same_file = true;
                                break;
                            }
                        }
                        if (same_file){
                            close(dev.evfd);
                            continue;
                        }
                        if (libevdev_new_from_fd(dev.evfd, &dev.evdev) < 0){
                            close(dev.evfd);
                            continue;
                        }
    
                        if (_wcinput_dev_passes_filters(&dev, filters)){
                            if (wcvector_push_back(&ctx->devices, &dev) < 0) return -1;
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

int wcinput_poll_events(wcinput_t* ctx, wcinput_event_t* out_ev){
    struct input_event event;
    int rc;

    wcinput_event_t tmp;
    for (size_t a = 0; a < wcvector_size(&ctx->devices); a++){
        wcinput_device_t* dev = wcvector_get(&ctx->devices, a);
        if (libevdev_has_event_pending(dev->evdev) <= 0) continue;
        do{
            rc = libevdev_next_event(dev->evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
            if (rc == LIBEVDEV_READ_STATUS_SYNC) continue;
            if (rc == -EAGAIN) break;
            if (rc == -ENODEV){
                libevdev_free(dev->evdev);
                close(dev->evfd);
                event.type = EV_DEVDROP;
                wcinput_event_t tmp = {dev, event};
                wcdeque_push_back_rot(&ctx->events, &tmp);
                wcvector_erase(&ctx->devices, a--);
                break;
            }
            if (event.type == EV_SYN) continue;
            wcinput_event_t tmp = {dev, event};
            wcdeque_push_back_rot(&ctx->events, &tmp);
        } while (rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == LIBEVDEV_READ_STATUS_SYNC);
    }
    if (!wcdeque_size(&ctx->events)) return -1;
    if (out_ev){
        *out_ev = *(wcinput_event_t*)wcdeque_back(&ctx->events);
        wcdeque_pop_front(&ctx->events);
    }
    return 0;
}

bool wcinput_pop_event(wcinput_t* ctx, wcinput_event_t* out_ev){
    if (!wcdeque_size(&ctx->events)) return false;
    if (out_ev) *out_ev = *(wcinput_event_t*)wcdeque_back(&ctx->events);
    wcdeque_pop_front(&ctx->events);
    return true;
}
