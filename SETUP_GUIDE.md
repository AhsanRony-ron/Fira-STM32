# Setup Integration Guide

## Overview

Struktur proyek telah disusun ulang menjadi modular dan terorganisir. Panduan ini membantu Anda mengintegrasikan file-file yang sudah ada ke struktur baru.

## Step 1: Verify New Structure

Pastikan semua folder berikut sudah ada:
```
src/
├── drivers/          ✓ Dibuat
├── control/          ✓ Dibuat
├── system/           ✓ Dibuat
├── usb/              ✓ Dibuat
└── Aditional/        (lama - untuk kopian file)

include/
├── drivers/          ✓ Dibuat
├── control/          ✓ Dibuat
├── system/           ✓ Dibuat
└── usb/              ✓ Dibuat
```

## Step 2: Copy System Files

File-file dari `src/Aditional/` perlu dipindahkan ke `src/system/`:

### Option A: Manual Copy

1. Buka File Explorer
2. Navigate ke `src/Aditional/`
3. Copy file berikut ke `src/system/`:
   - `system_stm32f4xx.c` → `src/system/system_stm32f4xx.c`
   - `stm32f4xx_it.c` → `src/system/stm32f4xx_it.c`
   - `stm32f4xx_hal_msp.c` → `src/system/stm32f4xx_hal_msp.c`

4. Copy file berikut ke `src/usb/`:
   - `usb_device.c` → `src/usb/usb_device.c`
   - `usbd_cdc_if.c` → `src/usb/usbd_cdc_if.c`
   - `usbd_conf.c` → `src/usb/usbd_conf.c`
   - `usbd_desc.c` → `src/usb/usbd_desc.c`

### Option B: Terminal Command (PowerShell)

```powershell
# Copy system files
Copy-Item "src/Aditional/system_stm32f4xx.c" "src/system/system_stm32f4xx.c" -Force
Copy-Item "src/Aditional/stm32f4xx_it.c" "src/system/stm32f4xx_it.c" -Force
Copy-Item "src/Aditional/stm32f4xx_hal_msp.c" "src/system/stm32f4xx_hal_msp.c" -Force

# Copy USB files
Copy-Item "src/Aditional/usb_device.c" "src/usb/usb_device.c" -Force
Copy-Item "src/Aditional/usbd_cdc_if.c" "src/usb/usbd_cdc_if.c" -Force
Copy-Item "src/Aditional/usbd_conf.c" "src/usb/usbd_conf.c" -Force
Copy-Item "src/Aditional/usbd_desc.c" "src/usb/usbd_desc.c" -Force
```

## Step 3: Update Include Paths in Copied Files

### stm32f4xx_it.c

Update callback includes jika ada references ke IMU:

```c
// Add near top of file:
#include "drivers/imu_mpu6500.h"

// In interrupt handlers yang ada, tambahkan callback jika diperlukan:
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MPU_INT_PIN)
    {
        // This will be called from main's EXTI handler
        mpu_data_ready = 1;
    }
}
```

### usbd_cdc_if.c

Pastikan include path sudah benar untuk control system jika needed:

```c
// Jika ingin mengintegrasikan USB dengan control:
#include "control/control_loop.h"
```

## Step 4: Verify Include Paths

Pastikan `platformio.ini` sudah update dengan build_flags:

```ini
build_flags = 
    -Iinclude
    -Iinclude/drivers
    -Iinclude/control
    -Iinclude/system
    -Iinclude/usb
```

## Step 5: Move Motor/Aditional System Init

Jika ada custom motor initialization di `src/Aditional/`, integrate ke `src/drivers/motor_bts7960.c`:

### Example: Pindahkan GPIO Init

**From stm32f4xx_hal_msp.c** (original):
```c
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        // ... etc
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    // etc
}
```

**Integrate to motor_bts7960.c**:
```c
int motor_init(void)
{
    // ... existing code ...
    
    // Add GPIO init for motor enable pins if not in HAL_MSP
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MOTOR_RIGHT_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MOTOR_RIGHT_EN_PORT, &GPIO_InitStruct);
    // etc
}
```

## Step 6: Build Test

Compile untuk verifikasi tidak ada error:

```bash
cd "c:\Users\ASUS\Documents\PlatformIO\Projects\Fira STM32"
platformio run
```

### Common Errors & Solutions

| Error | Solusi |
|-------|--------|
| `undefined reference to main` | Pastikan `src/main.c` ada dan terdeteksi |
| `include not found` | Verifikasi build_flags di platformio.ini |
| `multiple definition` | Ada duplikat file, hapus dari Aditional/ |
| `SPI not initialized` | Pastikan `MX_SPI1_Init()` dipanggil di main |
| `Timer not running` | Pastikan `MX_TIMx_Init()` dipanggil untuk semua timer |

## Step 7: GPIO Pin Configuration

Update pin definitions di header files sesuai PCB Anda:

### Motor Driver Pins
Edit `include/drivers/motor_bts7960.h`:
```c
#define MOTOR_RIGHT_EN_PORT      GPIOA      // ← Sesuaikan dengan board
#define MOTOR_RIGHT_EN_PIN       GPIO_PIN_0 // ← Sesuaikan dengan board
#define MOTOR_LEFT_EN_PORT       GPIOA
#define MOTOR_LEFT_EN_PIN        GPIO_PIN_1
```

### Ultrasonic Pins
Edit `include/drivers/sensor_ultrasonic.h`:
```c
#define ULTRASONIC_TRIG_PORT    GPIOB      // ← Sesuaikan
#define ULTRASONIC_TRIG_PIN     GPIO_PIN_2
#define ULTRASONIC_ECHO_PORT    GPIOB
#define ULTRASONIC_ECHO_PIN     GPIO_PIN_3
```

### Servo Pins
Servo sudah menggunakan Timer PWM, confirm di `include/drivers/servo.h`:
```c
#define SERVO_PWM_TIMER      &htim4        // TIM4 CH1 (sudah di main.c)
#define SERVO_PWM_CHANNEL    TIM_CHANNEL_1
```

### IMU Pins
Edit `include/drivers/imu_mpu6500.h`:
```c
#define MPU_NCS_GPIO_PORT  GPIOB           // ← SPI chip select
#define MPU_NCS_PIN        GPIO_PIN_1
#define MPU_INT_PORT       GPIOB           // ← Data ready interrupt
#define MPU_INT_PIN        GPIO_PIN_0
```

## Step 8: Timer Configuration

Verifikasi timer allocation di `src/main.c` sudah sesuai dengan fungsionalitas:

| Timer | Used By | Status |
|-------|---------|--------|
| TIM1 | System tick / Extra | ✓ Sudah di main |
| TIM2 | Motor PWM (Right) | ✓ Sudah di main |
| TIM3 | Motor PWM (Left) | ✓ Sudah di main |
| TIM4 | Servo PWM | ✓ Sudah di main |
| TIM5 | Reserved | Belum dipakai |
| TIM9 | Ultrasonic timing | Opsional |

Jika ada perubahan timer, update di:
- `include/drivers/motor_bts7960.h` - MOTOR_*_PWM_TIMER
- `include/drivers/servo.h` - SERVO_PWM_TIMER
- `include/drivers/sensor_ultrasonic.h` - ULTRASONIC_TIM

## Step 9: First Build & Upload

```bash
# Build
platformio run

# Upload ke board
platformio run --target upload

# Monitor USB output
platformio device monitor
```

## Step 10: Testing

Setelah upload, test masing-masing komponen:

### 1. Test IMU
Di USB terminal, monitor IMU data dari `main.c` output

### 2. Test Motor
```c
// Di main.c while loop, di section USER CODE:
motor_init();
motor_set_speed(MOTOR_RIGHT, 50);  // 50% forward
```

### 3. Test Servo
```c
servo_init();
servo_set_angle(45);  // Turn right
```

### 4. Test Ultrasonic
```c
ultrasonic_init();
uint32_t distance;
ultrasonic_read_blocking(&distance);
printf("Distance: %lu cm\n", distance);
```

## Step 11: Integrate Control Loop

Uncomment di main.c setelah semua komponen tested:

```c
// Di main(), sebelum while loop:
control_init();

// Di while loop:
if (HAL_GetTick() - last_tick >= 20) {  // 50Hz
    last_tick = HAL_GetTick();
    control_loop_update();
}
```

## Troubleshooting

### Build Fails
- Check all #include paths
- Verify no circular dependencies
- Clean build: `platformio run --target clean && platformio run`

### Motor Not Running
- Check PWM timer frequency
- Verify GPIO enable pins HIGH
- Check BTS7960 module connections

### Servo Not Moving
- Check servo pulse width range (1000-2000 us)
- Verify TIM4 frequency is 50 Hz
- Check servo power supply

### IMU Not Reading
- Verify SPI clock speed (~1-10 MHz)
- Check WHO_AM_I register returns 0x70
- Verify CS (chip select) pin toggling

### Ultrasonic Not Detecting
- Check trigger pin pulsing 10us
- Verify echo pin input capture configured
- Test with simple blocking read first

## Next: Advanced Configuration

Setelah struktur stabil, lanjutkan dengan:

1. **Autonomous Logic**: Implement navigation algorithm di `control_autonomous_mode()`
2. **Telemetry**: Send real-time data via USB
3. **Remote Control**: Parse commands from PC
4. **Sensor Fusion**: Combine IMU + ultrasonic for better decisions
5. **Advanced PID**: Tune untuk specific behavioral performance
