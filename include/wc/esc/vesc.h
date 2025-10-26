#ifndef WC_VESC_HEADER
#define WC_VESC_HEADER

#include "wc/hardware/can.h"

#include <stdint.h>

#include <linux/can/raw.h>
#include <linux/can.h>
#include <endian.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

// Reverse engineered from: https://github.com/vedderb/bldc/blob/master/comm/comm_can.c
//------------------------------------------------------------------------------------

typedef enum wcvesc_packet_id{
    WCVESC_SET_DUTY = 0,
    WCVESC_SET_CURRENT,
    WCVESC_SET_CURRENT_BRAKE,
    WCVESC_SET_RPM,
    WCVESC_SET_POS,
    WCVESC_FILL_RX_BUFFER,
    WCVESC_FILL_RX_BUFFER_LONG,
    WCVESC_PROCESS_RX_BUFFER,
    WCVESC_PROCESS_SHORT_BUFFER,
    WCVESC_STATUS_1,
    WCVESC_SET_CURRENT_REL,
    WCVESC_SET_CURRENT_BRAKE_REL,
    WCVESC_SET_CURRENT_HANDBRAKE,
    WCVESC_SET_CURRENT_HANDBRAKE_REL,
    WCVESC_STATUS_2,
    WCVESC_STATUS_3,
    WCVESC_STATUS_4,
    WCVESC_PING,
    WCVESC_PONG,
    WCVESC_DETECT_APPLY_ALL_FOC,
    WCVESC_DETECT_APPLY_ALL_FOC_RES,
    WCVESC_CONF_CURRENT_LIMITS,
    WCVESC_CONF_STORE_CURRENT_LIMITS,
    WCVESC_CONF_CURRENT_LIMITS_IN,
    WCVESC_CONF_STORE_CURRENT_LIMITS_IN,
    WCVESC_CONF_FOC_ERPMS,
    WCVESC_CONF_STORE_FOC_ERPMS,
    WCVESC_STATUS_5,
    WCVESC_STATUS_6
} wcvesc_packet_id_t;

typedef struct wcvesc_status1{
    float erpm;
    float current;
    float duty_cycle;
} wcvesc_status1_t;

typedef struct wcvesc_status2{
    float amp_hours;
    float amp_hours_charged;
} wcvesc_status2_t;

typedef struct wcvesc_status3{
    float watt_hours;
    float watt_hours_charged;
} wcvesc_status3_t;

typedef struct wcvesc_status4{
    float temp_fet;
    float temp_motor;
    float current_in;
    float pid_pos;
} wcvesc_status4_t;

typedef struct wcvesc_status5{
    float tachometer;
    float volts_in;
} wcvesc_status5_t;

typedef struct wcvesc_status6{
    float adc1;
    float adc2;
    float adc3;
    float ppm;
} wcvesc_status6_t;

typedef struct wcvesc_status{
    uint8_t type;
    union{
        wcvesc_status1_t s1;
        wcvesc_status2_t s2;
        wcvesc_status3_t s3;
        wcvesc_status4_t s4;
        wcvesc_status5_t s5;
        wcvesc_status6_t s6;
    };
} wcvesc_status_t;

canid_t wcvesc_encode_id(wcvesc_packet_id_t packet_id, uint8_t unit_id);
void wcvesc_decode_id(canid_t id, wcvesc_packet_id_t* out_packet_id, uint8_t* out_unit_id);

void wcvesc_push_u16(uint8_t* data, uint16_t uval);
void wcvesc_push_u32(uint8_t* data, uint32_t uval);
void wcvesc_push_i16(uint8_t* data, int16_t val);
void wcvesc_push_i32(uint8_t* data, int32_t val);
void wcvesc_push_f16(uint8_t* data, float val, float scale);
void wcvesc_push_f32(uint8_t* data, float val, float scale);

uint16_t wcvesc_pop_u16(const uint8_t* data);
uint32_t wcvesc_pop_u32(const uint8_t* data);
int16_t wcvesc_pop_i16(const uint8_t* data);
int32_t wcvesc_pop_i32(const uint8_t* data);
float wcvesc_pop_f16(const uint8_t* data, float scale);
float wcvesc_pop_f32(const uint8_t* data, float scale);

void wcvesc_encode_conf_store_current_limits_in(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_conf_store_current_limits(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_conf_current_limits_in(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_conf_current_limits(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_current_brake_rel(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_current_brake(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_current_rel(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_current(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_erpm(wccan_frame_t* frame, uint8_t unit_id, float erpm);
void wcvesc_encode_duty(wccan_frame_t* frame, uint8_t unit_id, float duty);
void wcvesc_encode_pos(wccan_frame_t* frame, uint8_t unit_id, float pos);

wcvesc_status1_t wcvesc_decode_status1(const wccan_frame_t* frame);
wcvesc_status2_t wcvesc_decode_status2(const wccan_frame_t* frame);
wcvesc_status3_t wcvesc_decode_status3(const wccan_frame_t* frame);
wcvesc_status4_t wcvesc_decode_status4(const wccan_frame_t* frame);
wcvesc_status5_t wcvesc_decode_status5(const wccan_frame_t* frame);
wcvesc_status6_t wcvesc_decode_status6(const wccan_frame_t* frame);

#endif
