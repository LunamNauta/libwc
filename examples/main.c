#include "wc/io/input.h"

#include <stdio.h>

int main(){
    wcinput_ctx_t ctx;
    wcinput_ctx_init(&ctx);
    wcinput_ctx_find_devices(&ctx);
    if (!wcvec_size(&ctx.devices)){
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
