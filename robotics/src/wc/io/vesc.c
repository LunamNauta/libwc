#include "wc/io/vesc.h"

#include <string.h>

//------------------------------------------------------------------------------------

void wcvesc_push_u16(uint8_t* data, uint16_t uval){
    data[0] = uval >> 8;
    data[1] = uval;
}
void wcvesc_push_u32(uint8_t* data, uint32_t uval){
    data[0] = uval >> 24;
    data[1] = uval >> 16;
    data[2] = uval >> 8;
    data[3] = uval;
}
void wcvesc_push_i16(uint8_t* data, int16_t val){
    uint16_t uval = val >= 0 ? val : (~((uint16_t)(-val))+1);
    wcvesc_push_u16(data, uval);
}
void wcvesc_push_i32(uint8_t* data, int32_t val){
    uint32_t uval = val >= 0 ? val : (~((uint32_t)(-val))+1);
    wcvesc_push_u32(data, uval);
}
void wcvesc_push_f16(uint8_t* data, float val, float scale){
    wcvesc_push_i16(data, (int16_t)(val*scale));
}
void wcvesc_push_f32(uint8_t* data, float val, float scale){
    wcvesc_push_i32(data, (int32_t)(val*scale));
}

//------------------------------------------------------------------------------------

uint16_t wcvesc_pop_u16(const uint8_t* data){
    uint16_t out = ((uint16_t)data[0] << 8) | 
                   ((uint16_t)data[1]);
    return out;
}
uint32_t wcvesc_pop_u32(const uint8_t* data){
    uint32_t out = ((uint32_t)data[0] << 24) | 
                   ((uint32_t)data[1] << 16) |
                   ((uint32_t)data[2] << 8) |
                   ((uint32_t)data[3]);
    return out;
}
int16_t wcvesc_pop_i16(const uint8_t* data){
    uint16_t out = wcvesc_pop_u16(data);
    if (out & 0x8000) return -((int16_t)(~out+1));
    return out;
}
int32_t wcvesc_pop_i32(const uint8_t* data){
    uint32_t out = wcvesc_pop_u32(data);
    if (out & 0x80000000) return -((int32_t)(~out+1));
    return out;
}
float wcvesc_pop_f16(const uint8_t* data, float scale){
    return ((float)wcvesc_pop_i16(data)) / scale;
}
float wcvesc_pop_f32(const uint8_t* data, float scale){
    return ((float)wcvesc_pop_i32(data)) / scale;
}

//------------------------------------------------------------------------------------

canid_t wcvesc_encode_id(wcvesc_packet_t packet_id, uint8_t unit_id){
    canid_t out = unit_id;
    out |= ((uint32_t)packet_id) << 0x8;
    out |= CAN_EFF_FLAG;
    return out;
}
void wcvesc_decode_id(canid_t id, wcvesc_packet_t* out_packet_id, uint8_t* out_unit_id){
    *out_unit_id = id & 255;
    *out_packet_id = (wcvesc_packet_t)((id >> 0x8) & 0xff);
}

//------------------------------------------------------------------------------------

void wcvesc_encode_conf_store_current_limits_in(struct can_frame* frame, uint8_t unit_id, float min_curr, float max_curr){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_CONF_STORE_CURRENT_LIMITS_IN, unit_id);
    wcvesc_push_f32(frame->data, min_curr, 1000.0f);
    wcvesc_push_f32(frame->data+4, max_curr, 1000.0f);
    frame->len = 8;
}
void wcvesc_encode_conf_store_current_limits(struct can_frame* frame, uint8_t unit_id, float min_curr, float max_curr){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_CONF_STORE_CURRENT_LIMITS, unit_id);
    wcvesc_push_f32(frame->data, min_curr, 1000.0f);
    wcvesc_push_f32(frame->data+4, max_curr, 1000.0f);
    frame->len = 8;
}
void wcvesc_encode_conf_current_limits_in(struct can_frame* frame, uint8_t unit_id, float min_curr, float max_curr){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_CONF_CURRENT_LIMITS_IN, unit_id);
    wcvesc_push_i32(frame->data, min_curr*1000);
    wcvesc_push_i32(frame->data+4, max_curr*1000);
    frame->len = 4;
}
void wcvesc_encode_conf_current_limits(struct can_frame* frame, uint8_t unit_id, float min_curr, float max_curr){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_CONF_CURRENT_LIMITS, unit_id);
    wcvesc_push_f32(frame->data, min_curr, 1000.0f);
    wcvesc_push_f32(frame->data+4, max_curr, 1000.0f);
    frame->len = 8;
}
void wcvesc_encode_current_brake_rel(struct can_frame* frame, uint8_t unit_id, float current){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_CURRENT_BRAKE_REL, unit_id);
    wcvesc_push_f32(frame->data, current, 100000.0f);
    frame->len = 4;
}
void wcvesc_encode_current_brake(struct can_frame* frame, uint8_t unit_id, float current){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_CURRENT_BRAKE, unit_id);
    wcvesc_push_f32(frame->data, current, 1000.0f);
    frame->len = 4;
}
void wcvesc_encode_current_rel(struct can_frame* frame, uint8_t unit_id, float current){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_CURRENT_REL, unit_id);
    wcvesc_push_f32(frame->data, current, 100000.0f);
    frame->len = 4;
}
void wcvesc_encode_current(struct can_frame* frame, uint8_t unit_id, float current){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_CURRENT, unit_id);
    wcvesc_push_f32(frame->data, current, 1000.0f);
    frame->len = 4;
}
void wcvesc_encode_erpm(struct can_frame* frame, uint8_t unit_id, float erpm){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_RPM, unit_id);
    wcvesc_push_i32(frame->data, erpm);
    frame->len = 4;
}
void wcvesc_encode_duty(struct can_frame* frame, uint8_t unit_id, float duty){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_DUTY, unit_id);
    wcvesc_push_f32(frame->data, duty, 100000.0f);
    frame->len = 4;
}
void wcvesc_encode_pos(struct can_frame* frame, uint8_t unit_id, float pos){
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = wcvesc_encode_id(VESC_SET_POS, unit_id);
    wcvesc_push_i32(frame->data, pos);
    frame->len = 4;
}

//------------------------------------------------------------------------------------

wcvesc_status1_t wcvesc_decode_status1(struct can_frame frame){
    wcvesc_status1_t out;
    out.erpm = wcvesc_pop_f32(frame.data, 1.0f);
    out.current = wcvesc_pop_f16(frame.data+4, 10.0f);
    out.duty_cycle = wcvesc_pop_f16(frame.data+6, 1000.0f);
    return out;
}
wcvesc_status2_t wcvesc_decode_status2(struct can_frame frame){
    wcvesc_status2_t out;
    out.amp_hours = wcvesc_pop_f32(frame.data, 10000.0f);
    out.amp_hours_charged = wcvesc_pop_f32(frame.data+4, 10000.0f);
    return out;
}
wcvesc_status3_t wcvesc_decode_status3(struct can_frame frame){
    wcvesc_status3_t out;
    out.watt_hours = wcvesc_pop_f32(frame.data, 10000.0f);
    out.watt_hours_charged = wcvesc_pop_f32(frame.data+4, 10000.0f);
    return out;
}
wcvesc_status4_t wcvesc_decode_status4(struct can_frame frame){
    wcvesc_status4_t out;
    out.temp_fet = wcvesc_pop_f16(frame.data, 10.0f);
    out.temp_motor = wcvesc_pop_f16(frame.data+2, 10.0f);
    out.current_in = wcvesc_pop_f16(frame.data+4, 10.f);
    out.pid_pos = wcvesc_pop_f16(frame.data+6, 50.0f);
    return out;
}
wcvesc_status5_t wcvesc_decode_status5(struct can_frame frame){
    wcvesc_status5_t out;
    out.tachometer = wcvesc_pop_f32(frame.data, 6.0f);
    out.volts_in = wcvesc_pop_f16(frame.data, 10.0f);
    return out;
}
wcvesc_status6_t wcvesc_decode_status6(struct can_frame frame){
    wcvesc_status6_t out;
    out.adc1 = wcvesc_pop_f16(frame.data, 1000.0f);
    out.adc2 = wcvesc_pop_f16(frame.data, 1000.0f);
    out.adc3 = wcvesc_pop_f16(frame.data, 1000.0f);
    out.ppm = wcvesc_pop_f16(frame.data, 1000.0f);
    return out;
}

//-------------------------------------------------------------------------------------
