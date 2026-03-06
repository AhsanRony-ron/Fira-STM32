/**
 * @file communication.h
 * @brief Kommunikasi STM32 ↔ Jetson via USB CDC
 * @detail Protocol layer untuk command/telemetry terstruktur
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stddef.h>

/* ===== Protocol Configuration ===== */
#define COMM_BUFFER_SIZE        256
#define COMM_TX_INTERVAL_MS     50   /* Send telemetry every 50ms */
#define COMM_TIMEOUT_MS         5000 /* Timeout jika tidak ada command */

/* ===== Message Types ===== */
typedef enum {
    MSG_TYPE_COMMAND   = 0x01,  /* Jetson → STM32 command */
    MSG_TYPE_TELEMETRY = 0x02,  /* STM32 → Jetson telemetry */
    MSG_TYPE_ACK       = 0x03,  /* Acknowledgment */
    MSG_TYPE_ERROR     = 0x04   /* Error message */
} msg_type_t;

/* ===== Command Types ===== */
typedef enum {
    CMD_SET_MOTOR_SPEED = 0x10,    /* Set motor speed */
    CMD_SET_SERVO_ANGLE = 0x11,    /* Set servo angle */
    CMD_SET_MODE        = 0x12,    /* Set control mode */
    CMD_EMERGENCY_STOP  = 0x13,    /* Emergency stop */
    CMD_GET_STATUS      = 0x14,    /* Request status */
    CMD_RESET           = 0x15     /* Reset all systems */
} cmd_type_t;

/* ===== Command Structure ===== */
typedef struct {
    uint8_t type;           /* cmd_type_t */
    int16_t param1;         /* Parameter 1 (speed, angle, mode) */
    int16_t param2;         /* Parameter 2 (optional) */
    uint16_t timestamp;     /* Timestamp from Jetson */
} command_t;

/* ===== Telemetry Structure ===== */
typedef struct {
    uint16_t timestamp_ms;      /* STM32 timestamp */
    
    /* Motor status */
    int16_t motor_speed;        /* Current motor speed -100 to +100 */
    
    /* Servo status */
    int16_t servo_angle;        /* Current servo angle -90 to +90 */
    
    /* IMU data */
    int16_t accel_x, accel_y, accel_z;    /* Acceleration (mg) */
    int16_t gyro_x, gyro_y, gyro_z;       /* Angular velocity (dps) */
    int16_t temp_raw;                     /* Temperature raw */
    
    /* Sensor data */
    uint16_t distance_cm;       /* Ultrasonic distance */
    
    /* Status */
    uint8_t collision_detected; /* Collision flag */
    uint8_t control_mode;       /* Current control mode */
    uint8_t system_status;      /* System health status */
} telemetry_t;

/* ===== Message Frame ===== */
typedef struct {
    uint8_t start_byte;     /* 0xFF */
    uint8_t msg_type;       /* msg_type_t */
    uint8_t payload_len;    /* Payload length */
    uint8_t *payload;       /* Variable length payload */
    uint8_t checksum;       /* CRC8 or simple sum */
    uint8_t end_byte;       /* 0xFE */
} msg_frame_t;

/* ===== API ===== */

/**
 * @brief Initialize communication system
 * @return 0 on success, -1 on error
 */
int comm_init(void);

/**
 * @brief Main communication update (call periodically ~20Hz)
 * Check USB input, send telemetry
 */
void comm_update(void);

/**
 * @brief Send telemetry to Jetson
 * @param telem: telemetry data to send
 */
void comm_send_telemetry(const telemetry_t *telem);

/**
 * @brief Send error message to Jetson
 * @param error_code: error code
 * @param message: error description
 */
void comm_send_error(uint8_t error_code, const char *message);

/**
 * @brief Send acknowledgment
 * @param cmd: command that was acknowledged
 */
void comm_send_ack(cmd_type_t cmd);

/**
 * @brief Get latest command from USB input
 * @param cmd: pointer to store command
 * @return 1 if new command available, 0 otherwise
 */
int comm_get_command(command_t *cmd);

/**
 * @brief Check if communication timeout occurred
 * @return 1 if timeout, 0 otherwise
 */
uint8_t comm_is_timeout(void);

/**
 * @brief Get statistics
 * @return number of commands processed
 */
uint32_t comm_get_cmd_count(void);

#endif /* COMMUNICATION_H */
