# Drivers README

## Motor Driver (motor_bts7960.h/c)

### Features
- Controls 2 DC motors (left & right wheels)
- Speed control via PWM: -100 (reverse) to +100 (forward)
- BTS7960 dual H-bridge IC support

### API
```c
motor_init();                          // Initialize drivers
motor_set_speed(MOTOR_RIGHT, 75);      // Set right motor to 75% forward
motor_set_speed(MOTOR_LEFT, -50);      // Set left motor to 50% reverse
motor_stop_all();                      // Soft stop
motor_emergency_stop();                // Hard stop (disable outputs)
```

### Configuration
Edit macros in header:
- `MOTOR_*_PWM_TIMER` - Timer handle
- `MOTOR_*_PWM_CHANNEL` - Timer channel
- `MOTOR_*_EN_PIN` - Enable pin

---

## Ultrasonic Sensor (sensor_ultrasonic.h/c)

### Features
- HC-SR04 ultrasonic distance measurement
- Range: 2-400 cm
- Non-blocking API

### API
```c
ultrasonic_init();
ultrasonic_trigger();                  // Start measurement
ultrasonic_read_blocking(&distance);   // Wait for result (blocking)
ultrasonic_is_collision_threat();      // Check if close enough
```

### Configuration
- `COLLISION_THRESHOLD`: 20 cm (adjust as needed)
- Requires EXTI on echo pin for timing

---

## Servo Motor (servo.h/c)

### Features
- Standard RC servo (50 Hz, 1-2 ms pulse)
- Angle control: -90° (left) to +90° (right)
- Non-blocking PWM control

### API
```c
servo_init();
servo_set_angle(45);                   // 45° right
servo_center();                        // 0° (straight)
servo_set_pulse(1500);                 // Manual pulse width
```

### Calibration
Adjust these macros for your servo:
- `SERVO_PULSE_MIN` - Full left angle pulse
- `SERVO_PULSE_CENTER` - Neutral position
- `SERVO_PULSE_MAX` - Full right angle pulse

---

## IMU Sensor (imu_mpu6500.h/c)

### Features
- 6-axis IMU: 3-axis accelerometer + 3-axis gyroscope
- SPI interface
- Data-ready interrupt support

### API
```c
mpu6500_init();                        // Initialize (returns WHO_AM_I)
mpu6500_read_accel_gyro(accel, gyro, temp);  // Read measurements
```

### Data Format
- Accel: ±2g range, units in mg
- Gyro: ±250 dps range, units in dps
- Temp: Raw value (requires conversion if needed)

### Interrupt
- Connected to MCU EXTI pin
- ISR sets flag, main loop reads data
- Prevents blocking in real-time control loop

---

## Configuration Checklist

- [ ] Verify all GPIO pins match your PCB design
- [ ] Verify timer and channel assignments
- [ ] Calibrate servo pulse width range
- [ ] Test each driver independently
- [ ] Verify interrupt priorities for EXTI handlers
- [ ] Check SPI clock speed for IMU (should be ~1-10 MHz)
