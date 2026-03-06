/**
 * @file servo.c
 * @brief Servo Motor Control Implementation
 */

#include "drivers/servo.h"

extern TIM_HandleTypeDef htim4;

/* Local state */
static servo_state_t servo_state = {
    .angle = 0,
    .pulse_width_us = SERVO_PULSE_CENTER,
    .timestamp_ms = 0
};

/* ===== Internal Helpers ===== */

/**
 * @brief Convert angle (-90 to +90) to pulse width (1000 to 2000 us)
 */
static inline uint16_t angle_to_pulse(int16_t angle)
{
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    
    /* Linear interpolation */
    int16_t delta = angle - SERVO_ANGLE_CENTER;
    uint16_t pulse = SERVO_PULSE_CENTER + (delta * (SERVO_PULSE_MAX - SERVO_PULSE_CENTER) / 90);
    
    return pulse;
}

/**
 * @brief Convert pulse width (1000-2000 us) to angle (-90 to +90)
 */
static inline int16_t pulse_to_angle(uint16_t pulse)
{
    if (pulse < SERVO_PULSE_MIN) pulse = SERVO_PULSE_MIN;
    if (pulse > SERVO_PULSE_MAX) pulse = SERVO_PULSE_MAX;
    
    int32_t delta = pulse - SERVO_PULSE_CENTER;
    int16_t angle = SERVO_ANGLE_CENTER + (delta * 90 / (SERVO_PULSE_MAX - SERVO_PULSE_CENTER));
    
    return angle;
}

/* ===== Implementation ===== */

int servo_init(void)
{
    /* Start PWM */
    if (HAL_TIM_PWM_Start(&htim4, SERVO_PWM_CHANNEL) != HAL_OK) {
        return -1;
    }
    
    /* Set to center position */
    servo_center();
    
    return 0;
}

int servo_set_angle(int16_t angle)
{
    uint16_t pulse = angle_to_pulse(angle);
    return servo_set_pulse(pulse);
}

int servo_set_pulse(uint16_t pulse_us)
{
    if (pulse_us < SERVO_PULSE_MIN || pulse_us > SERVO_PULSE_MAX) {
        return -1;
    }
    
    /* Convert pulse width to PWM compare value */
    /* Assuming 50Hz (20ms period) with 20kHz timer -> ARR = 400 */
    /* 1ms pulse = 20 count, 2ms pulse = 40 count */
    uint32_t compare_value = (pulse_us * SERVO_FREQ_DIV) / 1000000;
    
    __HAL_TIM_SET_COMPARE(&htim4, SERVO_PWM_CHANNEL, compare_value);
    
    servo_state.pulse_width_us = pulse_us;
    servo_state.angle = pulse_to_angle(pulse_us);
    servo_state.timestamp_ms = HAL_GetTick();
    
    return 0;
}

void servo_get_state(servo_state_t *state)
{
    if (state) {
        *state = servo_state;
    }
}

void servo_center(void)
{
    servo_set_angle(SERVO_ANGLE_CENTER);
}
