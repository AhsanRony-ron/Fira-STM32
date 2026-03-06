# Control Loop README

## Overview

The control loop is the **heart** of the autonomous car system. It coordinates all sensors and actuators in a real-time closed-loop control structure.

## Architecture

```
Control Loop (50 Hz)
   ↓
   ├─→ Read Sensors
   │   ├── IMU (accel, gyro)
   │   ├── Ultrasonic (distance)
   │   └── Motor feedback (via state tracking)
   │
   ├─→ Process Logic (based on mode)
   │   ├── MANUAL: Pass-through remote commands
   │   ├── AUTONOMOUS: Execute navigation/collision avoidance
   │   └── FAILSAFE: Emergency stop
   │
   └─→ Command Actuators
       ├── Motor speed PWM
       ├── Servo angle
       └── Status/telemetry
```

## Modes

### Manual Mode
- Remote control via USB UART
- Direct pass-through of motor & servo commands
- Still detects collisions for safety (optional emergency stop)

```c
control_set_mode(CONTROL_MODE_MANUAL);
control_manual_command(50, 45);  // 50% forward, 45° steering angle
```

### Autonomous Mode
- Self-contained decision making
- Collision avoidance active
- Following implemented algorithm (line following, obstacle detection, etc)
- Example placeholder: moves forward until obstacle detected

```c
control_set_mode(CONTROL_MODE_AUTONOMOUS);
// Then control_loop_update() runs autonomously
```

### Failsafe Mode
- All motors disabled
- Servo centered
- Triggered by timeout or explicit command

```c
control_failsafe();  // Enter failsafe immediately
```

## PID Control

The system includes basic PID controller for potential tuning:

```c
// Steering PID (for keeping straight line)
static pid_controller_t steering_pid = {
    .kp = 0.5f,   // Proportional gain
    .ki = 0.1f,   // Integral gain
    .kd = 0.2f    // Derivative gain
};

// Speed PID (for maintaining target speed)
static pid_controller_t speed_pid = {
    .kp = 1.0f,
    .ki = 0.05f,
    .kd = 0.1f
};
```

### Tuning Tips
1. **Start with Kp only**: Set Ki and Kd to 0, increase Kp until oscillation
2. **Add Kd to dampen**: Reduce overshoot
3. **Use Ki for steady-state error**: Small values (usually <0.1 × Kp)
4. **Test on real hardware**: Simulation never matches reality

## Implementation Guide

### Basic Usage

```c
// Initialize system
control_init();

// Main loop (call every 20ms for 50Hz)
uint32_t last_time = HAL_GetTick();

while (1) {
    if (HAL_GetTick() - last_time >= 20) {
        last_time = HAL_GetTick();
        
        control_loop_update();  // Main control logic
        
        // Optional: Send telemetry
        control_state_t state;
        control_get_state(&state);
        // → Send to PC/RPi/Jetson via USB
    }
}
```

### Implementing Custom Autonomous Logic

Edit `control_autonomous_mode()` in `control_loop.c`:

```c
static void control_autonomous_mode(void)
{
    // Read sensor data
    ultrasonic_sample_t ultrasonic;
    motor_state_t motor_right, motor_left;
    servo_state_t servo;
    
    // TODO: Your algorithm here
    // Example: Line follower
    if (line_sensor_detects_left) {
        control_state.servo_angle = -30;  // Turn left
    } else if (line_sensor_detects_right) {
        control_state.servo_angle = 30;   // Turn right
    } else {
        control_state.servo_angle = 0;    // Go straight
    }
    
    // Always check collision
    if (ultrasonic_is_collision_threat()) {
        control_state.motor_speed = 0;  // Stop
    } else {
        control_state.motor_speed = 75; // Forward
    }
    
    // Apply commands
    motor_set_speed(MOTOR_RIGHT, control_state.motor_speed);
    motor_set_speed(MOTOR_LEFT, control_state.motor_speed);
    servo_set_angle(control_state.servo_angle);
}
```

## State Structure

```c
typedef struct {
    int16_t accel[3];           // IMU acceleration
    int16_t gyro[3];            // IMU angular velocity
    uint32_t distance_front_cm; // Ultrasonic distance
    int16_t motor_speed;        // Current motor command
    int16_t servo_angle;        // Current servo angle
    uint32_t timestamp_ms;      // When measured
    uint8_t collision_detected; // Safety flag
} control_state_t;
```

## Timing & Performance

- **Control loop frequency**: 50 Hz (20 ms period)
- **Timeout threshold**: 5 seconds without update → failsafe
- **Sensor reading**: Non-blocking by design
- **Actuation latency**: <5 ms (PWM update immediate)

## Debugging

Enable USB CDC output to monitor:

```c
control_state_t state;
control_get_state(&state);

printf("Motor: %d%%, Servo: %d°, Distance: %d cm\r\n",
       state.motor_speed, state.servo_angle, state.distance_front_cm);
```

## Safety Considerations

1. **Always implement failsafe**: Timeout stops all motors
2. **Collision detection**: Ultrasonic threshold prevents crashes
3. **Speed limiting**: Cap motor PWM to safe levels
4. **Emergency button**: External switch on failsafe pin recommended
5. **Testing environment**: Use safe enclosed space until tuned
