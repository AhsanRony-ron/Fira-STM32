/**
 * @file encoder.c
 * @brief Motor Encoder Driver Implementation
 */

#include "drivers/encoder.h"

/* Handle timer dari main.c (CubeMX generated) */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

/* Local state */
static encoder_state_t encoder_state[2] = {
    {ENCODER_MOTOR1, 0, 0, 0, 0},
    {ENCODER_MOTOR2, 0, 0, 0, 0}
};

/* ===== Implementation ===== */

int encoder_init(void)
{
    /* Start encoder mode for TIM2 and TIM3 */
    if (HAL_TIM_Encoder_Start(ENCODER_MOTOR1_TIM, TIM_CHANNEL_ALL) != HAL_OK) {
        return -1;
    }
    if (HAL_TIM_Encoder_Start(ENCODER_MOTOR2_TIM, TIM_CHANNEL_ALL) != HAL_OK) {
        return -1;
    }

    /* Initialize timestamps */
    encoder_state[ENCODER_MOTOR1].last_time = HAL_GetTick();
    encoder_state[ENCODER_MOTOR2].last_time = HAL_GetTick();

    return 0;
}

int encoder_get_position(encoder_id_t encoder_id, int32_t *position)
{
    if (!position || encoder_id > ENCODER_MOTOR2) return -1;

    TIM_HandleTypeDef *htim = (encoder_id == ENCODER_MOTOR1) ? ENCODER_MOTOR1_TIM : ENCODER_MOTOR2_TIM;
    uint32_t current_count = __HAL_TIM_GET_COUNTER(htim);

    /* Handle counter overflow/underflow for incremental encoders */
    int32_t delta = (int32_t)current_count - (int32_t)encoder_state[encoder_id].last_count;

    /* For 16-bit timers, handle wraparound */
    if (delta > 32767) delta -= 65536;
    if (delta < -32767) delta += 65536;

    encoder_state[encoder_id].position += delta;
    encoder_state[encoder_id].last_count = current_count;

    *position = encoder_state[encoder_id].position;
    return 0;
}

int encoder_get_velocity(encoder_id_t encoder_id, int32_t *velocity)
{
    if (!velocity || encoder_id > ENCODER_MOTOR2) return -1;

    *velocity = encoder_state[encoder_id].velocity;
    return 0;
}

int encoder_reset_position(encoder_id_t encoder_id)
{
    if (encoder_id > ENCODER_MOTOR2) return -1;

    TIM_HandleTypeDef *htim = (encoder_id == ENCODER_MOTOR1) ? ENCODER_MOTOR1_TIM : ENCODER_MOTOR2_TIM;

    /* Reset hardware counter */
    __HAL_TIM_SET_COUNTER(htim, 0);

    /* Reset state */
    encoder_state[encoder_id].position = 0;
    encoder_state[encoder_id].last_count = 0;
    encoder_state[encoder_id].velocity = 0;

    return 0;
}

void encoder_update_velocity(void)
{
    static uint32_t last_update = 0;
    uint32_t current_time = HAL_GetTick();

    /* Update every 10ms for velocity calculation */
    if (current_time - last_update < 10) return;
    last_update = current_time;

    for (int i = 0; i < 2; i++) {
        encoder_id_t id = (encoder_id_t)i;
        int32_t current_position;
        encoder_get_position(id, &current_position);

        /* Calculate velocity (counts per second) */
        uint32_t dt = current_time - encoder_state[i].last_time;
        if (dt > 0) {
            int32_t delta_pos = current_position - encoder_state[i].position;
            encoder_state[i].velocity = (delta_pos * 1000) / dt;  /* counts per second */
        }

        encoder_state[i].last_time = current_time;
    }
}