/**
 * @file servo.h
 * @brief Servo Motor Control - Header
 * @detail Standard RC servo untuk steering autonomous car
 *         Menggunakan TIM9 untuk PWM (1-2 ms pulse width)
 */

#ifndef SERVO_H
#define SERVO_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ===== Configuration ===== */
#define SERVO_PWM_TIMER      &htim9
#define SERVO_PWM_CHANNEL    TIM_CHANNEL_1
#define SERVO_PWM_FREQ       50       /* 50 Hz standard servo */
#define SERVO_FREQ_DIV       20000    /* Timer frequency (assume 20kHz timer) */

/* Servo pulse width limits (microseconds) */
#define SERVO_PULSE_MIN      1000     /* ~0 degrees (full left) */
#define SERVO_PULSE_CENTER   1500     /* ~90 degrees (center/straight) */
#define SERVO_PULSE_MAX      2000     /* ~180 degrees (full right) */

/* Angle mapping */
#define SERVO_ANGLE_MIN      -90      /* Left */
#define SERVO_ANGLE_CENTER   0        /* Straight */
#define SERVO_ANGLE_MAX      90       /* Right */

/* ===== Types ===== */
typedef struct {
    int16_t angle;              /* Current angle (-90 to +90) */
    uint16_t pulse_width_us;    /* Current pulse width in microseconds */
    uint32_t timestamp_ms;      /* Last update timestamp */
} servo_state_t;

/* ===== API ===== */

/**
 * @brief Initialize servo motor
 * @return 0 on success, -1 on error
 */
int servo_init(void);

/**
 * @brief Set servo angle
 * @param angle: -90 (left) to +90 (right), 0 = center
 * @return 0 on success, -1 on invalid angle
 */
int servo_set_angle(int16_t angle);

/**
 * @brief Set servo by pulse width (advanced)
 * @param pulse_us: pulse width in microseconds (1000-2000)
 * @return 0 on success, -1 on invalid pulse
 */
int servo_set_pulse(uint16_t pulse_us);

/**
 * @brief Get current servo state
 * @param state: pointer to servo_state_t struct
 */
void servo_get_state(servo_state_t *state);

/**
 * @brief Center servo (angle = 0)
 */
void servo_center(void);

#endif /* SERVO_H */
