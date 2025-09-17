#include "wc/io/input.h"
#include "wc/containers/que.h"
#include "wc/containers/vec.h"

#include <string.h>
#include <errno.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <libevdev-1.0/libevdev/libevdev.h>

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max){
    if (event.ev.type == EV_KEY){
        if (event.ev.value) return out_max;
        else return out_min;
    }
    if (event.ev.type != EV_ABS) return event.ev.value;
    int flat = libevdev_get_abs_flat(event.dev->dev, event.ev.code);
    int in_max = libevdev_get_abs_maximum(event.dev->dev, event.ev.code) - flat;
    int in_min = libevdev_get_abs_minimum(event.dev->dev, event.ev.code) + flat;
    int in_range = in_max - in_min;
    int out_range = out_max - out_min;
    float value = event.ev.value < 0 ? event.ev.value + flat : event.ev.value - flat;
    return (value - in_min)*out_range/in_range + out_min;
}

int wcinput_ctx_init(wcinput_ctx_t* ctx){
    wcvec_init(&ctx->devices, sizeof(wcinput_device_t));
    wcque_init(&ctx->events, sizeof(wcinput_event_t));
    if (wcque_reserve(&ctx->events, WC_INPUT_MAX_EVENTS)) return -1;
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

int wcinput_ctx_find_devices(wcinput_ctx_t* ctx, const wcvec_t* filters){
    struct dirent* entry;
    DIR* folder;
    int rc = 1;
    int fd;

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

        fd = open(file_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        struct libevdev* dev;
        rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) continue;
        
        bool valid = true;
        if (!filters) goto add_to_vec;
        for (size_t a = 0; valid && a < wcvec_size(filters); a++){
            const wcinput_event_filter_t* filter = wcvec_get(filters, a);
            if (!libevdev_has_event_type(dev, filter->type)){
                valid = false;
                break;
            }
            for (size_t b = 0; b < wcvec_size(&filter->codes); b++){
                const typeof((struct input_event){0}.code)* code = wcvec_get(&filter->codes, b);
                if (!libevdev_has_event_code(dev, filter->type, *code)){
                    valid = false;
                    break;
                }
            }
        }
        add_to_vec:
        if (valid){
            wcinput_device_t tmp = {dev, fd};
            wcvec_push_back(&ctx->devices, &tmp);
        }
        else{
            libevdev_free(dev);
            close(fd);
        }
    }

    closedir(folder);
    return 0;
}

int wcinput_ctx_poll(wcinput_ctx_t* ctx, wcinput_event_t* out){
    struct input_event event;
    int rc;

    wcinput_event_t tmp;
    for (size_t a = 0; a < wcvec_size(&ctx->devices); a++){
        wcinput_device_t* device = wcvec_get(&ctx->devices, a);
        do{
            rc = libevdev_next_event(device->dev, LIBEVDEV_READ_FLAG_NORMAL, &event);
            if (rc == -EAGAIN || rc == LIBEVDEV_READ_STATUS_SYNC) continue;
            if (rc == -ENODEV){
                libevdev_free(device->dev);
                close(device->fd);
                device->dev = NULL;
                device->fd = -1;
                event.type = 1;
                wcinput_event_t tmp = {device, event, true};
                wcque_push_back_rot(&ctx->events, &tmp);
                wcvec_erase(&ctx->devices, a--);
                break;
            }
            if (event.type == EV_SYN) continue;
            wcinput_event_t tmp = {device, event, false};
            wcque_push_back_rot(&ctx->events, &tmp);
        } while (rc != -EAGAIN && (rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == LIBEVDEV_READ_STATUS_SYNC));
    }
    if (!wcque_size(&ctx->events)) return -1;
    if (out){
        *out = *(wcinput_event_t*)wcque_back(&ctx->events);
        wcque_pop_front(&ctx->events);
    }
    return 0;
}
