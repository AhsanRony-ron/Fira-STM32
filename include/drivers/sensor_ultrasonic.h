/**
 * @file sensor_ultrasonic.h
 * @brief Ultrasonic Distance Sensor - Header
 * @detail HC-SR04 ultrasonic sensor untuk autonomous car collision avoidance
 *         Menggunakan TIM5 untuk timing measurement
 */

#ifndef SENSOR_ULTRASONIC_H
#define SENSOR_ULTRASONIC_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ===== Configuration ===== */
#define ULTRASONIC_TRIG_PORT    GPIOB
#define ULTRASONIC_TRIG_PIN     GPIO_PIN_8
#define ULTRASONIC_ECHO_PORT    GPIOB
#define ULTRASONIC_ECHO_PIN     GPIO_PIN_9
#define ULTRASONIC_TIM          &htim5

/* Distance thresholds (cm) */
#define ULTRASONIC_MAX_RANGE    400  /* 4 meters */
#define ULTRASONIC_MIN_RANGE    2    /* 2 cm */
#define ULTRASONIC_COLLISION_THRESHOLD  20  /* 20 cm */

/* ===== Types ===== */
typedef struct {
    uint32_t distance_cm;      /* Distance in cm */
    uint32_t pulse_width_us;   /* Raw pulse width in microseconds */
    uint32_t timestamp_ms;     /* Timestamp when measurement taken */
    uint8_t is_valid;          /* 1 if valid, 0 if out of range */
} ultrasonic_sample_t;

/* ===== API ===== */

/**
 * @brief Initialize ultrasonic sensor
 * @return 0 on success, -1 on error
 */
int ultrasonic_init(void);

/**
 * @brief Trigger distance measurement (non-blocking)
 * @note Call this to start measurement, use ultrasonic_read() to get result
 */
void ultrasonic_trigger(void);

/**
 * @brief Read latest distance measurement
 * @param sample: pointer to ultrasonic_sample_t struct
 * @return 0 if measurement ready, -1 if not ready yet
 */
int ultrasonic_read(ultrasonic_sample_t *sample);

/**
 * @brief Blocking distance read (waits max ~46ms)
 * @param distance_cm: pointer to store distance in cm
 * @return 0 on success, -1 on timeout/error
 */
int ultrasonic_read_blocking(uint32_t *distance_cm);

/**
 * @brief Check if collision threat exists
 * @return 1 if distance < COLLISION_THRESHOLD, 0 otherwise
 */
uint8_t ultrasonic_is_collision_threat(void);

#endif /* SENSOR_ULTRASONIC_H */
