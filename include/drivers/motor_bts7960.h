/**
 * @file motor_bts7960.h
 * @brief BTS7960 Motor Driver (Dual H-Bridge) - Header
 * @detail Untuk autonomous car: mengontrol 2 motor DC (kiri & kanan)
 *         Menggunakan PWM via TIM2 (motor kanan) dan TIM3 (motor kiri)
 */

#ifndef MOTOR_BTS7960_H
#define MOTOR_BTS7960_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ===== Configuration ===== */
/* Motor Kanan (Right Motor) - TIM2 */
#define MOTOR_RIGHT_PWM_TIMER    &htim2
#define MOTOR_RIGHT_PWM_CHANNEL  TIM_CHANNEL_1
#define MOTOR_RIGHT_EN_PORT      GPIOA
#define MOTOR_RIGHT_EN_PIN       GPIO_PIN_0

/* Motor Kiri (Left Motor) - TIM3 */
#define MOTOR_LEFT_PWM_TIMER     &htim3
#define MOTOR_LEFT_PWM_CHANNEL   TIM_CHANNEL_1
#define MOTOR_LEFT_EN_PORT       GPIOA
#define MOTOR_LEFT_EN_PIN        GPIO_PIN_1

/* PWM Parameters */
#define MOTOR_PWM_FREQ           20000  /* 20 kHz */
#define MOTOR_PWM_MAX            100    /* 100% */
#define MOTOR_PWM_MIN            0      /* 0% */

/* ===== Types ===== */
typedef enum {
    MOTOR_RIGHT = 0,
    MOTOR_LEFT  = 1
} motor_id_t;

typedef enum {
    MOTOR_FORWARD  = 1,
    MOTOR_BACKWARD = -1,
    MOTOR_STOP     = 0
} motor_direction_t;

typedef struct {
    motor_id_t id;
    int16_t speed;        /* -100 to +100 (%) */
    motor_direction_t dir;
} motor_state_t;

/* ===== API ===== */

/**
 * @brief Initialize motor drivers
 * @return 0 on success, -1 on error
 */
int motor_init(void);

/**
 * @brief Set motor speed and direction
 * @param motor_id: MOTOR_RIGHT or MOTOR_LEFT
 * @param speed: -100 to +100 (negative=backward, positive=forward, 0=stop)
 * @return 0 on success
 */
int motor_set_speed(motor_id_t motor_id, int16_t speed);

/**
 * @brief Get current motor state
 * @param motor_id: MOTOR_RIGHT or MOTOR_LEFT
 * @param state: pointer to motor_state_t struct
 */
void motor_get_state(motor_id_t motor_id, motor_state_t *state);

/**
 * @brief Stop all motors
 */
void motor_stop_all(void);

/**
 * @brief Emergency stop (immediate halt)
 */
void motor_emergency_stop(void);

#endif /* MOTOR_BTS7960_H */
