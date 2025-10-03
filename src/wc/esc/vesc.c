#include "wc/esc/vesc.h"

//------------------------------------------------------------------------------------

// Firmware implementation: https://github.com/vedderb/bldc/blob/master/lispBM/c_libs/examples/config/conf/buffer.c#L24
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
    uint16_t uval = val >= 0 ? val : (~((uint16_t)(-val)) + 1);
    wcvesc_push_u16(data, uval);
}
void wcvesc_push_i32(uint8_t* data, int32_t val){
    uint32_t uval = val >= 0 ? val : (~((uint32_t)(-val)) + 1);
    wcvesc_push_u32(data, uval);
}
void wcvesc_push_f16(uint8_t* data, float val, float scale){
    wcvesc_push_i16(data, (int16_t)(val*scale));
}
void wcvesc_push_f32(uint8_t* data, float val, float scale){
    wcvesc_push_i32(data, (int32_t)(val*scale));
}

//------------------------------------------------------------------------------------

// Firmware implementation: https://github.com/vedderb/bldc/blob/master/lispBM/c_libs/examples/config/conf/buffer.c#L121
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
    if (out & 0x8000) return -((int16_t)(~out + 1));
    return out;
}
int32_t wcvesc_pop_i32(const uint8_t* data){
    uint32_t out = wcvesc_pop_u32(data);
    if (out & 0x80000000) return -((int32_t)(~out + 1));
    return out;
}
float wcvesc_pop_f16(const uint8_t* data, float scale){
    return ((float)wcvesc_pop_i16(data))/scale;
}
float wcvesc_pop_f32(const uint8_t* data, float scale){
    return ((float)wcvesc_pop_i32(data))/scale;
}

//------------------------------------------------------------------------------------

canid_t wcvesc_encode_id(wcvesc_packet_id_t packet_id, uint8_t unit_id){
    canid_t out = unit_id;
    out |= ((canid_t)packet_id) << 8;
    return out;
}
void wcvesc_decode_id(canid_t id, wcvesc_packet_id_t* out_packet_id, uint8_t* out_unit_id){
    *out_unit_id = id & 0xff;
    *out_packet_id = (wcvesc_packet_id_t)((id >> 8) & 0xff);
}

bool wcvesc_has_packet_id(canid_t id, wcvesc_packet_id_t packet_id){
    return (wcvesc_packet_id_t)((id >> 8) & 0xff) == packet_id;
}
bool wcvesc_has_unit_id(canid_t id, uint8_t unit_id){
    return (id & 0xff) == unit_id;
}

//------------------------------------------------------------------------------------

// Firmware implementation: https://github.com/vedderb/bldc/blob/master/comm/comm_can.c#L507
void wcvesc_encode_conf_store_current_limits_in(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_CONF_STORE_CURRENT_LIMITS_IN, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), min_curr, 1000.0f);
    wcvesc_push_f32((void*)wccan_frame_data(frame) + 4, max_curr, 1000.0f);
    wccan_frame_set_len(frame, 8);
}
void wcvesc_encode_conf_store_current_limits(wccan_frame_t* frame, uint8_t unit_id, float min_curr, float max_curr){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_CONF_STORE_CURRENT_LIMITS, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), min_curr, 1000.0f);
    wcvesc_push_f32((void*)wccan_frame_data(frame) + 4, max_curr, 1000.0f);
    wccan_frame_set_len(frame, 8);
}
void wcvesc_encode_set_current_brake_rel(wccan_frame_t* frame, uint8_t unit_id, float current){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_CURRENT_BRAKE_REL, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), current, 100000.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_current_brake(wccan_frame_t* frame, uint8_t unit_id, float current){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_CURRENT_BRAKE, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), current, 1000.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_current_rel(wccan_frame_t* frame, uint8_t unit_id, float current){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_CURRENT_REL, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), current, 100000.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_current(wccan_frame_t* frame, uint8_t unit_id, float current){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_CURRENT, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), current, 1000.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_duty(wccan_frame_t* frame, uint8_t unit_id, float duty){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_DUTY, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), duty, 100000.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_rpm(wccan_frame_t* frame, uint8_t unit_id, float rpm){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_RPM, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), rpm, 1.0f);
    wccan_frame_set_len(frame, 4);
}
void wcvesc_encode_set_pos(wccan_frame_t* frame, uint8_t unit_id, float pos){
    wccan_frame_init(frame, 0, 1, 0);
    wccan_frame_set_id(frame, wcvesc_encode_id(WCVESC_SET_POS, unit_id));
    wcvesc_push_f32((void*)wccan_frame_data(frame), pos, 1000000.0f);
    wccan_frame_set_len(frame, 4);
}

//------------------------------------------------------------------------------------

wcvesc_status1_t wcvesc_decode_status1(const wccan_frame_t* frame){
    wcvesc_status1_t out;
    out.erpm = wcvesc_pop_f32(wccan_frame_data(frame), 1.0f);
    out.current = wcvesc_pop_f16(wccan_frame_data(frame) + 4, 10.0f);
    out.duty_cycle = wcvesc_pop_f16(wccan_frame_data(frame) + 6, 1000.0f);
    return out;
}
wcvesc_status2_t wcvesc_decode_status2(const wccan_frame_t* frame){
    wcvesc_status2_t out;
    out.amp_hours = wcvesc_pop_f32(wccan_frame_data(frame), 10000.0f);
    out.amp_hours_charged = wcvesc_pop_f32(wccan_frame_data(frame) + 4, 10000.0f);
    return out;
}
wcvesc_status3_t wcvesc_decode_status3(const wccan_frame_t* frame){
    wcvesc_status3_t out;
    out.watt_hours = wcvesc_pop_f32(wccan_frame_data(frame), 10000.0f);
    out.watt_hours_charged = wcvesc_pop_f32(wccan_frame_data(frame) + 4, 10000.0f);
    return out;
}
wcvesc_status4_t wcvesc_decode_status4(const wccan_frame_t* frame){
    wcvesc_status4_t out;
    out.temp_fet = wcvesc_pop_f16(wccan_frame_data(frame), 10.0f);
    out.temp_motor = wcvesc_pop_f16(wccan_frame_data(frame) + 2, 10.0f);
    out.current_in = wcvesc_pop_f16(wccan_frame_data(frame) + 4, 10.f);
    out.pid_pos = wcvesc_pop_f16(wccan_frame_data(frame) + 6, 50.0f);
    return out;
}
wcvesc_status5_t wcvesc_decode_status5(const wccan_frame_t* frame){
    wcvesc_status5_t out;
    out.tachometer = wcvesc_pop_f32(wccan_frame_data(frame), 6.0f);
    out.volts_in = wcvesc_pop_f16(wccan_frame_data(frame) + 4, 10.0f);
    return out;
}
wcvesc_status6_t wcvesc_decode_status6(const wccan_frame_t* frame){
    wcvesc_status6_t out;
    out.adc1 = wcvesc_pop_f16(wccan_frame_data(frame), 1000.0f);
    out.adc2 = wcvesc_pop_f16(wccan_frame_data(frame) + 2, 1000.0f);
    out.adc3 = wcvesc_pop_f16(wccan_frame_data(frame) + 4, 1000.0f);
    out.ppm = wcvesc_pop_f16(wccan_frame_data(frame) + 6, 1000.0f);
    return out;
}

//-------------------------------------------------------------------------------------
