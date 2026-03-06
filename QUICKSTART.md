# Quick Start Guide

## ⚡ 5-Minute Setup

### 1. Copy Missing Files (2 min)
```powershell
# PowerShell - copy system & USB files
cd "c:\Users\ASUS\Documents\PlatformIO\Projects\Fira STM32"

# System files
Copy-Item "src/Aditional/system_stm32f4xx.c" "src/system/system_stm32f4xx.c" -Force
Copy-Item "src/Aditional/stm32f4xx_it.c" "src/system/stm32f4xx_it.c" -Force
Copy-Item "src/Aditional/stm32f4xx_hal_msp.c" "src/system/stm32f4xx_hal_msp.c" -Force

# USB files
Copy-Item "src/Aditional/usb_device.c" "src/usb/usb_device.c" -Force
Copy-Item "src/Aditional/usbd_cdc_if.c" "src/usb/usbd_cdc_if.c" -Force
Copy-Item "src/Aditional/usbd_conf.c" "src/usb/usbd_conf.c" -Force
Copy-Item "src/Aditional/usbd_desc.c" "src/usb/usbd_desc.c" -Force

Write-Host "✓ Files copied successfully"
```

### 2. Verify Includes (30 sec)
Check `platformio.ini`:
```ini
build_flags = 
    -Iinclude
    -Iinclude/drivers
    -Iinclude/control
    -Iinclude/system
    -Iinclude/usb
```
✓ Already updated!

### 3. First Build (2 min)
```bash
cd "c:\Users\ASUS\Documents\PlatformIO\Projects\Fira STM32"
platformio run
```

### 4. Upload & Monitor (1 min)
```bash
platformio run --target upload
platformio device monitor
```

---

## 📋 Hardware Configuration Checklist

Edit these files with your actual GPIO pins:

### Motor (include/drivers/motor_bts7960.h)
```c
#define MOTOR_RIGHT_EN_PORT    GPIOA
#define MOTOR_RIGHT_EN_PIN     GPIO_PIN_0
#define MOTOR_LEFT_EN_PORT     GPIOA
#define MOTOR_LEFT_EN_PIN      GPIO_PIN_1
```

### Servo (include/drivers/servo.h)
- Connected to TIM4 CH1 (already configured in main.c)

### Ultrasonic (include/drivers/sensor_ultrasonic.h)
```c
#define ULTRASONIC_TRIG_PORT   GPIOB
#define ULTRASONIC_TRIG_PIN    GPIO_PIN_2
#define ULTRASONIC_ECHO_PORT   GPIOB
#define ULTRASONIC_ECHO_PIN    GPIO_PIN_3
```

### IMU (include/drivers/imu_mpu6500.h)
```c
#define MPU_NCS_GPIO_PORT      GPIOB        // SPI chip select
#define MPU_NCS_PIN            GPIO_PIN_1
#define MPU_INT_PORT           GPIOB        // Data ready interrupt
#define MPU_INT_PIN            GPIO_PIN_0
```

---

## 🧪 Component Testing

### Test 1: Build Succeeds
```bash
platformio run
```
Expected: `[SUCCESS]` no errors

### Test 2: Upload Works
```bash
platformio run --target upload
```
Expected: `[SUCCESS]` device ready

### Test 3: Monitor Connection
```bash
platformio device monitor
```
Expected: Serial connection active, no garbage

### Test 4: Individual Components
In USB terminal (after uploading):
```c
// Test IMU
int who = mpu6500_init();  // Should return 0x70

// Test Motor
motor_init();
motor_set_speed(MOTOR_RIGHT, 50);  // Motor should turn at 50%

// Test Servo
servo_init();
servo_set_angle(45);  // Servo should turn right

// Test Ultrasonic
ultrasonic_init();
uint32_t distance_cm;
ultrasonic_read_blocking(&distance_cm);  // Should show distance
```

---

## 📚 Documentation

Read these in order:

1. **STRUCTURE.md** - Project layout overview
2. **README.md** - Full documentation
3. **SETUP_GUIDE.md** - Detailed integration steps
4. **src/drivers/README.md** - Driver specs & pin config
5. **src/control/README.md** - Control logic & PID tuning
6. **src/usb/README.md** - USB debug output

---

## ⚙️ Main Loop (src/main.c)

The main control loop already includes:
```c
// In main() initialization:
// mpu6500_init() is called
// All timers are started

// In while loop:
if (mpu_data_ready) {
    // IMU data automatically read
    mpu_data_ready = 0;
}

if (HAL_GetTick() - last_tick > 200) {
    // Fallback periodic read
    last_tick = HAL_GetTick();
}

// TODO: Add your control logic here
```

### To Enable Full Control:

Add after timer initialization in main():
```c
control_init();

uint32_t control_tick = HAL_GetTick();
```

In main while loop:
```c
if (HAL_GetTick() - control_tick >= 20) {  // 50 Hz
    control_tick = HAL_GetTick();
    control_loop_update();
}
```

---

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| Build fails - "include not found" | Check platformio.ini build_flags |
| "undefined reference to" | Missing copy from Aditional/ folder |
| Motor not moving | Check GPIO pins and TIM2/TIM3 enabled |
| Servo not responding | Check TIM4 and servo pulse range |
| IMU returns 0 | Check SPI speed and CS pin toggling |
| Ultrasonic no reading | Check trigger pin high/low pulse |

---

## 📊 Project Structure at a Glance

```
✓ READY:           NEW & CONFIGURED
├── platformio.ini (updated)
├── main.c (updated with includes)
├── src/drivers/ (new: motor, servo, ultrasonic, imu)
├── src/control/ (new: control loop)
├── README.md, SETUP_GUIDE.md, STRUCTURE.md (documentation)

⧖ PENDING:         COPY FROM src/Aditional/
├── src/system/ (needs: system_stm32f4xx.c, stm32f4xx_it.c, stm32f4xx_hal_msp.c)
└── src/usb/ (needs: usb_device.c, usbd_cdc_if.c, usbd_conf.c, usbd_desc.c)
```

---

## 🚀 Next Steps

1. **Copy missing files** (see step 1 above)
2. **Update hardware pins** (GPIO, timer)
3. **Build & upload**: `platformio run --target upload`
4. **Test components** individually
5. **Integrate control** system
6. **Implement your logic** in control_autonomous_mode()
7. **Tune PID** parameters
8. **Deploy** on your autonomous car

---

## 📞 Support References

- **Motor Issues**: Check src/drivers/README.md - Motor section
- **Sensor Questions**: Check src/drivers/README.md - respective sensor
- **Control Tuning**: Check src/control/README.md - PID section
- **USB Communication**: Check src/usb/README.md - Protocol section
- **System Files**: Check src/system/README.md - Integration notes

---

**Status**: 🟢 Ready to compile and upload!

Copy those 8 files, update your GPIO pins, and `platformio run` 🚀
