# Execution Checklist - Ready to Implement

## ✅ Pre-Integration Verification

Pastikan project sudah dalam state siap:

- [ ] Semua folder di bawah sudah ada:
  ```
  src/drivers/, src/control/, src/system/, src/usb/
  include/drivers/, include/control/, include/system/, include/usb/
  ```

- [ ] Semua file baru sudah ada (22 files):
  ```
  ✓ Driver .c/.h files (8)
  ✓ Control files (2)
  ✓ README files (8)
  ✓ Documentation files (4)
  ```

- [ ] platformio.ini sudah updated dengan build_flags

- [ ] main.c sudah updated dengan include paths

---

## 🔄 Integration Steps (Dalam Urutan)

### Step 1: Copy Files from Aditional/ (5 min)

**Using PowerShell:**
```powershell
$srcDir = "c:\Users\ASUS\Documents\PlatformIO\Projects\Fira STM32"
cd $srcDir

# Copy system files
Copy-Item "src/Aditional/system_stm32f4xx.c" "src/system/system_stm32f4xx.c" -Force
Copy-Item "src/Aditional/stm32f4xx_it.c" "src/system/stm32f4xx_it.c" -Force
Copy-Item "src/Aditional/stm32f4xx_hal_msp.c" "src/system/stm32f4xx_hal_msp.c" -Force

# Copy USB files
Copy-Item "src/Aditional/usb_device.c" "src/usb/usb_device.c" -Force
Copy-Item "src/Aditional/usbd_cdc_if.c" "src/usb/usbd_cdc_if.c" -Force
Copy-Item "src/Aditional/usbd_conf.c" "src/usb/usbd_conf.c" -Force
Copy-Item "src/Aditional/usbd_desc.c" "src/usb/usbd_desc.c" -Force

Write-Host "✓ All 8 files copied successfully"
```

**Verify Result:**
- [ ] `src/system/system_stm32f4xx.c` exists
- [ ] `src/system/stm32f4xx_it.c` exists
- [ ] `src/system/stm32f4xx_hal_msp.c` exists
- [ ] `src/usb/usb_device.c` exists
- [ ] `src/usb/usbd_cdc_if.c` exists
- [ ] `src/usb/usbd_conf.c` exists
- [ ] `src/usb/usbd_desc.c` exists

---

### Step 2: Verify Hardware Pin Definitions (10 min)

Edit file: `include/drivers/motor_bts7960.h` (lines 18-28)
```c
#define MOTOR_RIGHT_EN_PORT    GPIOA          // ← YOUR PORT
#define MOTOR_RIGHT_EN_PIN     GPIO_PIN_0     // ← YOUR PIN
#define MOTOR_LEFT_EN_PORT     GPIOA          // ← YOUR PORT
#define MOTOR_LEFT_EN_PIN      GPIO_PIN_1     // ← YOUR PIN
```
- [ ] Updated motor enable pins

Edit file: `include/drivers/sensor_ultrasonic.h` (lines 10-14)
```c
#define ULTRASONIC_TRIG_PORT   GPIOB          // ← YOUR PORT
#define ULTRASONIC_TRIG_PIN    GPIO_PIN_2     // ← YOUR PIN
#define ULTRASONIC_ECHO_PORT   GPIOB          // ← YOUR PORT
#define ULTRASONIC_ECHO_PIN    GPIO_PIN_3     // ← YOUR PIN
```
- [ ] Updated ultrasonic pins

Edit file: `include/drivers/imu_mpu6500.h` (lines 21-26)
```c
#define MPU_NCS_GPIO_PORT      GPIOB          // ← SPI CS PORT
#define MPU_NCS_PIN            GPIO_PIN_1     // ← SPI CS PIN
#define MPU_INT_PORT           GPIOB          // ← INT PORT
#define MPU_INT_PIN            GPIO_PIN_0     // ← INT PIN
```
- [ ] Updated IMU pins

Servo pins: Already in TIM4 (configured di main.c - no change needed)
- [ ] Servo TIM4 confirmed

---

### Step 3: Build Test (5 min)

**Terminal:**
```bash
cd "c:\Users\ASUS\Documents\PlatformIO\Projects\Fira STM32"
platformio run
```

**Expected Output:**
```
[SUCCESS] Environment blackpill_f401cc: Build finished successfully
```

- [ ] Build succeeds without errors
- [ ] No "undefined reference" errors
- [ ] No "include not found" errors
- [ ] File size reasonable (~50-100KB)

**If Build Fails:**
1. Check all 8 files copied
2. Verify GPIO pins defined correctly
3. Clean build: `platformio run --target clean && platformio run`
4. Check STRUCTURE.md troubleshooting section

---

### Step 4: Upload to Board (3 min)

**Connect USB cable to STM32**

**Terminal:**
```bash
platformio run --target upload
```

**Expected Output:**
```
[SUCCESS] Environment blackpill_f401cc uploaded successfully
```

- [ ] Upload completes successfully
- [ ] LED blinks (if configured)
- [ ] No USB errors

**If Upload Fails:**
1. Check USB cable connection
2. Verify board detected: `platformio device list`
3. Try manual reset (press reset button on board)
4. Reinstall drivers if needed

---

### Step 5: Serial Monitor Test (2 min)

**Terminal:**
```bash
platformio device monitor
```

Expected: Virtual COM port opens, no errors

- [ ] Serial port connects
- [ ] 115200 baud visible
- [ ] Press Ctrl+C to exit

---

## 🧪 Component Testing (30 min)

### Test 1: IMU Sensor

**In main.c while loop, temp code:**
```c
if (mpu_data_ready) {
    printf("IMU Data Ready!\n");
    mpu_data_ready = 0;
}
```

**Expected Result:**
- [ ] Terminal shows periodic "IMU Data Ready!"
- Every 200ms (from main.c fallback read)

---

### Test 2: Motor Control

**In main.c after MX_USB_DEVICE_Init():**
```c
motor_init();
HAL_Delay(500);
motor_set_speed(MOTOR_RIGHT, 50);   // 50% forward
HAL_Delay(2000);
motor_set_speed(MOTOR_RIGHT, 0);    // Stop
```

**Expected Result:**
- [ ] Motor Right spins forward at ~50% for 2 seconds
- [ ] Motor stops cleanly
- [ ] No strange noises

---

### Test 3: Servo Control

**Add to code:**
```c
servo_init();
HAL_Delay(100);
servo_set_angle(-90);  // Full left
HAL_Delay(1000);
servo_center();        // Center
HAL_Delay(1000);
servo_set_angle(90);   // Full right
```

**Expected Result:**
- [ ] Servo moves left
- [ ] Returns to center
- [ ] Moves right
- [ ] Smooth motion

---

### Test 4: Ultrasonic Sensor

**Add to code:**
```c
ultrasonic_init();
uint32_t distance;
for (int i = 0; i < 5; i++) {
    if (ultrasonic_read_blocking(&distance) == 0) {
        printf("Distance: %lu cm\n", distance);
    }
    HAL_Delay(100);
}
```

**Expected Result:**
- [ ] Reads distance values
- [ ] Values between 2-400 cm
- [ ] No error messages

---

## 🎯 Final Verification

### Code Quality
- [ ] No compiler warnings
- [ ] No undefined symbols
- [ ] All includes resolve correctly

### Hardware
- [ ] Motors respond to commands
- [ ] Servo moves smoothly
- [ ] IMU sends data
- [ ] Ultrasonic detects objects

### Communication
- [ ] USB CDC connection works
- [ ] Serial monitor shows data
- [ ] Baud rate 115200 correct

### Safety
- [ ] Motors can be stopped (emergency_stop works)
- [ ] Servo centers properly
- [ ] Failsafe mode stops all

---

## 📋 Documentation Review

Read sequentially:

- [ ] README.md - 5 min (overview)
- [ ] QUICKSTART.md - 5 min (quick ref)
- [ ] SETUP_GUIDE.md - 10 min (details)
- [ ] src/drivers/README.md - 10 min (API)
- [ ] src/control/README.md - 10 min (logic)
- [ ] STRUCTURE.md - 5 min (reference)

**Total Reading Time**: ~45 min

---

## ✅ Sign-Off Checklist

**Before Testing Hardware:**
- [ ] All files copied from Aditional/
- [ ] GPIO pins verified against hardware
- [ ] Build completes successfully
- [ ] Upload completes successfully
- [ ] Serial monitor connects

**Before Operating Motors:**
- [ ] Motors disconnected from power initially
- [ ] Test with USB power only (no motor power)
- [ ] Servo centered before power on
- [ ] Ultrasonic bracket secured

**During Testing:**
- [ ] Work area is clear
- [ ] Motors can't hit anything
- [ ] Failsafe button accessible
- [ ] Emergency stop (e-stop) ready
- [ ] No loose wires

**After Testing:**
- [ ] All components still working
- [ ] No smoke or burning smells
- [ ] No unusual vibrations
- [ ] Serial data consistent

---

## 🚀 Go/No-Go Decision

### ✅ GO if:
- ✓ All build/upload successful
- ✓ All 4 components respond correctly
- ✓ Serial communication works
- ✓ No error messages
- ✓ Hardware intact

### ❌ NO-GO if:
- ✗ Build errors remain
- ✗ Upload fails repeatedly
- ✗ Motor doesn't spin
- ✗ Servo doesn't move
- ✗ IMU no data
- ✗ Ultrasonic no reading

**If NO-GO**: Check STRUCTURE.md troubleshooting, redo pin verification

---

## 📞 Troubleshooting Quick Reference

| Issue | Fix | Time |
|-------|-----|------|
| Build error - include not found | Check platformio.ini build_flags | 2 min |
| Build error - undefined reference | Verify all 8 files copied | 3 min |
| Motor not moving | Check enable pin GPIO | 5 min |
| Servo not responding | Verify TIM4 timer | 5 min |
| IMU no data | Check SPI speed 1-10 MHz | 5 min |
| Ultrasonic no reading | Check trigger pin pulsing | 5 min |
| Serial garbage | Check baud rate 115200 | 2 min |

---

## 📝 Status Updates

```
□ Step 1: Copy files          → Estimated: 5 min
□ Step 2: Verify pins         → Estimated: 10 min
□ Step 3: Build test          → Estimated: 5 min
□ Step 4: Upload              → Estimated: 3 min
□ Step 5: Serial monitor      → Estimated: 2 min
□ Testing: Components         → Estimated: 30 min
                             ─────────────────
                            TOTAL: 55 minutes
```

---

## ✨ Success Criteria

Project is **READY** when:

✓ Build finishes with no errors  
✓ Upload successful to board  
✓ Serial monitor shows output  
✓ Each component responds correctly  
✓ Motor can be commanded  
✓ Servo can be controlled  
✓ Ultrasonic returns distance  
✓ IMU provides data  
✓ Failsafe works  

---

**Ready to start? Begin with Step 1! 🚀**

Questions? Check the README.md or SETUP_GUIDE.md files.
