#include "wc/esc/odrive.h"
#include "wc/io/can.h"

#include <math.h>
#include <stdint.h>

//------------------------------------------------------------------------------------

void wcodrive_push_u8(uint8_t* data, uint16_t uval){
    data[0] = uval;
}
void wcodrive_push_u16(uint8_t* data, uint16_t uval){
    data[0] = uval >> 8;
    data[1] = uval;
}
void wcodrive_push_u32(uint8_t* data, uint32_t uval){
    data[0] = uval >> 24;
    data[1] = uval >> 16;
    data[2] = uval >> 8;
    data[3] = uval;
}
void wcodrive_push_u48(uint8_t* data, uint64_t uval){
    data[0] = uval >> 32;
    data[1] = uval >> 24;
    data[2] = uval >> 16;
    data[3] = uval >> 8;
    data[4] = uval;
}
void wcodrive_push_i16(uint8_t* data, int16_t val){
    uint16_t uval = val >= 0 ? val : (~((uint16_t)(-val)) + 1);
    wcodrive_push_u16(data, uval);
}
// Black magic found here: https://github.com/vedderb/bldc/blob/master/lispBM/c_libs/examples/config/conf/buffer.c#L96
void wcodrive_push_f32(uint8_t* data, float val){
    if (fabsf(val) < 1.5e-38) val = 0.0f;

    int exp = 0;
    float sig = frexpf(val, &exp);
    float sig_abs = fabsf(sig);
    uint32_t sig_i = 0;

    if (sig_abs >= 0.5f){
        sig_i = (uint32_t)((sig_abs - 0.5f)*2.0f*8388608.0f);
        exp += 126;
    }

    uint32_t res = ((exp & 0xff) << 23) | (sig_i & 0x7fffff);
    if (sig < 0.0f) res |= (uint32_t)1 << 31;
    wcodrive_push_u32(data, res);
}

//------------------------------------------------------------------------------------

uint8_t wcodrive_pop_u8(const uint8_t* data){
    uint8_t out = data[0];
    return out;
}
uint16_t wcodrive_pop_u16(const uint8_t* data){
    uint16_t out = ((uint16_t)data[0] << 8) | 
                   ((uint16_t)data[1]);
    return out;
}
uint32_t wcodrive_pop_u32(const uint8_t* data){
    uint32_t out = ((uint32_t)data[0] << 24) | 
                   ((uint32_t)data[1] << 16) |
                   ((uint32_t)data[2] << 8) |
                   ((uint32_t)data[3]);
    return out;
}
uint64_t wcodrive_pop_u48(const uint8_t* data){
    uint64_t out = ((uint64_t)data[0] << 32) |
                   ((uint64_t)data[1] << 24) | 
                   ((uint64_t)data[2] << 16) |
                   ((uint64_t)data[3] << 8) |
                   ((uint64_t)data[4]);
    return out;
}
int16_t wcodrive_pop_i16(const uint8_t* data){
    uint16_t out = wcodrive_pop_u16(data);
    if (out & 0x8000) return -((int16_t)(~out + 1));
    return out;
}
// Black magic found here: https://github.com/vedderb/bldc/blob/master/lispBM/c_libs/examples/config/conf/buffer.c#L161
float wcodrive_pop_f32(const uint8_t* data){
    uint32_t res = wcodrive_pop_u32(data);

    int exp = (res >> 23) & 0xff;
    uint32_t sig_i = res & 0x7fffff;
    bool neg = res & ((uint32_t)1 << 31);

    float sig = 0.0f;
    if (exp != 0.0f || sig_i != 0){
        sig = (float)sig_i / (8388608.0f*2.0f) + 0.5f;
        exp -= 126;
    }

    if (neg) sig = -neg;
    return ldexpf(sig, exp);
}

//------------------------------------------------------------------------------------

canid_t wcodrive_encode_id(wcodrive_packet_id_t packet_id, uint8_t node_id){
    canid_t out = packet_id;
    out |= ((canid_t)node_id) << 5;
    return out;
}
void wcodrive_decode_id(canid_t id, wcodrive_packet_id_t* out_packet_id, uint8_t* out_node_id){
    *out_packet_id = id & 0x1f;
    *out_node_id = (uint8_t)((id >> 5) & 0x3f);
}

bool wcodrive_has_packet_id(canid_t id, wcodrive_packet_id_t packet_id){
    return (id & 0x1f) == packet_id;
}
bool wcodrive_has_node_id(canid_t id, uint8_t node_id){
    return (uint8_t)((id >> 5) & 0x3f) == node_id;
}

//------------------------------------------------------------------------------------

void wcodrive_encode_set_traj_accel_limits(wccan_frame_t* frame, uint8_t node_id, float traj_accel_limit, float traj_decel_limit){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_TRAJ_ACCEL_LIMITS, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), traj_accel_limit);
    wcodrive_push_f32((void*)wccan_frame_data(frame) + 4, traj_decel_limit);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_set_input_pos(wccan_frame_t* frame, uint8_t node_id, float input_pos, int16_t vel_ff, int16_t torque_ff){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_INPUT_POS, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), input_pos);
    wcodrive_push_i16((void*)wccan_frame_data(frame) + 4, vel_ff);
    wcodrive_push_i16((void*)wccan_frame_data(frame) + 6, torque_ff);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_set_controller_mode(wccan_frame_t* frame, uint8_t node_id, uint32_t control_mode, uint32_t input_mode){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_CONTROLLER_MODE, node_id));
    wcodrive_push_u32((void*)wccan_frame_data(frame), control_mode);
    wcodrive_push_u32((void*)wccan_frame_data(frame) + 4, input_mode);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_rxsdo(wccan_frame_t* frame, uint8_t node_id, uint8_t opcode, uint16_t endpoint_id, uint32_t value){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_RXSDO, node_id));
    wcodrive_push_u8((void*)wccan_frame_data(frame), opcode);
    wcodrive_push_u16((void*)wccan_frame_data(frame) + 1, endpoint_id);
    wcodrive_push_u32((void*)wccan_frame_data(frame) + 3, endpoint_id);
    wccan_frame_set_len(frame, 7);
}
void wcodrive_encode_set_vel_gains(wccan_frame_t* frame, uint8_t node_id, float vel_gain, float vel_integrator_gain){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_VEL_GAINS, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), vel_gain);
    wcodrive_push_f32((void*)wccan_frame_data(frame) + 4, vel_integrator_gain);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_get_torques(wccan_frame_t* frame, uint8_t node_id, float torque_target, float torque_estimate){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_TORQUES, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), torque_target);
    wcodrive_push_f32((void*)wccan_frame_data(frame) + 4, torque_estimate);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_address(wccan_frame_t* frame, uint8_t node_id, uint64_t serial_number, uint8_t connection_id){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_ADDRESS, node_id));
    wcodrive_push_u8((void*)wccan_frame_data(frame), node_id);
    wcodrive_push_u48((void*)wccan_frame_data(frame) + 1, serial_number);
    wcodrive_push_u8((void*)wccan_frame_data(frame) + 7, connection_id);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_set_input_vel(wccan_frame_t* frame, uint8_t node_id, float input_vel, float input_torque_ff){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_INPUT_VEL, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), input_vel);
    wcodrive_push_f32((void*)wccan_frame_data(frame) + 4, input_torque_ff);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_set_limits(wccan_frame_t* frame, uint8_t node_id, float velocity_limit, float current_limit){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_LIMITS, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), velocity_limit);
    wcodrive_push_f32((void*)wccan_frame_data(frame) + 4, current_limit);
    wccan_frame_set_len(frame, 8);
}
void wcodrive_encode_set_axis_state(wccan_frame_t* frame, uint8_t node_id, uint32_t axis_requested_state){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_AXIS_STATE, node_id));
    wcodrive_push_u32((void*)wccan_frame_data(frame), axis_requested_state);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_set_traj_vel_limit(wccan_frame_t* frame, uint8_t node_id, float traj_vel_limit){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_TRAJ_VEL_LIMIT, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), traj_vel_limit);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_set_absolute_position(wccan_frame_t* frame, uint8_t node_id, float position){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_ABSOLUTE_POSITION, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), position);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_set_traj_inertia(wccan_frame_t* frame, uint8_t node_id, float traj_inertia){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_TRAJ_INERTIA, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), traj_inertia);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_set_input_torque(wccan_frame_t* frame, uint8_t node_id, float input_toque){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_INPUT_TORQUE, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), input_toque);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_clear_errors(wccan_frame_t* frame, uint8_t node_id, uint8_t identify){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_CLEAR_ERRORS, node_id));
    wcodrive_push_u8((void*)wccan_frame_data(frame), identify);
    wccan_frame_set_len(frame, 1);
}
void wcodrive_encode_set_pos_gain(wccan_frame_t* frame, uint8_t node_id, float pos_gain){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_SET_POS_GAIN, node_id));
    wcodrive_push_f32((void*)wccan_frame_data(frame), pos_gain);
    wccan_frame_set_len(frame, 4);
}
void wcodrive_encode_get_bus_voltage_current(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_BUS_VOLTAGE_CURRENT, node_id));
}
void wcodrive_encode_get_encoder_estimates(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_ENCODER_ESTIMATES, node_id));
}
void wcodrive_encode_get_temperature(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 1);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_TEMPERATURE, node_id));
}
void wcodrive_encode_enter_dfu_mode(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_ENTER_DFU_MODE, node_id));
}
void wcodrive_encode_get_powers(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_POWERS, node_id));
}
void wcodrive_encode_get_error(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_ERROR, node_id));
}
void wcodrive_encode_get_iq(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_GET_IQ, node_id));
}
void wcodrive_encode_reboot(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_REBOOT, node_id));
}
void wcodrive_encode_estop(wccan_frame_t* frame, uint8_t node_id){
    wccan_frame_init(frame, 0, 0, 0);
    wccan_frame_set_id(frame, wcodrive_encode_id(WCODRIVE_ESTOP, node_id));
}

//------------------------------------------------------------------------------------

wcodrive_bus_voltage_current_t wcodrive_decode_bus_voltage_current(const wccan_frame_t* frame){
    wcodrive_bus_voltage_current_t out;
    out.bus_voltage = wcodrive_pop_f32(wccan_frame_data(frame));
    out.bus_current = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_encoder_estimates_t wcodrive_decode_encoder_estimates(const wccan_frame_t* frame){
    wcodrive_encoder_estimates_t out;
    out.pos_estimate = wcodrive_pop_f32(wccan_frame_data(frame));
    out.vel_estimate = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_temperature_t wcodrive_decode_temperature(const wccan_frame_t* frame){
    wcodrive_temperature_t out;
    out.fet_temperature = wcodrive_pop_f32(wccan_frame_data(frame));
    out.motor_temperature = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_heartbeat_t wcodrive_decode_heartbeat(const wccan_frame_t* frame){
    wcodrive_heartbeat_t out;
    out.axis_error = wcodrive_pop_u32(wccan_frame_data(frame));
    out.axis_state = wcodrive_pop_u8(wccan_frame_data(frame) + 4);
    out.procedure_result = wcodrive_pop_u8(wccan_frame_data(frame) + 5);
    out.trajectory_done_flag = wcodrive_pop_u8(wccan_frame_data(frame) + 6);
    return out;
}
wcodrive_version_t wcodrive_decode_version(const wccan_frame_t* frame){
    wcodrive_version_t out;
    out.protocol_version = wcodrive_pop_u8(wccan_frame_data(frame));
    out.hw_version_major = wcodrive_pop_u8(wccan_frame_data(frame) + 1);
    out.hw_version_minor = wcodrive_pop_u8(wccan_frame_data(frame) + 2);
    out.hw_version_variant = wcodrive_pop_u8(wccan_frame_data(frame) + 3);
    out.fw_version_major = wcodrive_pop_u8(wccan_frame_data(frame) + 4);
    out.fw_version_minor = wcodrive_pop_u8(wccan_frame_data(frame) + 5);
    out.fw_version_revision = wcodrive_pop_u8(wccan_frame_data(frame) + 6);
    out.fw_version_unreleased = wcodrive_pop_u8(wccan_frame_data(frame) + 7);
    return out;
}
wcodrive_address_t wcodrive_decode_address(const wccan_frame_t* frame){
    wcodrive_address_t out;
    out.node_id = wcodrive_pop_u8(wccan_frame_data(frame));
    out.serial_number = wcodrive_pop_u48(wccan_frame_data(frame) + 1);
    out.connection_id = wcodrive_pop_u8(wccan_frame_data(frame) + 7);
    return out;
}
wcodrive_torques_t wcodrive_decode_torques(const wccan_frame_t* frame){
    wcodrive_torques_t out;
    out.torque_target = wcodrive_pop_f32(wccan_frame_data(frame));
    out.torque_estimate = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_powers_t wcodrive_decode_powers(const wccan_frame_t* frame){
    wcodrive_powers_t out;
    out.electrical_power = wcodrive_pop_f32(wccan_frame_data(frame));
    out.mechanical_power = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_error_t wcodrive_decode_error(const wccan_frame_t* frame){
    wcodrive_error_t out;
    out.active_errors = wcodrive_pop_u32(wccan_frame_data(frame));
    out.disarm_reason = wcodrive_pop_u32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_txsdo_t wcodrive_decode_txsdo(const wccan_frame_t* frame){
    wcodrive_txsdo_t out;
    out.endpoint_id = wcodrive_pop_u16(wccan_frame_data(frame) + 1);
    out.value = wcodrive_pop_u32(wccan_frame_data(frame) + 4);
    return out;
}
wcodrive_iq_t wcodrive_decode_iq(const wccan_frame_t* frame){
    wcodrive_iq_t out;
    out.iq_setpoint = wcodrive_pop_f32(wccan_frame_data(frame));
    out.iq_measured = wcodrive_pop_f32(wccan_frame_data(frame) + 4);
    return out;
}

//------------------------------------------------------------------------------------
