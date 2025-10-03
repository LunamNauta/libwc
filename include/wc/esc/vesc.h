#ifndef WC_VESC_HEADER
#define WC_VESC_HEADER

#include "wc/io/can.h"

#include <stdint.h>

#include <linux/can/raw.h>
#include <linux/can.h>
#include <endian.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

// Reversed engineered from the VESC firmware: https://github.com/vedderb/bldc
//------------------------------------------------------------------------------------

// There are other commands, but these seem like the most useful for basic functionality
// Firmware implementation: https://github.com/vedderb/bldc/blob/master/datatypes.h#L1134
typedef enum wcvesc_packet_id{
    WCVESC_SET_DUTY                     = 0x00,
    WCVESC_SET_CURRENT                  = 0x01,
    WCVESC_SET_CURRENT_BRAKE            = 0x02,
    WCVESC_SET_RPM                      = 0x03,
    WCVESC_SET_POS                      = 0x04,
    WCVESC_FILL_RX_BUFFER               = 0x05,
    WCVESC_FILL_RX_BUFFER_LONG          = 0x06,
    WCVESC_PROCESS_RX_BUFFER            = 0x07,
    WCVESC_PROCESS_SHORT_BUFFER         = 0x08,
    WCVESC_STATUS_1                     = 0x09,
    WCVESC_SET_CURRENT_REL              = 0x0a,
    WCVESC_SET_CURRENT_BRAKE_REL        = 0x0b,
    WCVESC_SET_CURRENT_HANDBRAKE        = 0x0c,
    WCVESC_SET_CURRENT_HANDBRAKE_REL    = 0x0d,
    WCVESC_STATUS_2                     = 0x0e,
    WCVESC_STATUS_3                     = 0x0f,
    WCVESC_STATUS_4                     = 0x10,
    WCVESC_PING                         = 0x11,
    WCVESC_PONG                         = 0x12,
    WCVESC_DETECT_APPLY_ALL_FOC         = 0x13,
    WCVESC_DETECT_APPLY_ALL_FOC_RES     = 0x14,
    WCVESC_CONF_CURRENT_LIMITS          = 0x15,
    WCVESC_CONF_STORE_CURRENT_LIMITS    = 0x16,
    WCVESC_CONF_CURRENT_LIMITS_IN       = 0x17,
    WCVESC_CONF_STORE_CURRENT_LIMITS_IN = 0x18,
    WCVESC_CONF_FOC_ERPMS               = 0x19,
    WCVESC_CONF_STORE_FOC_ERPMS         = 0x1a,
    WCVESC_STATUS_5                     = 0x1b,
    WCVESC_SHUTDOWN                     = 0x1f,
    WCVESC_NOTIFY_BOOT                  = 0x39,
    WCVESC_STATUS_6                     = 0x3a
} wcvesc_packet_id_t;

//------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------

canid_t wcvesc_encode_id(wcvesc_packet_id_t packet_id, uint8_t unit_id);
void wcvesc_decode_id(canid_t id, wcvesc_packet_id_t* out_packet_id, uint8_t* out_unit_id);

bool wcvesc_has_packet_id(canid_t id, wcvesc_packet_id_t packet_id);
bool wcvesc_has_unit_id(canid_t id, uint8_t unit_id);

//------------------------------------------------------------------------------------

void wcvesc_encode_conf_store_current_limits_in(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_conf_store_current_limits(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr);
void wcvesc_encode_set_current_brake_rel(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_set_current_brake(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_set_current_rel(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_set_current(wccan_frame_t* frame, uint8_t unit_id, float current);
void wcvesc_encode_set_duty(wccan_frame_t* frame, uint8_t unit_id, float duty);
void wcvesc_encode_set_rpm(wccan_frame_t* frame, uint8_t unit_id, float rpm);
void wcvesc_encode_set_pos(wccan_frame_t* frame, uint8_t unit_id, float pos);

//------------------------------------------------------------------------------------

wcvesc_status1_t wcvesc_decode_status1(const wccan_frame_t* frame);
wcvesc_status2_t wcvesc_decode_status2(const wccan_frame_t* frame);
wcvesc_status3_t wcvesc_decode_status3(const wccan_frame_t* frame);
wcvesc_status4_t wcvesc_decode_status4(const wccan_frame_t* frame);
wcvesc_status5_t wcvesc_decode_status5(const wccan_frame_t* frame);
wcvesc_status6_t wcvesc_decode_status6(const wccan_frame_t* frame);

//-------------------------------------------------------------------------------------

#endif
