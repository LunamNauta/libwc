#include "wc/io/can.h"

#include <stdint.h>

#include <linux/can.h>

//------------------------------------------------------------------------------------

typedef enum wcodrive_packet_id{
    WCODRIVE_GET_VERSION             = 0x00,
    WCODRIVE_HEARTBEAT               = 0x01,
    WCODRIVE_ESTOP                   = 0x02,
    WCODRIVE_GET_ERROR               = 0x03,
    WCODRIVE_RXSDO                   = 0x04,
    WCODRIVE_TXSDO                   = 0x05,
    WCODRIVE_ADDRESS                 = 0x06,
    WCODRIVE_SET_AXIS_STATE          = 0x07,
    WCODRIVE_GET_ENCODER_ESTIMATES   = 0x09,
    WCODRIVE_SET_CONTROLLER_MODE     = 0x0b,
    WCODRIVE_SET_INPUT_POS           = 0x0c,
    WCODRIVE_SET_INPUT_VEL           = 0x0d,
    WCODRIVE_SET_INPUT_TORQUE        = 0x0e,
    WCODRIVE_SET_LIMITS              = 0x0f,
    WCODRIVE_SET_TRAJ_VEL_LIMIT      = 0x11,
    WCODRIVE_SET_TRAJ_ACCEL_LIMITS   = 0x12,
    WCODRIVE_SET_TRAJ_INERTIA        = 0x13,
    WCODRIVE_GET_IQ                  = 0x14,
    WCODRIVE_GET_TEMPERATURE         = 0x15,
    WCODRIVE_REBOOT                  = 0x16,
    WCODRIVE_GET_BUS_VOLTAGE_CURRENT = 0x17,
    WCODRIVE_CLEAR_ERRORS            = 0x18,
    WCODRIVE_SET_ABSOLUTE_POSITION   = 0x19,
    WCODRIVE_SET_POS_GAIN            = 0x1a,
    WCODRIVE_SET_VEL_GAINS           = 0x1b,
    WCODRIVE_GET_TORQUES             = 0x1c,
    WCODRIVE_GET_POWERS              = 0x1d,
    WCODRIVE_ENTER_DFU_MODE          = 0x1f
} wcodrive_packet_id_t;

//------------------------------------------------------------------------------------

typedef struct wcodrive_version{
    uint8_t protocol_version;
    uint8_t hw_version_major;
    uint8_t hw_version_minor;
    uint8_t hw_version_variant;
    uint8_t fw_version_major;
    uint8_t fw_version_minor;
    uint8_t fw_version_revision;
    uint8_t fw_version_unreleased;
} wcodrive_version_t;

typedef struct wcodrive_heartbeat{
    uint32_t axis_error;
    uint8_t axis_state;
    uint8_t procedure_result;
    uint8_t trajectory_done_flag;
} wcodrive_heartbeat_t;

typedef struct wcodrive_error{
    uint32_t active_errors;
    uint32_t disarm_reason;
} wcodrive_error_t;

typedef struct wcodrive_txsdo{
    uint16_t endpoint_id;
    uint32_t value;
} wcodrive_txsdo_t;

typedef struct wcodrive_address{
    uint8_t node_id;
    uint64_t serial_number : 48;
    uint8_t connection_id;
} wcodrive_address_t;

typedef struct wcodrive_encoder_estimates{
    float pos_estimate;
    float vel_estimate;
} wcodrive_encoder_estimates_t;

typedef struct wcodrive_iq{
    float iq_setpoint;
    float iq_measured;
} wcodrive_iq_t;

typedef struct wcodrive_temperature{
    float fet_temperature;
    float motor_temperature;
} wcodrive_temperature_t;

typedef struct wcodrive_bus_voltage_current{
    float bus_voltage;
    float bus_current;
} wcodrive_bus_voltage_current_t;

typedef struct wcodrive_torques{
    float torque_target;
    float torque_estimate;
} wcodrive_torques_t;

typedef struct wcodrive_powers{
    float electrical_power;
    float mechanical_power;
} wcodrive_powers_t;

//------------------------------------------------------------------------------------

canid_t wcodrive_encode_id(wcodrive_packet_id_t packet_id, uint8_t node_id);
void wcodrive_decode_id(canid_t id, wcodrive_packet_id_t* out_packet_id, uint8_t* out_node_id);

bool wcodrive_has_packet_id(canid_t id, wcodrive_packet_id_t packet_id);
bool wcodrive_has_node_id(canid_t id, uint8_t node_id);

//------------------------------------------------------------------------------------

void wcodrive_encode_set_traj_accel_limits(wccan_frame_t* frame, uint8_t node_id, float traj_accel_limit, float traj_decel_limit);
void wcodrive_encode_set_input_pos(wccan_frame_t* frame, uint8_t node_id, float input_pos, int16_t vel_ff, int16_t torque_ff);
void wcodrive_encode_set_controller_mode(wccan_frame_t* frame, uint8_t node_id, uint32_t control_mode, uint32_t input_mode);
void wcodrive_encode_rxsdo(wccan_frame_t* frame, uint8_t node_id, uint8_t opcode, uint16_t endpoint_id, uint32_t value);
void wcodrive_encode_set_vel_gains(wccan_frame_t* frame, uint8_t node_id, float vel_gain, float vel_integrator_gain);
void wcodrive_encode_get_torques(wccan_frame_t* frame, uint8_t node_id, float torque_target, float torque_estimate);
void wcodrive_encode_address(wccan_frame_t* frame, uint8_t node_id, uint64_t serial_number, uint8_t connection_id);
void wcodrive_encode_set_input_vel(wccan_frame_t* frame, uint8_t node_id, float input_vel, float input_torque_ff);
void wcodrive_encode_set_limits(wccan_frame_t* frame, uint8_t node_id, float velocity_limit, float current_limit);
void wcodrive_encode_set_axis_state(wccan_frame_t* frame, uint8_t node_id, uint32_t axis_requested_state);
void wcodrive_encode_set_traj_vel_limit(wccan_frame_t* frame, uint8_t node_id, float traj_vel_limit);
void wcodrive_encode_set_absolute_position(wccan_frame_t* frame, uint8_t node_id, float position);
void wcodrive_encode_set_traj_inertia(wccan_frame_t* frame, uint8_t node_id, float traj_inertia);
void wcodrive_encode_set_input_torque(wccan_frame_t* frame, uint8_t node_id, float input_toque);
void wcodrive_encode_clear_errors(wccan_frame_t* frame, uint8_t node_id, uint8_t identify);
void wcodrive_encode_set_pos_gain(wccan_frame_t* frame, uint8_t node_id, float pos_gain);
void wcodrive_encode_get_bus_voltage_current(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_get_encoder_estimates(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_get_temperature(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_enter_dfu_mode(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_get_powers(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_get_error(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_get_iq(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_reboot(wccan_frame_t* frame, uint8_t node_id);
void wcodrive_encode_estop(wccan_frame_t* frame, uint8_t node_id);

//------------------------------------------------------------------------------------

wcodrive_bus_voltage_current_t wcodrive_decode_bus_voltage_current(const wccan_frame_t* frame);
wcodrive_encoder_estimates_t wcodrive_decode_encoder_estimates(const wccan_frame_t* frame);
wcodrive_temperature_t wcodrive_decode_temperature(const wccan_frame_t* frame);
wcodrive_heartbeat_t wcodrive_decode_heartbeat(const wccan_frame_t* frame);
wcodrive_version_t wcodrive_decode_version(const wccan_frame_t* frame);
wcodrive_address_t wcodrive_decode_address(const wccan_frame_t* frame);
wcodrive_torques_t wcodrive_decode_torques(const wccan_frame_t* frame);
wcodrive_powers_t wcodrive_decode_powers(const wccan_frame_t* frame);
wcodrive_error_t wcodrive_decode_error(const wccan_frame_t* frame);
wcodrive_txsdo_t wcodrive_decode_txsdo(const wccan_frame_t* frame);
wcodrive_iq_t wcodrive_decode_iq(const wccan_frame_t* frame);

//------------------------------------------------------------------------------------
