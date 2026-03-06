# Autonomous Car Close-Loop Control System
## Project Structure

*For detailed communication setup, see [`COMMUNICATION_SUMMARY.md`](COMMUNICATION_SUMMARY.md)*

```
src/
├── main.c                      # Entry point & main loop
├── drivers/                    # Hardware drivers
│   ├── imu_mpu6500.c          # MPU6500 IMU sensor (SPI)
│   ├── imu_mpu6500.h          # IMU header
│   ├── motor_bts7960.c        # BTS7960 motor driver
│   ├── motor_bts7960.h        # Motor driver header
│   ├── servo.c                # RC servo control
│   ├── servo.h                # Servo header
│   ├── sensor_ultrasonic.c    # HC-SR04 ultrasonic sensor
│   ├── sensor_ultrasonic.h    # Ultrasonic header
│   └── README.md              # Driver documentation
├── control/                   # Control logic
│   ├── control_loop.c         # Main close-loop controller
│   ├── control_loop.h         # Controller header
│   └── README.md              # Control logic documentation
├── system/                    # STM32 system files
│   ├── system_stm32f4xx.c     # System initialization
│   ├── stm32f4xx_it.c         # Interrupt handlers
│   └── stm32f4xx_hal_msp.c    # HAL MSP callbacks
├── usb/                       # USB CDC communication
│   ├── usbd_cdc_if.c         # USB CDC interface
│   ├── usbd_conf.c           # USB configuration
│   ├── usbd_desc.c           # USB descriptor
│   └── usb_device.c          # USB device init
└── Aditional/                 # Original system files (deprecated)
    └── [old files...]

include/
├── main.h
├── drivers/
│   ├── imu_mpu6500.h
│   ├── motor_bts7960.h
│   ├── sensor_ultrasonic.h
│   └── servo.h
├── control/
│   └── control_loop.h
├── system/
├── usb/
└── [other STM32 headers]
```

## Component Description

### Drivers (src/drivers/)
- **imu_mpu6500**: 6-axis IMU sensor via SPI - provides acceleration & angular velocity
- **motor_bts7960**: Dual H-bridge motor driver for left/right wheel motors
- **servo**: RC servo control for steering
- **sensor_ultrasonic**: HC-SR04 ultrasonic sensor for collision detection

### Control (src/control/)
- **control_loop**: Main close-loop controller that:
  - Reads all sensors (IMU, ultrasonic, motor feedback)
  - Processes control logic (collision avoidance, PID loops)
  - Commands actuators (motors, servo)
  - Supports autonomous & manual modes

### System (src/system/)
Files copied from STM32CubeMX generation:
- system_stm32f4xx.c - Clock configuration
- stm32f4xx_it.c - Interrupt handlers
- stm32f4xx_hal_msp.c - HAL MSP callbacks

### USB (src/usb/)
USB CDC (virtual COM port) for:
- Debug logging
- Telemetry transmission
- Remote control commands

## Timer Allocation

| Timer | Function | Purpose |
|-------|----------|---------|
| TIM1  | Software timer | System tick |
| TIM2  | PWM CH1  | Right motor speed control |
| TIM3  | PWM CH1  | Left motor speed control |
| TIM4  | PWM CH1  | Servo steering |
| TIM5  | PWM      | Reserved/Extra |
| TIM9  | Ultrasonic timing | Echo pulse measurement |

## Next Steps

### 1. Update Include Paths
In platformio.ini, ensure:
```
build_flags = 
  -Iinclude
  -Iinclude/drivers
  -Iinclude/control
  -Iinclude/system
  -Iinclude/usb
```

### 2. Move/Copy System Files
Copy from src/Aditional/ to src/system/:
- system_stm32f4xx.c → src/system/
- stm32f4xx_it.c → src/system/
- stm32f4xx_hal_msp.c → src/system/

### 3. Update main.c
Already includes drivers, just verify includes:
```c
#include "drivers/imu_mpu6500.h"
#include "drivers/motor_bts7960.h"
#include "drivers/servo.h"
#include "drivers/sensor_ultrasonic.h"
#include "control/control_loop.h"
```

### 4. Implement Hardware Configuration
Verify GPIO pins and timer assignments in:
- `include/drivers/motor_bts7960.h` - Motor PWM pins
- `include/drivers/servo.h` - Servo PWM pin
- `include/drivers/sensor_ultrasonic.h` - Ultrasonic trigger/echo pins
- `include/drivers/imu_mpu6500.h` - SPI pins for IMU

### 5. Tune Control Parameters
In `src/control/control_loop.c`, adjust PID coefficients:
```c
static pid_controller_t steering_pid = {
    .kp = 0.5f,   // ← Proportional gain
    .ki = 0.1f,   // ← Integral gain
    .kd = 0.2f    // ← Derivative gain
};
```

### 6. Implement Autonomous Logic
In `control_autonomous_mode()` function, add your navigation algorithm:
- Line follower
- Obstacle avoidance
- Path following
- AI-based planning
