/**
 * @file sensor_ultrasonic.c
 * @brief Ultrasonic Distance Sensor Implementation
 */

#include "drivers/sensor_ultrasonic.h"

extern TIM_HandleTypeDef htim5;

/* Local state */
static ultrasonic_sample_t latest_sample = {0};
static uint32_t measurement_start_tick = 0;
static uint8_t measurement_in_progress = 0;

/* ===== Implementation ===== */

int ultrasonic_init(void)
{
    /* Initialize GPIO for trigger pin */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ULTRASONIC_TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ULTRASONIC_TRIG_PORT, &GPIO_InitStruct);
    
    /* Echo pin configured as input (assume configured in CubeMX as input capture) */
    
    return 0;
}

void ultrasonic_trigger(void)
{
    /* Send 10us pulse to trigger */
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_SET);
    // HAL_Delay_us(10);  // STM32 HAL doesn't have us delay, using 1ms instead
    HAL_Delay(1);
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_RESET);
    
    measurement_start_tick = HAL_GetTick();
    measurement_in_progress = 1;
}

int ultrasonic_read(ultrasonic_sample_t *sample)
{
    if (!sample) return -1;
    
    if (!measurement_in_progress) {
        *sample = latest_sample;
        return 0;
    }
    
    /* Check timeout (~46ms for max range) */
    if (HAL_GetTick() - measurement_start_tick > 50) {
        measurement_in_progress = 0;
        *sample = latest_sample;
        return 0;
    }
    
    return -1;  /* Not ready yet */
}

int ultrasonic_read_blocking(uint32_t *distance_cm)
{
    if (!distance_cm) return -1;
    
    ultrasonic_trigger();
    
    uint32_t timeout = HAL_GetTick() + 50;
    while (HAL_GetTick() < timeout) {
        if (ultrasonic_read(&latest_sample) == 0) {
            if (latest_sample.is_valid) {
                *distance_cm = latest_sample.distance_cm;
                return 0;
            }
        }
    }
    
    return -1;  /* Timeout */
}

uint8_t ultrasonic_is_collision_threat(void)
{
    return (latest_sample.distance_cm < ULTRASONIC_COLLISION_THRESHOLD && 
            latest_sample.is_valid);
}

/* ===== EXTI Callback (Echo Pin) ===== */
/* Note: Implement this in your EXTI handler when echo pin transitions */
void ultrasonic_echo_callback(uint8_t echo_pin_state)
{
    static uint32_t echo_start = 0;
    
    if (echo_pin_state == 1) {
        /* Echo pulse start */
        echo_start = HAL_GetTick();
    } else {
        /* Echo pulse end - calculate distance */
        uint32_t pulse_duration_us = (HAL_GetTick() - echo_start) * 1000;
        
        /* Distance (cm) = pulse_duration (us) / 58 */
        uint32_t distance = pulse_duration_us / 58;
        
        if (distance <= ULTRASONIC_MAX_RANGE && distance >= ULTRASONIC_MIN_RANGE) {
            latest_sample.distance_cm = distance;
            latest_sample.pulse_width_us = pulse_duration_us;
            latest_sample.is_valid = 1;
        } else {
            latest_sample.is_valid = 0;
        }
        
        latest_sample.timestamp_ms = HAL_GetTick();
        measurement_in_progress = 0;
    }
}
