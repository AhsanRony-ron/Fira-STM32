/**
 * @file control_loop.c
 * @brief Autonomous Car Close-Loop Control System Implementation
 */

#include "control/control_loop.h"
#include "drivers/motor_bts7960.h"
#include "drivers/servo.h"
#include "drivers/sensor_ultrasonic.h"
#include "drivers/imu_mpu6500.h"

/* ===== External Data ===== */
extern imu_sample_t imu_latest;

/* ===== Local State ===== */
static control_mode_t control_mode = CONTROL_MODE_MANUAL;
static control_state_t control_state = {0};
static uint32_t last_control_tick = 0;

/* PID Controller Parameters (tuning required) */
typedef struct {
    float kp, ki, kd;
    float integral;
    float last_error;
} pid_controller_t;

// static pid_controller_t steering_pid = {
//     .kp = 0.5f,
//     .ki = 0.1f,
//     .kd = 0.2f,
//     .integral = 0.f,
//     .last_error = 0.f
// };

// static pid_controller_t speed_pid = {
//     .kp = 1.0f,
//     .ki = 0.05f,
//     .kd = 0.1f,
//     .integral = 0.f,
//     .last_error = 0.f
// };

/* ===== Internal Helpers ===== */

/**
 * @brief Simple PID controller
 */
// static float pid_update(pid_controller_t *pid, float error, float dt)
// {
//     if (dt <= 0) return 0;
    
//     pid->integral += error * dt;
//     if (pid->integral > 100) pid->integral = 100;
//     if (pid->integral < -100) pid->integral = -100;
    
//     float derivative = (error - pid->last_error) / dt;
//     pid->last_error = error;
    
//     float output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
    
//     if (output > 100) output = 100;
//     if (output < -100) output = -100;
    
//     return output;
// }

/**
 * @brief Autonomous mode: collision avoidance + heading hold
 */
static void control_autonomous_mode(void)
{
    /* Read sensor data */
    motor_state_t motor_right, motor_left;
    servo_state_t servo;
    // ultrasonic_sample_t ultrasonic;  // TODO: implement ultrasonic sensor
    
    motor_get_state(MOTOR_RIGHT, &motor_right);
    motor_get_state(MOTOR_LEFT, &motor_left);
    servo_get_state(&servo);
    
    control_state.accel[0] = imu_latest.accel[0];
    control_state.accel[1] = imu_latest.accel[1];
    control_state.accel[2] = imu_latest.accel[2];
    control_state.gyro[0] = imu_latest.gyro[0];
    control_state.gyro[1] = imu_latest.gyro[1];
    control_state.gyro[2] = imu_latest.gyro[2];
    
    /* Collision avoidance */
    if (ultrasonic_is_collision_threat()) {
        control_state.collision_detected = 1;
        motor_set_speed(MOTOR_RIGHT, 0);
        motor_set_speed(MOTOR_LEFT, 0);
        servo_center();
        control_state.motor_speed = 0;
        control_state.servo_angle = 0;
    } else {
        control_state.collision_detected = 0;
        
        /* TODO: Implement your autonomous logic here */
        /* Example: simple line follower, obstacle avoidance, etc */
        
        control_state.motor_speed = 50;  /* Default: forward at 50% */
        control_state.servo_angle = 0;   /* Straight ahead */
        
        motor_set_speed(MOTOR_RIGHT, control_state.motor_speed);
        motor_set_speed(MOTOR_LEFT, control_state.motor_speed);
        servo_set_angle(control_state.servo_angle);
    }
}

/**
 * @brief Manual mode: pass-through remote control commands
 */
static void control_manual_mode(void)
{
    /* Commands set via control_manual_command() are applied directly */
}

/**
 * @brief Failsafe mode: stop all
 */
static void control_failsafe_mode(void)
{
    motor_emergency_stop();
    servo_center();
    control_state.motor_speed = 0;
    control_state.servo_angle = 0;
    control_state.collision_detected = 1;
}

/* ===== API Implementation ===== */

int control_init(void)
{
    if (motor_init() != 0) return -1;
    if (servo_init() != 0) return -1;
    if (ultrasonic_init() != 0) return -1;
    
    last_control_tick = HAL_GetTick();
    control_mode = CONTROL_MODE_MANUAL;
    
    return 0;
}

void control_loop_update(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t dt_ms = now - last_control_tick;
    
    /* Check timeout */
    if (dt_ms > CONTROL_LOOP_TIMEOUT_MS) {
        control_mode = CONTROL_MODE_FAILSAFE;
    }
    
    last_control_tick = now;
    
    /* Update based on mode */
    switch (control_mode) {
        case CONTROL_MODE_AUTONOMOUS:
            control_autonomous_mode();
            break;
        case CONTROL_MODE_MANUAL:
            control_manual_mode();
            break;
        case CONTROL_MODE_FAILSAFE:
            control_failsafe_mode();
            break;
        default:
            control_failsafe_mode();
    }
    
    control_state.timestamp_ms = now;
}

void control_set_mode(control_mode_t mode)
{
    control_mode = mode;
    
    if (mode == CONTROL_MODE_FAILSAFE) {
        control_failsafe_mode();
    }
}

control_mode_t control_get_mode(void)
{
    return control_mode;
}

void control_get_state(control_state_t *state)
{
    if (state) {
        *state = control_state;
    }
}

void control_manual_command(int16_t motor_speed, int16_t servo_angle)
{
    if (control_mode == CONTROL_MODE_MANUAL) {
        /* Allow -1 to mean "don't change" */
        if (motor_speed != -1) {
            control_state.motor_speed = motor_speed;
            motor_set_speed(MOTOR_RIGHT, motor_speed);
            motor_set_speed(MOTOR_LEFT, motor_speed);
        }
        
        if (servo_angle != -1) {
            control_state.servo_angle = servo_angle;
            servo_set_angle(servo_angle);
        }
    }
}

void control_failsafe(void)
{
    control_set_mode(CONTROL_MODE_FAILSAFE);
}
