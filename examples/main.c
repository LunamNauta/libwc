#include "wc/container/vector.h"
#include "wc/hardware/gpio.h"
#include "wc/hardware/pwm.h"
#include "wc/hardware/can.h"
#include "wc/input/input.h"
#include "wc/esc/vesc.h"

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <ncurses.h>

//-----------------------------------------------------------------------------------------

#define RIGHT_MOTOR_ID_1 121
#define RIGHT_MOTOR_ID_2
#define LEFT_MOTOR_ID_1 122
#define LEFT_MOTOR_ID_2

#define TRACK_VEL_MAX 0.8f
#define TRACK_DEADZONE 0.1f
#define TRACK_ACCEL 0.5f

#define SERVO_DIST_FLAT 0.1f
#define SERVO_DIST_MAX 0.7f
#define SERVO_DIST_MIN 0.0f
#define SERVO_ACC_SCL 0.25f

#define FIRE_OPEN_TIME 500
#define FIRE_REFILL_TIME 500

#define KONAMI_RESET_MS 1000

#define UPDATE_RATE_MS 16

//-----------------------------------------------------------------------------------------

float current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float)ts.tv_sec * 1000.0f + (float)ts.tv_nsec / 1000000.0f;
}

//-----------------------------------------------------------------------------------------

typedef enum log_level{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE
} log_level_t;

FILE* log_file;
size_t error_count = 0;
size_t warning_count = 0;
size_t notice_count = 0;
void write_log_file(log_level_t level, const char* format, ...){
    va_list args;
    va_start(args, format);
    fprintf(log_file, "%.3f: ", current_time_ms());
    if (level == LOG_LEVEL_ERROR){
        fprintf(log_file, "Error: ");
        error_count++;
    }
    else if (level == LOG_LEVEL_WARNING){
        fprintf(log_file, "Warning: ");
        warning_count++;
    }
    else if (level == LOG_LEVEL_NOTICE){
        fprintf(log_file, "Notice: ");
        notice_count++;
    }
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    fflush(log_file);
    va_end(args);
}

wcvector_t* get_filters(){
    static wcvector_t filters = {0};
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
    if (wcvector_capacity(&filters)) return &filters;
    wcvector_init(&filters, sizeof(wcinput_event_filter_t));
    if (wcvector_reserve(&filters, 2) < 0) return NULL;

    wcvector_t abs_filters_vec;
    wcvector_init(&abs_filters_vec, sizeof(typeof(*abs_filters_raw)));
    if (wcvector_resize(&abs_filters_vec, sizeof(abs_filters_raw)/sizeof(*abs_filters_raw)) < 0) return NULL;
    memcpy(wcvector_front(&abs_filters_vec), abs_filters_raw, sizeof(abs_filters_raw));
    wcinput_event_filter_t abs_filters = {EV_ABS, abs_filters_vec};

    wcvector_t key_filters_vec;
    wcvector_init(&key_filters_vec, sizeof(typeof(*key_filters_raw)));
    if (wcvector_resize(&key_filters_vec, sizeof(key_filters_raw)/sizeof(*key_filters_raw)) < 0) return NULL;
    memcpy(wcvector_front(&key_filters_vec), key_filters_raw, sizeof(key_filters_raw));
    wcinput_event_filter_t key_filters = {EV_KEY, key_filters_vec};

    wcvector_push_back(&filters, &abs_filters);
    wcvector_push_back(&filters, &key_filters);

    return &filters;
}
void free_filters(wcvector_t* filters){
    for (size_t a = 0; a < wcvector_size(filters); a++){
        wcinput_event_filter_t* filter = wcvector_get(filters, a);
        wcvector_free(&filter->codes);
    }
    wcvector_free(filters);
    memset(filters, 0, sizeof(wcvector_t));
}

wcvector_t* filters;
bool module_filters = false;
int init_filters_module(){
    filters = get_filters();
    if (!filters){
        write_log_file(LOG_LEVEL_ERROR, "Failed to get device filters");
        return -1;
    }
    module_filters = true;
    return 0;
}

wcinput_t input_ctx;
bool module_input = false;
int init_input_module(){
    if (init_filters_module() < 0) return -1;
    if (wcinput_init(&input_ctx) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to create input context");
        return -1;
    }
    wcinput_scan_devices(&input_ctx, filters);
    if (!wcinput_num_devices(&input_ctx)){
        write_log_file(LOG_LEVEL_WARNING, "No input devices found. Waiting for valid device to connect");
        wcinput_wait_device(&input_ctx, filters);
        write_log_file(LOG_LEVEL_NOTICE, "Valid input device found");
    }
    write_log_file(LOG_LEVEL_NOTICE, "Valid input device found");
    module_input = true;
    return 0;
}

wccan_t can_ctx;
wccan_frame_t frame;
bool module_can = false;
int init_can_module(){
    if (wccan_init(&can_ctx) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to create CAN socket");
        return -1;
    }
    if (wccan_bind(&can_ctx, "can0") < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to bind CAN socket to \"can0\"");
        return -1;
    }
    module_can = true;
    return 0;
}

wcpwm_chip_t pwm_chip;
bool module_pwm = false;
int init_pwm_module(){
    if (wcpwm_chip_init(&pwm_chip, 0) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to initialize PWM-chip 0");
        return -1;
    }
    if (wcpwm_pin_init(&pwm_chip, 0) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to initialize pin 0 (on PWM-chip 0)");
        return -1;
    }
    if (wcpwm_set_period(&pwm_chip, 0, 1900000) < 0){
    	write_log_file(LOG_LEVEL_ERROR, "Failed to set pin 0's (on PWM-chip 0) period");
        return -1;
    }
    if (wcpwm_set_duty(&pwm_chip, 0, 1100000) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to set pin 0's (on PWM-chip 0) duty cycle");
        return -1;
    }
    if (wcpwm_set_enabled(&pwm_chip, 0, true) < 0){
    	write_log_file(LOG_LEVEL_ERROR, "Failed to enable pin 0 (on PWM-chip 0)");
        return -1;
    }
    module_pwm = true;
    return 0;
}

wcgpio_chip_t gpio_chip;
wcgpio_line_req_t req;
wcgpio_line_cfg_t cfg;
bool module_gpio_1 = false;
bool module_gpio_2 = false;
int init_gpio_module(){
    wcgpio_line_vals_t vals;

    wcgpio_line_cfg_zero(&cfg);
    wcgpio_line_req_zero(&req);
    wcgpio_line_vals_zero(&vals);

    if (wcgpio_chip_init(&gpio_chip, 0) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to initialize GPIO-chip 0");
        return -1;
    }
    module_gpio_1 = true;
    wcgpio_line_cfg_add_attr(
        &cfg,
        WCGPIO_LINE_ATTR_FLAGS,
        WCGPIO_LINE_FLAG_OUTPUT | WCGPIO_LINE_FLAG_BIAS_PULL_DOWN, 
        WCGPIO_PIN_MASK(0, 1)
    );
    wcgpio_line_cfg_add_attr(
        &cfg,
        WCGPIO_LINE_ATTR_OUTPUT_VALUES,
        WCGPIO_PIN_LOW_MASK(1) | WCGPIO_PIN_HIGH_MASK(0),
        WCGPIO_PIN_MASK(0, 1)
    );
    wcgpio_line_req_set_offsets(&req, WCGPIO_PIN_MASK(0, 1));
    wcgpio_line_req_set_consumer(&req, "solenoid_flipper_gpio");
    wcgpio_line_req_set_config(&req, &cfg);
    if (wcgpio_line_req_init(&req, &gpio_chip) < 0){
        write_log_file(LOG_LEVEL_ERROR, "Failed to setup GPIO pins");
        return -1;
    }
    module_gpio_2 = true;
    return 0;
}

int init_tshirt_tank(FILE* lf){
    log_file = lf;
    //if (init_gpio_module() < 0) return -1;
    //if (init_pwm_module() < 0) return -1;
    //if (init_can_module() < 0) return -1;
    if (init_input_module() < 0) return -1;
    return 0;
}

void free_tshirt_tank(){
    if (module_filters) free_filters(filters);
    if (module_input) wcinput_free(&input_ctx);
    if (module_can) wccan_free(&can_ctx);
    if (module_pwm) wcpwm_chip_free(&pwm_chip);
    if (module_gpio_1) wcgpio_chip_free(&gpio_chip);
    if (module_gpio_2) wcgpio_line_req_free(&req);
}

//-----------------------------------------------------------------------------------------

int konami_cannon_lock[] = {
    EV_ABS, ABS_HAT0X, -1,
    EV_ABS, ABS_HAT0X, 1,
    EV_ABS, ABS_HAT0X, -1,
    EV_ABS, ABS_HAT0X, 1,
    EV_KEY, BTN_B, 1,
    EV_KEY, BTN_Y, 1,
    EV_KEY, BTN_START, 1
};
size_t cannon_lock_state = 0;
bool cannon_locked = false;
float cannon_lock_time = 0.0f;

int konami_track_lock[] = {
    EV_ABS, ABS_HAT0X, -1,
    EV_ABS, ABS_HAT0X, 1,
    EV_ABS, ABS_HAT0X, -1,
    EV_ABS, ABS_HAT0X, 1,
    EV_KEY, BTN_Y, 1,
    EV_KEY, BTN_B, 1,
    EV_KEY, BTN_START, 1
};
size_t track_lock_state = 0;
bool track_locked = false;
float track_lock_time = 0.0f;

float right_vel_act = 0.0f;
float right_vel_target = 0.0f;

float left_vel_act = 0.0f;
float left_vel_target = 0.0f;

float servo_dist = SERVO_DIST_FLAT;
float servo_vel = 0.0f;

size_t fire_state = 0;
float fire_time = 0;

//-----------------------------------------------------------------------------------------

int update_cannon(float time){
    if (!fire_state) return 0;
    wcgpio_line_vals_t vals;
    wcgpio_line_vals_zero(&vals);
    wcgpio_line_vals_set_mask(&vals, WCGPIO_PIN_MASK(0, 1));
    if (fire_state == 1){
        wcgpio_line_vals_set_bits(&vals, WCGPIO_PIN_LOW_MASK(0) | WCGPIO_PIN_HIGH_MASK(1));
        fire_time = time;
    }
    else if (fire_state == 2){
        if (time - fire_time < FIRE_OPEN_TIME) return 0;
        wcgpio_line_vals_set_bits(&vals, WCGPIO_PIN_LOW_MASK(1) | WCGPIO_PIN_HIGH_MASK(0));
        fire_time = time;
    }
    else if (fire_state == 3){
        if (time - fire_time < FIRE_REFILL_TIME) return 0;
        fire_time = time;
    }
    else assert(false);
    /*
    if (wcgpio_line_vals_set_request(&vals, &req) < 0){
        write_log_file(LOG_LEVEL_WARNING, "Failed to set GPIO pin states");
        return -1;
    }
    */
    fire_state = (fire_state + 1) % 4;
    return 0;
}

int update_pitch(float delta_time){
    if (cannon_locked) servo_dist = SERVO_DIST_FLAT;
    else servo_dist = fmaxf(SERVO_DIST_MIN, fminf(servo_dist + servo_vel * delta_time, SERVO_DIST_MAX));
    /*
    if (wcpwm_set_duty(&pwm_chip, 0, 1100000.0f+800000.0f*servo_dist) < 0){
        write_log_file(LOG_LEVEL_WARNING, "Failed to update pitch");
        return -1;
    }
    */
    return 0;
}

int update_tracks(float delta_time){
    if (track_locked){
        right_vel_target = 0.0f;
        left_vel_target = 0.0f;
    }
    right_vel_act += (-right_vel_target - right_vel_act)*delta_time*TRACK_ACCEL;
    left_vel_act += (-left_vel_target - left_vel_act)*delta_time*TRACK_ACCEL;
    if (right_vel_target == 0.0f && right_vel_act < TRACK_DEADZONE) right_vel_act = 0.0f;
    if (left_vel_target == 0.0f && left_vel_act < TRACK_DEADZONE) left_vel_act = 0.0f;
    /*
    wcvesc_encode_duty(&frame, RIGHT_MOTOR_ID_1, right_vel_act);
    if (wccan_write_bus(&can_ctx, &frame) < 0){
        write_log_file(LOG_LEVEL_WARNING, "Failed to set duty cycle for motor 1 (on right track)");
        return -1;
    }
    */
    /*
    wcvesc_encode_duty(&frame, RIGHT_MOTOR_ID_2, right_vel_act);
    if (wccan_write_bus(&can_ctx, &frame) < 0){
        printf(LOG_LEVEL_WARNING, "Failed to set duty cycle for motor 2 (on right track)");
        return -1;
    }
    */
    /*
    wcvesc_encode_duty(&frame, LEFT_MOTOR_ID_1, left_vel_act);
    if (wccan_write_bus(&can_ctx, &frame) < 0){
        printf(LOG_LEVEL_WARNING, "Failed to set duty cycle for motor 1 (on left track)");
        return -1;
    }
    */
    /*
    wcvesc_encode_duty(&frame, LEFT_MOTOR_ID_2, left_vel_act);
    if (wccan_write_bus(&can_ctx, &frame) < 0){
        printf(LOG_LEVEL_WARNING, "Failed to set duty cycle for motor 2 (on left track)");
        return -1;
    }
    */
    return 0;
}

bool update_cannon_lock(float time, int type, int code, int value){
    if (time - cannon_lock_time > KONAMI_RESET_MS) cannon_lock_state = 0;
    if (type != konami_cannon_lock[cannon_lock_state]) return false;
    if (code != konami_cannon_lock[cannon_lock_state + 1]) return false;
    if (value != konami_cannon_lock[cannon_lock_state + 2]) return false;
    if (cannon_lock_state == (sizeof(konami_cannon_lock)/sizeof(*konami_cannon_lock) - 3)){
        cannon_locked = !cannon_locked;
        cannon_lock_time = 0.0f;
        cannon_lock_state = 0;
    }
    else cannon_lock_state += 3;
    cannon_lock_time = time;
    return true;
}

bool update_track_lock(float time, int type, int code, int value){
    if (time - track_lock_time > KONAMI_RESET_MS) track_lock_state = 0;
    if (type != konami_track_lock[track_lock_state]) return false;
    if (code != konami_track_lock[track_lock_state + 1]) return false;
    if (value != konami_track_lock[track_lock_state + 2]) return false;
    if (track_lock_state == (sizeof(konami_track_lock)/sizeof(*konami_track_lock) - 3)){
        track_locked = !track_locked;
        track_lock_time = 0.0f;
        track_lock_state = 0;
    }
    else track_lock_state += 3;
    track_lock_time = time;
    return true;
}

//-----------------------------------------------------------------------------------------

int main(int argc, char** argv){
    if (argc != 2){
        printf("Please pass log file path\n");
        return -1;
    }
    FILE* log = fopen(argv[1], "a");
    if (!log){
        printf("Provided path was not a file\n");
        return -1;
    }

    if (init_tshirt_tank(log)){
        free_tshirt_tank();
        return -1;
    }

    wcinput_event_t event;
    int rc;

    float frame_start = 0.0f;
    float frame_end = 0.0f;

    initscr();

    bool quit = false;
    while (!quit){
        frame_end = current_time_ms();
        float delta_time_ms = frame_end - frame_start;
        if (delta_time_ms < UPDATE_RATE_MS) continue;
        float delta_time = delta_time_ms / 1000.0f;
        frame_start = frame_end;

        if (!wcinput_num_devices(&input_ctx)){
            if (right_vel_target != 0.0f || left_vel_target != 0.0f || servo_vel != 0.0f) write_log_file(LOG_LEVEL_WARNING, "No input devices found. Resetting system");
            right_vel_target = 0.0f;
            left_vel_target = 0.0f;
            servo_vel = 0.0f;
            if (!fire_state && right_vel_act == 0.0f && left_vel_act == 0.0f && servo_dist == SERVO_DIST_FLAT){
                write_log_file(LOG_LEVEL_NOTICE, "Searching for new input device");
                wcinput_scan_devices(&input_ctx, filters);
                if (!wcinput_num_devices(&input_ctx)){
                    write_log_file(LOG_LEVEL_NOTICE, "No input device found. Waiting for new input device");
                    wcinput_wait_device(&input_ctx, filters);
                }
                write_log_file(LOG_LEVEL_NOTICE, "Valid input device found");
            }
            servo_dist = SERVO_DIST_FLAT;
        }

        rc = wcinput_poll_events(&input_ctx, NULL);
        memset(&event, 0, sizeof(wcinput_event_t));
        while (wcinput_pop_event(&input_ctx, &event)){
            if (wcinput_event_type(event) == EV_DEVDROP) break;
            bool cannon_lock_updated = update_cannon_lock(frame_start, wcinput_event_type(event), wcinput_event_code(event), wcinput_event_value(event));
            bool track_lock_updated = update_track_lock(frame_start, wcinput_event_type(event), wcinput_event_code(event), wcinput_event_value(event));
            if (cannon_lock_updated || track_lock_updated) continue;
            if (wcinput_event_type(event) == EV_KEY){
                if (wcinput_event_code(event) == BTN_A){
                    if (!cannon_locked && !fire_state) fire_state = 1;
                }
                else if (wcinput_event_code(event) == BTN_TL){
                    servo_dist = SERVO_DIST_FLAT;
                }
            }
            if (wcinput_event_type(event) == EV_ABS){
                float val = wcinput_event_normalized(event, -1.0f, 1.0f);
                if (wcinput_event_code(event) == ABS_RY){
                    right_vel_target = fabsf(val) < TRACK_DEADZONE ? 0.0f : val;
                }
                else if (wcinput_event_code(event) == ABS_Y){
                    left_vel_target = fabsf(val) < TRACK_DEADZONE ? 0.0f : val;
                }
                else if (wcinput_event_code(event) == ABS_HAT0Y){
                    servo_vel = SERVO_ACC_SCL*(wcinput_event_value(event) < 0.0f ? 1.0f : (wcinput_event_value(event) > 0.0f ? -1.0f : 0.0f));
                }
            }
        }
        if (wcinput_event_type(event) == EV_DEVDROP){
            while (wcinput_pop_event(&input_ctx, NULL));
            continue;
        }

        // TODO: Figure out appropriate responses to failure
        if (update_cannon(frame_start) < 0);
        if (update_pitch(delta_time) < 0);
        if (update_tracks(delta_time) < 0);

        clear();
        mvprintw(
            0, 0,
            "Cannon: [ Lock: %s, Pitch: %.3f, Status: %s ]",
            cannon_locked ? "Locked" : "Unlocked",
            servo_dist,
            fire_state == 0 ? "Ready" : (fire_state == 2 ? "Firing" : (fire_state == 3 ? "Refilling" : "Ready"))
        );
        mvprintw(
            1, 0,
            "Tracks: [ Lock: %s, Left: %s%.3f, Right: %s%.3f]",
            track_locked ? "Locked" : "Unlocked",
            left_vel_act >= 0.0f ? " " : "", left_vel_act,
            right_vel_act >= 0.0f ? " " : "", right_vel_act
        );
        mvprintw(
            2, 0, "Logs:   [ Errors: %zu, Warnings: %zu, Notices: %zu]",
            error_count,
            warning_count,
            notice_count
        );
        refresh();
    }


    free_tshirt_tank();
    return 0;
}
