/**
 * @file COMMUNICATION_INTEGRATION.md
 * @brief Panduan integrasi komunikasi ke usbd_cdc_if.c
 */

# Integrasi Komunikasi USB CDC

## Overview

File `communication.c` sudah siap untuk menerima USB data, tapi perlu "hook" ke `usbd_cdc_if.c` untuk passing data yang diterima.

## Langkah Integrasi

### Step 1: Update usbd_cdc_if.h

Add forward declaration di header file `include/usbd_cdc_if.h`:

```c
/* At top, after includes */
#include "communication.h"

/* Add this declaration */
void comm_handle_usb_rx(uint8_t *data, uint16_t len);
```

### Step 2: Update usbd_cdc_if.c Receive Callback

Find function `void CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)` di `src/usb/usbd_cdc_if.c`

**Sebelum:**
```c
void CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  /* USER CODE END 6 */
  return;
}
```

**Sesudah:**
```c
void CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  
  /* Pass received data to communication system */
  if (Len > 0) {
    comm_handle_usb_rx(Buf, *Len);
  }
  
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  
  /* USER CODE END 6 */
  return;
}
```

### Step 3: (Optional) Add Debug Output

Jika ingin debug log, add di communication.c:

```c
/* In comm_handle_usb_rx function, after successful parse: */
printf("CMD received: type=%d, param1=%d, param2=%d\r\n", 
       cmd.type, cmd.param1, cmd.param2);
```

## Testing Integration

### Test 1: Verify USB Connection
```bash
platformio run --target upload
platformio device monitor
```

Expected: Serial connection opens at 115200 baud

### Test 2: Send Command via Terminal

**Linux/Mac**:
```bash
echo '$CMD,16,75,0,1001' > /dev/ttyACM0
```

**Windows (PowerShell)**:
```powershell
$port = New-Object System.IO.Ports.SerialPort("COM4", 115200)
$port.Open()
$port.WriteLine('$CMD,16,75,0,1001')
$port.Close()
```

### Test 3: Monitor Telemetry Output

Expected output dari STM32:
```
@TELEM,75,0,-50,100,-900,10,5,-20,25,0,0,1234
@ACK,16
```

## Troubleshooting

| Issue | Penyebab | Solusi |
|-------|----------|--------|
| Telemetry tidak dikirim | comm_update() tidak dipanggil | Check main.c loop - pastikan comm_update() ada |
| Command tidak diterima | comm_handle_usb_rx() tidak called | Update CDC_Receive_FS() di usbd_cdc_if.c |
| Garbage di output | Baud rate tidak 115200 | Check platformio.ini atau device setting |
| ACK dikirim tapi motor tidak gerak | Mode bukan MANUAL | Verifikasi control_set_mode() dipanggil |

## Architecture After Integration

```
Jetson ←→ USB ←→ STM32
           ↓
    usbd_cdc_if.c (CDC_Receive_FS)
           ↓
    comm_handle_usb_rx()
           ↓
    communication.c (parse & handler)
           ↓
    control_loop.c (execute)
           ↓
    drivers (motor, servo, etc)
```

## Message Flow Example

```
Jetson sends: $CMD,16,75,0,1001

STM32 USB ISR:
  → usbd_cdc_if.c::CDC_Receive_FS() called
  → comm_handle_usb_rx(data, len) called
  → Parse: type=16, param1=75
  → Switch to MANUAL mode
  → motor_set_speed(MOTOR_RIGHT, 75)
  → send_ack_frame(16) → @ACK,16

Main loop (comm_update):
  → Collect telemetry
  → send_telemetry_frame()
  → @TELEM,75,0,...

Jetson receives: @ACK,16 + @TELEM data
```

## Performance Notes

- **Command latency**: ~5-10ms (ISR → main loop)
- **Telemetry rate**: 20 Hz (50ms between packets)
- **USB throughput**: ~1 MB/s available (only using ~1 KB/s)

## Next Steps

1. Update `src/usb/usbd_cdc_if.c` dengan perubahan di atas
2. Rebuild: `platformio run` 
3. Test komunikasi dari Jetson (lihat JETSON_PROTOCOL.md)
4. Monitor telemetry di Jetson

---

**File Reference**:
- Communication API: `include/communication.h`
- Protocol Spec: `JETSON_PROTOCOL.md`
- Control Loop: `src/control/control_loop.c`
