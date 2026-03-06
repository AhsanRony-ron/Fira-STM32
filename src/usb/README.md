# USB Communication README

## Overview

USB CDC (Communications Device Class) provides a virtual COM port for:
- Debug output and logging
- Telemetry transmission to PC/RPi/Jetson
- Remote control commands input

## Files

- **usb_device.c** - USB device initialization (CubeMX-generated)
- **usbd_cdc_if.c** - USB CDC class interface implementation
- **usbd_conf.c** - USB configuration (endpoints, buffers)
- **usbd_desc.c** - USB device descriptors (vendor ID, product ID, serial number)

## Usage

### Sending Data
```c
#include "usbd_cdc_if.h"

// Send data via USB CDC
uint8_t data[] = "Hello World\r\n";
CDC_Transmit_FS(data, sizeof(data));
```

### Hardware Flow

USB Device (STM32) ↔ USB Host (PC/Mac/Linux)
- Virtual COM port appears as COMx or /dev/ttyACMx
- Standard serial communication (115200 baud in config)

### Common Issues

1. **Can't find COM port**
   - Check USB cable connection
   - Verify STM32 USB pins connected properly
   - Install STM32 VCP drivers if needed

2. **Garbled output**
   - Check baud rate (default 115200)
   - Verify USB endpoint configuration in usbd_conf.c

3. **Transmission timeout**
   - Check buffer sizes in usbd_conf.c
   - Ensure receive interrupt is properly configured

## Terminal Tools

### Windows
- PuTTY - Free SSH/serial terminal
- Tera Term - Simple serial monitor
- Arduino IDE - Built-in serial monitor

### Linux/Mac
```bash
# Monitor serial port
minicom -D /dev/ttyACM0 -b 115200

# Or using screen
screen /dev/ttyACM0 115200
```

### Python Example
```python
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

while True:
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        print(f"Received: {line}")
    
    # Send command
    ser.write(b'MOTOR:75,SERVO:45\r\n')
    time.sleep(0.1)
```

## Telemetry Format

Design a protocol for PC ↔ STM32 communication:

### Example Protocol
```
TELEMETRY FORMAT:
@TELEM,<motor>%,<servo>deg,<distance>cm,<accel_x>,<accel_y>,<accel_z>\r\n

COMMAND FORMAT:
@CMD,<motor_pct>,<servo_deg>\r\n

Example:
@TELEM,75%,45deg,30cm,100,50,-1000
@CMD,50,-30
```

## Implementation Guide

### Basic USB TX/RX

```c
// Send telemetry periodically
static void send_telemetry(control_state_t *state) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf),
        "@TELEM,%d,%d,%lu\r\n",
        state->motor_speed,
        state->servo_angle,
        state->distance_front_cm
    );
    CDC_Transmit_FS((uint8_t*)buf, len);
}

// Parse incoming commands
void handle_usb_command(uint8_t *data, uint32_t len) {
    if (strncmp((char*)data, "@CMD,", 5) == 0) {
        int motor, servo;
        if (sscanf((char*)data, "@CMD,%d,%d", &motor, &servo) == 2) {
            control_manual_command(motor, servo);
        }
    }
}
```

## Configuration

### Device Descriptor (usbd_desc.c)
Customize before shipping:
- USBD_MANUFACTURER_STRING: "Your Company"
- USBD_PRODUCT_STRING: "Autonomous Car V1"
- USBD_SERIALNUMBER_STRING: "AC-001"

### Endpoint Configuration (usbd_conf.c)
Usually pre-configured, but if needed:
- CDC_IN_EP: Transmit endpoint
- CDC_OUT_EP: Receive endpoint
- CDC_CMD_EP: Command endpoint
- EP buffer sizes

## Safety & Best Practices

1. **Never block in TX**: Use non-blocking CDC_Transmit_FS
2. **Buffer overflow**: Check data length before storing
3. **Command validation**: Verify ranges before applying
4. **Timeout protection**: Implement watchdog for USB disconnection
5. **Dual-layer**: USB for debug, serial (UART) for mission-critical (optional)

## Debugging USB

### On Windows
Use Device Manager → Ports to verify COM port
Use Zadig to manage USB driver

### On Linux
```bash
# List USB devices
lsusb

# Monitor USB traffic
sudo usbmon -i /dev/bus/usb/001/001 -w log.bin
```

## Power Consumption

USB provides Vbus power:
- **Full-speed**: Up to 500mA
- **High-speed**: Up to 500mA initially, up to 1A after negotiation

Ensure power budget allows for:
- STM32 microcontroller (~50mA)
- Motor drivers (~2-5A): **Usually need external power!**
- Sensors (~100mA total)
