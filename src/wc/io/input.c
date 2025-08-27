#include "wc/io/input.h"

#include <string.h>
#include <errno.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

float wcinput_event_normalized(wcinput_event_t event, float out_min, float out_max){
    if (event.ev.type == EV_KEY){
        if (event.ev.value) return out_max;
        else return out_min;
    }
    if (event.ev.type != EV_ABS) return event.ev.value;
    int in_max = libevdev_get_abs_maximum(event.dev->dev, event.ev.code);
    int in_min = libevdev_get_abs_minimum(event.dev->dev, event.ev.code);
    int in_range = in_max - in_min;
    int out_range = out_max - out_min;
    return ((float)event.ev.value - in_min)*out_range / in_range + out_min;
}

int wcinput_ctx_init(wcinput_ctx_t* ctx){
    if (wcvec_init(&ctx->devices, sizeof(wcinput_device_t))) return -1;
    if (wcque_init(&ctx->events, sizeof(wcinput_event_t))) return -1;
    return 0;
}
void wcinput_ctx_free(wcinput_ctx_t* ctx){
    wcvec_free(&ctx->devices);
    wcque_free(&ctx->events);
}

int wcinput_ctx_find_devices(wcinput_ctx_t* ctx){
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
        if (rc >= 0){
            wcinput_device_t tmp = {dev, fd};
            wcvec_push_back(&ctx->devices, &tmp);
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
        do{
            rc = libevdev_next_event(((wcinput_device_t*)wcvec_get(&ctx->devices, a))->dev, LIBEVDEV_READ_FLAG_NORMAL, &event);
            if (rc == -EAGAIN || rc == LIBEVDEV_READ_STATUS_SYNC) continue;
            if (event.type == EV_SYN) continue;
            wcinput_event_t tmp = {wcvec_get(&ctx->devices, a), event};
            wcque_push(&ctx->events, &tmp);
        } while (rc != -EAGAIN && (rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == LIBEVDEV_READ_STATUS_SYNC));
    }
    if (!wcque_size(&ctx->events)) return -1;
    if (out){
        *out = *(wcinput_event_t*)wcque_back(&ctx->events);
        wcque_pop(&ctx->events);
    }
    return 0;
}
