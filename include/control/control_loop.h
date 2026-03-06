/**
 * @file control_loop.h
 * @brief Autonomous Car Close-Loop Control System - Header
 * @detail Menyatukan sensor & actuator untuk closed-loop steering & speed control
 */

#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ===== Control Parameters ===== */
#define CONTROL_LOOP_RATE_MS    20     /* 50 Hz control loop */
#define CONTROL_LOOP_TIMEOUT_MS 5000   /* Failsafe timeout */

/* ===== Types ===== */
typedef enum {
    CONTROL_MODE_MANUAL   = 0,  /* Remote control via USB/UART */
    CONTROL_MODE_AUTONOMOUS = 1,/* Autonomous navigation */
    CONTROL_MODE_FAILSAFE = 2   /* Emergency stop mode */
} control_mode_t;

typedef struct {
    /* Sensor inputs */
    int16_t accel[3];           /* IMU acceleration (mg) */
    int16_t gyro[3];            /* IMU gyro (dps) */
    uint32_t distance_front_cm; /* Ultrasonic distance (cm) */
    
    /* Actuator commands */
    int16_t motor_speed;        /* Motor speed -100 to +100 (%) */
    int16_t servo_angle;        /* Servo angle -90 to +90 (degrees) */
    
    /* Status */
    uint32_t timestamp_ms;
    uint8_t collision_detected;
} control_state_t;

/* ===== API ===== */

/**
 * @brief Initialize control system
 * @return 0 on success, -1 on error
 */
int control_init(void);

/**
 * @brief Main control loop - call this periodically (~50Hz)
 */
void control_loop_update(void);

/**
 * @brief Set control mode
 * @param mode: CONTROL_MODE_MANUAL, AUTONOMOUS, or FAILSAFE
 */
void control_set_mode(control_mode_t mode);

/**
 * @brief Get current control state
 * @param state: pointer to control_state_t struct
 */
void control_get_state(control_state_t *state);

/**
 * @brief Get current control mode
 * @return Current control_mode_t
 */
control_mode_t control_get_mode(void);

/**
 * @brief Manual command: set motor speed and servo angle
 * @param motor_speed: -100 to +100 (%), or -1 to keep unchanged
 * @param servo_angle: -90 to +90 (degrees), or -1 to keep unchanged
 */
void control_manual_command(int16_t motor_speed, int16_t servo_angle);

/**
 * @brief Execute failsafe (stop all)
 */
void control_failsafe(void);

#endif /* CONTROL_LOOP_H */
