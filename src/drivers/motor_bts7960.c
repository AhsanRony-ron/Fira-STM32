/**
 * @file motor_bts7960.c
 * @brief BTS7960 Motor Driver Implementation
 */

#include "drivers/motor_bts7960.h"

/* Handle timer dari main.c (CubeMX generated) */
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;

/* Local state */
static motor_state_t motor_state[2] = {
    {MOTOR_1, 0, MOTOR_STOP},
    {MOTOR_2, 0, MOTOR_STOP}
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
    /* Enable PWM channels */
    /* Motor 1: TIM1 CH1 & CH2 */
    if (HAL_TIM_PWM_Start(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL1) != HAL_OK) {
        return -1;
    }
    if (HAL_TIM_PWM_Start(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL2) != HAL_OK) {
        return -1;
    }
    
    /* Motor 2: TIM1 CH3 & TIM4 CH1 */
    if (HAL_TIM_PWM_Start(MOTOR_2_PWM_TIMER1, MOTOR_2_PWM_CHANNEL1) != HAL_OK) {
        return -1;
    }
    if (HAL_TIM_PWM_Start(MOTOR_2_PWM_TIMER2, MOTOR_2_PWM_CHANNEL2) != HAL_OK) {
        return -1;
    }
    
    /* Initialize GPIO for enable pins */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MOTOR_1_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MOTOR_1_EN_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = MOTOR_2_EN_PIN;
    HAL_GPIO_Init(MOTOR_2_EN_PORT, &GPIO_InitStruct);
    
    /* Default: stopped */
    motor_stop_all();
    
    return 0;
}

int motor_set_speed(motor_id_t motor_id, int16_t speed)
{
    if (motor_id > MOTOR_2) return -1;
    
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
    
    if (motor_id == MOTOR_1) {
        /* Motor 1: TIM1 CH1 (forward) & CH2 (backward) */
        if (motor_state[motor_id].dir == MOTOR_FORWARD) {
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL1, pwm_value);
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL2, 0);
            HAL_GPIO_WritePin(MOTOR_1_EN_PORT, MOTOR_1_EN_PIN, GPIO_PIN_SET);
        } else if (motor_state[motor_id].dir == MOTOR_BACKWARD) {
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL1, 0);
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL2, pwm_value);
            HAL_GPIO_WritePin(MOTOR_1_EN_PORT, MOTOR_1_EN_PIN, GPIO_PIN_SET);
        } else {
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL1, 0);
            __HAL_TIM_SET_COMPARE(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL2, 0);
            HAL_GPIO_WritePin(MOTOR_1_EN_PORT, MOTOR_1_EN_PIN, GPIO_PIN_RESET);
        }
    } else {
        /* Motor 2: TIM1 CH3 (forward) & TIM4 CH1 (backward) */
        if (motor_state[motor_id].dir == MOTOR_FORWARD) {
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER1, MOTOR_2_PWM_CHANNEL1, pwm_value);
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER2, MOTOR_2_PWM_CHANNEL2, 0);
            HAL_GPIO_WritePin(MOTOR_2_EN_PORT, MOTOR_2_EN_PIN, GPIO_PIN_SET);
        } else if (motor_state[motor_id].dir == MOTOR_BACKWARD) {
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER1, MOTOR_2_PWM_CHANNEL1, 0);
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER2, MOTOR_2_PWM_CHANNEL2, pwm_value);
            HAL_GPIO_WritePin(MOTOR_2_EN_PORT, MOTOR_2_EN_PIN, GPIO_PIN_SET);
        } else {
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER1, MOTOR_2_PWM_CHANNEL1, 0);
            __HAL_TIM_SET_COMPARE(MOTOR_2_PWM_TIMER2, MOTOR_2_PWM_CHANNEL2, 0);
            HAL_GPIO_WritePin(MOTOR_2_EN_PORT, MOTOR_2_EN_PIN, GPIO_PIN_RESET);
        }
    }
    
    return 0;
}

void motor_get_state(motor_id_t motor_id, motor_state_t *state)
{
    if (motor_id <= MOTOR_2 && state) {
        *state = motor_state[motor_id];
    }
}

void motor_stop_all(void)
{
    motor_set_speed(MOTOR_1, 0);
    motor_set_speed(MOTOR_2, 0);
}

void motor_emergency_stop(void)
{
    HAL_TIM_PWM_Stop(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL1);
    HAL_TIM_PWM_Stop(MOTOR_1_PWM_TIMER, MOTOR_1_PWM_CHANNEL2);
    HAL_TIM_PWM_Stop(MOTOR_2_PWM_TIMER1, MOTOR_2_PWM_CHANNEL1);
    HAL_TIM_PWM_Stop(MOTOR_2_PWM_TIMER2, MOTOR_2_PWM_CHANNEL2);
    
    HAL_GPIO_WritePin(MOTOR_1_EN_PORT, MOTOR_1_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_2_EN_PORT, MOTOR_2_EN_PIN, GPIO_PIN_RESET);
    
    motor_state[MOTOR_1].speed = 0;
    motor_state[MOTOR_1].dir = MOTOR_STOP;
    motor_state[MOTOR_2].speed = 0;
    motor_state[MOTOR_2].dir = MOTOR_STOP;
}
