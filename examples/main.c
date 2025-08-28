#include "wc/containers/vec.h"
#include "wc/io/input.h"

#include <linux/input-event-codes.h>
#include <stdio.h>

const wcvec_t* get_filters(){
    static wcvec_t filters = {0};
    typeof((struct input_event){0}.code) abs_filters_raw[] = {
        ABS_X, ABS_Y, ABS_Z,
        ABS_RX, ABS_RY, ABS_RZ,
        ABS_HAT0X, ABS_HAT0Y
    };
    typeof((struct input_event){0}.code) key_filters_raw[] = {
        BTN_NORTH, BTN_SOUTH, BTN_EAST, BTN_WEST,
        BTN_TL, BTN_TR,
        BTN_MODE,
        BTN_START, BTN_SELECT,
        BTN_THUMBL, BTN_THUMBR
    };
    if (wcvec_data(&filters)) return &filters;
    wcvec_init(&filters, sizeof(wcinput_event_filter_t));
    if (wcvec_reserve(&filters, 2)) return NULL;

    wcvec_t abs_filters_vec;
    if (wcvec_init_copy(&abs_filters_vec, abs_filters_raw, sizeof(typeof((struct input_event){0}.code)), sizeof(abs_filters_raw)/sizeof(*abs_filters_raw))) return NULL;
    wcinput_event_filter_t abs_filters = {EV_ABS, abs_filters_vec};

    wcvec_t key_filters_vec;
    if (wcvec_init_copy(&key_filters_vec, key_filters_raw, sizeof(typeof((struct input_event){0}.code)), sizeof(key_filters_raw)/sizeof(*key_filters_raw))) return NULL;
    wcinput_event_filter_t key_filters = {EV_KEY, key_filters_vec};

    wcvec_push_back(&filters, &abs_filters);
    wcvec_push_back(&filters, &key_filters);

    return &filters;
}

int main(){
    const wcvec_t* filters = get_filters();
    if (!filters){
        printf("%s\n", "Error: Failed to get device filters");
        return -1;
    }
    wcinput_ctx_t ctx;
    wcinput_ctx_init(&ctx);
    wcinput_ctx_find_devices(&ctx, filters);
    if (!wcvec_size(wcinput_ctx_devices(&ctx))){
        printf("%s\n", "Error: Failed to find any input devices");
        return -1;
    }

    wcinput_event_t event;
    int rc;

    while (true){
        rc = wcinput_ctx_poll(&ctx, &event);
        if (rc) continue;

        if (event.ev.type == EV_KEY && event.ev.code == BTN_MODE && !event.ev.value) break;
        float val = wcinput_event_normalized(event, -1.0f, 1.0f);
        const wcque_t* que = &ctx.events;
        printf("%s -> %s %s %f\n",
            libevdev_get_name(event.dev->dev),
            libevdev_event_type_get_name(event.ev.type),
            libevdev_event_code_get_name(event.ev.type, event.ev.code),
            val
        );
    }

    wcinput_ctx_free(&ctx);
    return 0;
}
