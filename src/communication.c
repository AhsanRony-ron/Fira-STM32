/**
 * @file communication.c
 * @brief Komunikasi STM32 ↔ Jetson Implementation
 */

#include "communication.h"
#include "usbd_cdc_if.h"
#include "control/control_loop.h"
#include <string.h>
#include <stdio.h>

/* ===== Local State ===== */
// static uint8_t rx_buffer[COMM_BUFFER_SIZE];  // TODO: implement RX buffer handling
static uint8_t tx_buffer[COMM_BUFFER_SIZE];
static command_t pending_command = {0};
static uint8_t command_available = 0;
static uint32_t last_command_tick = 0;
static uint32_t command_count = 0;
static uint32_t last_telemetry_tick = 0;

/* ===== Internal Helpers ===== */

/**
 * @brief Calculate simple CRC8 checksum
 */
// static uint8_t calc_crc8(uint8_t *data, uint16_t len)
// {
//     uint8_t crc = 0;
//     for (uint16_t i = 0; i < len; i++) {
//         crc += data[i];
//     }
//     return crc;
// }

/**
 * @brief Parse command from USB input
 * Format: $CMD,<type>,<param1>,<param2>,<timestamp>\n
 * Example: $CMD,16,75,0,1234\n (set motor speed to 75%)
 */
static int parse_usb_command(uint8_t *data, uint16_t len, command_t *cmd)
{
    char buffer[COMM_BUFFER_SIZE];
    if (len >= COMM_BUFFER_SIZE) return -1;
    
    strncpy(buffer, (char*)data, len);
    buffer[len] = '\0';
    
    /* Check start marker */
    if (buffer[0] != '$') return -1;
    
    int type, param1, param2, ts;
    if (sscanf(buffer, "$CMD,%d,%d,%d,%d", &type, &param1, &param2, &ts) != 4) {
        return -1;
    }
    
    cmd->type = type;
    cmd->param1 = param1;
    cmd->param2 = param2;
    cmd->timestamp = ts;
    
    return 0;
}

/**
 * @brief Format and send telemetry frame
 * Format: @TELEM,<motor>,<servo>,<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<dist>,<collision>,<mode>\n
 */
static void send_telemetry_frame(const telemetry_t *telem)
{
    int len = snprintf((char*)tx_buffer, sizeof(tx_buffer),
        "@TELEM,%d,%d,%d,%d,%d,%d,%d,%d,%u,%d,%d,%u\r\n",
        telem->motor_speed,
        telem->servo_angle,
        telem->accel_x, telem->accel_y, telem->accel_z,
        telem->gyro_x, telem->gyro_y, telem->gyro_z,
        telem->distance_cm,
        telem->collision_detected,
        telem->control_mode,
        telem->timestamp_ms
    );
    
    if (len > 0) {
        CDC_Transmit_FS(tx_buffer, len);
    }
}

/**
 * @brief Send error frame
 * Format: @ERROR,<code>,<message>\n
 */
static void send_error_frame(uint8_t error_code, const char *message)
{
    int len = snprintf((char*)tx_buffer, sizeof(tx_buffer),
        "@ERROR,%d,%s\r\n", error_code, message
    );
    
    if (len > 0) {
        CDC_Transmit_FS(tx_buffer, len);
    }
}

/**
 * @brief Send ACK frame
 * Format: @ACK,<command_type>\n
 */
static void send_ack_frame(cmd_type_t cmd)
{
    int len = snprintf((char*)tx_buffer, sizeof(tx_buffer),
        "@ACK,%d\r\n", cmd
    );
    
    if (len > 0) {
        CDC_Transmit_FS(tx_buffer, len);
    }
}

/* ===== API Implementation ===== */

int comm_init(void)
{
    last_command_tick = HAL_GetTick();
    last_telemetry_tick = HAL_GetTick();
    command_available = 0;
    command_count = 0;
    
    return 0;
}

void comm_update(void)
{
    uint32_t now = HAL_GetTick();
    
    /* Handle received data from Jetson */
    /* Note: USB interrupt will fill rx_buffer
     * This is simplified - in real implementation, use circular buffer */
    
    /* Send telemetry periodically */
    if (now - last_telemetry_tick >= COMM_TX_INTERVAL_MS) {
        last_telemetry_tick = now;
        
        /* Collect telemetry data from control system */
        control_state_t state;
        control_get_state(&state);
        
        telemetry_t telem = {
            .timestamp_ms = now,
            .motor_speed = state.motor_speed,
            .servo_angle = state.servo_angle,
            .accel_x = state.accel[0],
            .accel_y = state.accel[1],
            .accel_z = state.accel[2],
            .gyro_x = state.gyro[0],
            .gyro_y = state.gyro[1],
            .gyro_z = state.gyro[2],
            .distance_cm = state.distance_front_cm,
            .collision_detected = state.collision_detected,
            .control_mode = (uint8_t)control_get_mode(),
            .system_status = 0   /* TODO: calculate health */
        };
        
        comm_send_telemetry(&telem);
    }
}

void comm_send_telemetry(const telemetry_t *telem)
{
    if (telem == NULL) return;
    send_telemetry_frame(telem);
}

void comm_send_error(uint8_t error_code, const char *message)
{
    if (message == NULL) message = "Unknown error";
    send_error_frame(error_code, message);
}

void comm_send_ack(cmd_type_t cmd)
{
    send_ack_frame(cmd);
}

int comm_get_command(command_t *cmd)
{
    if (!command_available) return 0;
    
    if (cmd != NULL) {
        *cmd = pending_command;
    }
    
    command_available = 0;
    return 1;
}

uint8_t comm_is_timeout(void)
{
    uint32_t now = HAL_GetTick();
    if (now - last_command_tick > COMM_TIMEOUT_MS) {
        return 1;
    }
    return 0;
}

uint32_t comm_get_cmd_count(void)
{
    return command_count;
}

/* ===== USB Receive Callback ===== */
/* Call this from usbd_cdc_if.c when data received */

void comm_handle_usb_rx(uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) return;
    
    /* Try to parse as command */
    command_t cmd;
    if (parse_usb_command(data, len, &cmd) == 0) {
        pending_command = cmd;
        command_available = 1;
        last_command_tick = HAL_GetTick();
        command_count++;
        
        /* Send acknowledgment */
        comm_send_ack(cmd.type);
        
        /* Process command based on type */
        switch (cmd.type) {
            case CMD_SET_MOTOR_SPEED:
                control_set_mode(CONTROL_MODE_MANUAL);  /* Switch to manual mode */
                control_manual_command(cmd.param1, -1);  /* Keep servo unchanged */
                break;
                
            case CMD_SET_SERVO_ANGLE:
                control_set_mode(CONTROL_MODE_MANUAL);  /* Switch to manual mode */
                control_manual_command(-1, cmd.param1);  /* Keep motor unchanged */
                break;
                
            case CMD_SET_MODE:
                control_set_mode((control_mode_t)cmd.param1);
                break;
                
            case CMD_EMERGENCY_STOP:
                control_failsafe();
                break;
                
            case CMD_GET_STATUS:
                /* Status will be sent in next telemetry packet */
                break;
                
            case CMD_RESET:
                /* TODO: Implement reset logic */
                break;
                
            default:
                comm_send_error(1, "Unknown command");
                break;
        }
    } else {
        /* Invalid command format */
        comm_send_error(2, "Invalid format");
    }
}
