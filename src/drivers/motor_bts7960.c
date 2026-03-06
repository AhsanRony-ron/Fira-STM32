/**
 * @file motor_bts7960.c
 * @brief BTS7960 Motor Driver Implementation
 */

#include "drivers/motor_bts7960.h"

/* Handle timer dari main.c (CubeMX generated) */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

/* Local state */
static motor_state_t motor_state[2] = {
    {MOTOR_RIGHT, 0, MOTOR_STOP},
    {MOTOR_LEFT, 0, MOTOR_STOP}
};

/* ===== Internal Helpers ===== */
static inline uint32_t speed_to_pwm(int16_t speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;
    return (uint32_t)((speed * 65535) / 100);
}

/* ===== Implementation ===== */

int motor_init(void)
{
    /* Enable PWM on TIM2 & TIM3 */
    if (HAL_TIM_PWM_Start(&htim2, MOTOR_RIGHT_PWM_CHANNEL) != HAL_OK) {
        return -1;
    }
    if (HAL_TIM_PWM_Start(&htim3, MOTOR_LEFT_PWM_CHANNEL) != HAL_OK) {
        return -1;
    }
    
    /* Initialize GPIO for enable pins */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MOTOR_RIGHT_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MOTOR_RIGHT_EN_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = MOTOR_LEFT_EN_PIN;
    HAL_GPIO_Init(MOTOR_LEFT_EN_PORT, &GPIO_InitStruct);
    
    /* Default: stopped */
    motor_stop_all();
    
    return 0;
}

int motor_set_speed(motor_id_t motor_id, int16_t speed)
{
    if (motor_id > MOTOR_LEFT) return -1;
    
    motor_state[motor_id].speed = speed;
    
    if (speed > 0) {
        motor_state[motor_id].dir = MOTOR_FORWARD;
    } else if (speed < 0) {
        motor_state[motor_id].dir = MOTOR_BACKWARD;
        speed = -speed;  /* Make positive for PWM */
    } else {
        motor_state[motor_id].dir = MOTOR_STOP;
    }
    
    uint32_t pwm_value = speed_to_pwm(speed);
    
    if (motor_id == MOTOR_RIGHT) {
        HAL_TIM_PWM_Stop(&htim2, MOTOR_RIGHT_PWM_CHANNEL);
        __HAL_TIM_SET_COMPARE(&htim2, MOTOR_RIGHT_PWM_CHANNEL, pwm_value);
        if (speed > 0) {
            HAL_TIM_PWM_Start(&htim2, MOTOR_RIGHT_PWM_CHANNEL);
            HAL_GPIO_WritePin(MOTOR_RIGHT_EN_PORT, MOTOR_RIGHT_EN_PIN, GPIO_PIN_SET);
        }
    } else {
        HAL_TIM_PWM_Stop(&htim3, MOTOR_LEFT_PWM_CHANNEL);
        __HAL_TIM_SET_COMPARE(&htim3, MOTOR_LEFT_PWM_CHANNEL, pwm_value);
        if (speed > 0) {
            HAL_TIM_PWM_Start(&htim3, MOTOR_LEFT_PWM_CHANNEL);
            HAL_GPIO_WritePin(MOTOR_LEFT_EN_PORT, MOTOR_LEFT_EN_PIN, GPIO_PIN_SET);
        }
    }
    
    return 0;
}

void motor_get_state(motor_id_t motor_id, motor_state_t *state)
{
    if (motor_id <= MOTOR_LEFT && state) {
        *state = motor_state[motor_id];
    }
}

void motor_stop_all(void)
{
    motor_set_speed(MOTOR_RIGHT, 0);
    motor_set_speed(MOTOR_LEFT, 0);
}

void motor_emergency_stop(void)
{
    HAL_TIM_PWM_Stop(&htim2, MOTOR_RIGHT_PWM_CHANNEL);
    HAL_TIM_PWM_Stop(&htim3, MOTOR_LEFT_PWM_CHANNEL);
    HAL_GPIO_WritePin(MOTOR_RIGHT_EN_PORT, MOTOR_RIGHT_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_LEFT_EN_PORT, MOTOR_LEFT_EN_PIN, GPIO_PIN_RESET);
    
    motor_state[MOTOR_RIGHT].speed = 0;
    motor_state[MOTOR_RIGHT].dir = MOTOR_STOP;
    motor_state[MOTOR_LEFT].speed = 0;
    motor_state[MOTOR_LEFT].dir = MOTOR_STOP;
}
