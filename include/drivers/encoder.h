/**
 * @file encoder.h
 * @brief Motor Encoder Driver - Header
 * @detail Quadrature encoder untuk motor feedback pada autonomous car
 *         Menggunakan TIM2 dan TIM3 dalam encoder mode
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ===== Configuration ===== */
#define ENCODER_MOTOR1_TIM      &htim2
#define ENCODER_MOTOR2_TIM      &htim3
#define ENCODER_COUNTS_PER_REV  1000  /* Sesuaikan dengan encoder spesifikasi */

/* ===== Types ===== */
typedef enum {
    ENCODER_MOTOR1 = 0,
    ENCODER_MOTOR2 = 1
} encoder_id_t;

typedef struct {
    encoder_id_t id;
    int32_t position;        /* Absolute position (counts) */
    int32_t velocity;        /* Velocity (counts per second) */
    uint32_t last_count;     /* Last encoder count */
    uint32_t last_time;      /* Last measurement time */
} encoder_state_t;

/* ===== API ===== */
int encoder_init(void);
int encoder_get_position(encoder_id_t encoder_id, int32_t *position);
int encoder_get_velocity(encoder_id_t encoder_id, int32_t *velocity);
int encoder_reset_position(encoder_id_t encoder_id);
void encoder_update_velocity(void);  /* Call periodically */

#endif /* ENCODER_H */