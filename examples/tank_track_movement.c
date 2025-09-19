#include "wc/containers/vec.h"
#include "wc/io/input.h"
#include "wc/io/vesc.h"
#include "wc/io/can.h"

#include <bits/time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DELTA_TIME_MS 25.0f

#define RIGHT_MOTOR_ID_1 121
#define LEFT_MOTOR_ID_1 122

#define RIGHT_ACCEL_DEADZONE 1e-1f
#define LEFT_ACCEL_DEADZONE 1e-1f

#define RIGHT_DUTY_DEADZONE 1e-2f
#define LEFT_DUTY_DEADZONE 1e-2f

#define TRACK_ACCEL 2.0f

wcvec_t* get_filters(){
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
    if (wcvec_init_copy(&abs_filters_vec, abs_filters_raw, sizeof(abs_filters_raw)/sizeof(*abs_filters_raw), sizeof(typeof((struct input_event){0}.code)))) return NULL;
    wcinput_event_filter_t abs_filters = {EV_ABS, abs_filters_vec};

    wcvec_t key_filters_vec;
    if (wcvec_init_copy(&key_filters_vec, key_filters_raw, sizeof(key_filters_raw)/sizeof(*key_filters_raw), sizeof(typeof((struct input_event){0}.code)))) return NULL;
    wcinput_event_filter_t key_filters = {EV_KEY, key_filters_vec};

    wcvec_push_back(&filters, &abs_filters);
    wcvec_push_back(&filters, &key_filters);

    return &filters;
}
void free_filters(wcvec_t* filters){
    for (size_t a = 0; a < wcvec_size(filters); a++){
        wcinput_event_filter_t* filter = wcvec_get(filters, a);
        wcvec_free(&filter->codes);
    }
    wcvec_free(filters);
    memset(filters, 0, sizeof(wcvec_t));
}

float current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float)ts.tv_sec * 1000.0f + (float)ts.tv_nsec / 1000000.0f;
}

int main(){
    wcinput_ctx_t input_ctx;
    wccan_ctx_t can_ctx;
    wccan_frame_t frame;
    wcvec_t* filters;

    filters = get_filters();
    if (!filters){
        printf("%s\n", "Error: Failed to get device filters");
        return -1;
    }
    if (wcinput_ctx_init(&input_ctx) < 0){
        printf("%s\n", "Error: Failed to create input context");
        return -1;
    }
    if (wccan_ctx_init(&can_ctx) < 0){
        printf("Error: Failed to create CAN socket\n");
        return -1;
    }
    if (wccan_ctx_bind(&can_ctx, "can0", NULL) < 0){
        printf("Error: Failed to bind CAN socket\n");
        return -1;
    }

    wcinput_ctx_scan_devices(&input_ctx, filters);
    if (!wcinput_ctx_num_devices(&input_ctx)){
        printf("%s\n", "Warning: No input devices found. Waiting for valid device to connect");
        wcinput_ctx_wait_device(&input_ctx, filters);
        printf("%s\n", "Notice: Valid input device found");
    }
    printf("%s\n", "Notice: Valid input device found");

    wcinput_event_t event;
    int rc;

    float right_duty = 0.0f;
    float left_duty = 0.0f;
    float right_accel = 0.0f;
    float left_accel = 0.0f;

    float frame_start = current_time_ms();
    float frame_end = frame_start;

    bool quit = false;
    while (!quit){
        if (!wcinput_ctx_num_devices(&input_ctx)){
            printf("%s\n", "Warning: No input devices found. Waiting for valid device to connect");
            wcinput_ctx_wait_device(&input_ctx, filters);
            printf("%s\n", "Notice: Valid input device found");
        }

        rc = wcinput_ctx_poll(&input_ctx, NULL);
        while (!wcinput_ctx_pop_event(&input_ctx, &event)){
            if (event.ev.type == EV_DEVDROP){
                printf("Device disconnected\n");
                continue;
            }

            if (event.ev.type == EV_KEY && event.ev.code == BTN_MODE) quit = true;
            if (event.ev.type == EV_ABS){
                float val = wcinput_event_normalized(event, -1.0f, 1.0f);
                if (event.ev.code == ABS_RY) right_accel = val;
                else if (event.ev.code == ABS_Y) left_accel = val;
            }
        }

        frame_end = current_time_ms();
        if (frame_end - frame_start < DELTA_TIME_MS) continue;
        frame_start = frame_end;

        right_accel = fabsf(right_accel) < RIGHT_ACCEL_DEADZONE ? 0.0f : right_accel;
        left_accel = fabsf(left_accel) < LEFT_ACCEL_DEADZONE ? 0.0f : left_accel;
        right_duty += ((-right_accel) - right_duty) * DELTA_TIME_MS*TRACK_ACCEL/1000.0f;
        left_duty += ((-left_accel) - left_duty) * DELTA_TIME_MS*TRACK_ACCEL/1000.0f;
        if (right_accel == 0.0f) right_duty = fabsf(right_duty) < RIGHT_DUTY_DEADZONE ? 0.0f : right_duty;
        if (left_accel == 0.0f) left_duty = fabsf(left_duty) < LEFT_DUTY_DEADZONE ? 0.0f : left_duty;
        right_duty = right_duty < -1.0f ? -1.0f : (right_duty > 1.0f ? 1.0f : right_duty);
        left_duty = left_duty < -1.0f ? -1.0f : (left_duty > 1.0f ? 1.0f : left_duty);

        wcvesc_encode_duty(&frame, RIGHT_MOTOR_ID_1, right_duty);
        wccan_ctx_write_bus(&can_ctx, &frame);
        wcvesc_encode_duty(&frame, LEFT_MOTOR_ID_1, left_duty);
        wccan_ctx_write_bus(&can_ctx, &frame);

        printf("%f\n%f\n\n", left_duty, right_duty);
    }

    wcinput_ctx_free(&input_ctx);
    wccan_ctx_free(&can_ctx);
    free_filters(filters);
    return 0;
}
