/**
 * @file jetson_protocol.md
 * @brief Protokol Komunikasi STM32 ↔ Jetson
 */

# Jetson Communication Protocol

## Overview

STM32 ↔ Jetson komunikasi via USB CDC (virtual COM port) dengan protokol text-based yang mudah di-debug.

```
Jetson (Command) → STM32 (Execute) → Jetson (Receive Telemetry)
```

---

## Frame Format

### Command Frame (Jetson → STM32)

**Format**: `$CMD,<type>,<param1>,<param2>,<timestamp>\n`

**Example**:
```
$CMD,16,75,0,1234        # Set motor speed to 75%
$CMD,17,45,0,1235        # Set servo angle to 45°
$CMD,18,1,0,1236         # Set mode to AUTONOMOUS (1)
$CMD,19,0,0,1237         # Emergency stop
```

**Parameters**:
- `type`: Command type (0x10-0x15)
- `param1`: Primary parameter
- `param2`: Secondary parameter (or 0)
- `timestamp`: Jetson timestamp (for latency measurement)

### Telemetry Frame (STM32 → Jetson)

**Format**: `@TELEM,<motor>,<servo>,<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<dist>,<collision>,<mode>,<time>\n`

**Example**:
```
@TELEM,50,30,-50,100,-900,10,5,-20,25,0,1,1234
```

**Parameters** (in order):
- `motor`: Motor speed (-100 to +100, %)
- `servo`: Servo angle (-90 to +90, degrees)
- `ax, ay, az`: Acceleration (X, Y, Z in mg)
- `gx, gy, gz`: Angular velocity (X, Y, Z in dps)
- `dist`: Distance from ultrasonic (cm)
- `collision`: Collision flag (0 or 1)
- `mode`: Control mode (0=manual, 1=autonomous, 2=failsafe)
- `time`: STM32 timestamp (ms)

### ACK Frame (STM32 → Jetson)

**Format**: `@ACK,<command_type>\n`

**Example**:
```
@ACK,16              # Acknowledged motor speed command
@ACK,19              # Acknowledged emergency stop
```

### Error Frame (STM32 → Jetson)

**Format**: `@ERROR,<code>,<message>\n`

**Example**:
```
@ERROR,1,Unknown command
@ERROR,2,Invalid format
@ERROR,3,Timeout
```

---

## Command Types

| Type | Code | Purpose | Param1 | Param2 |
|------|------|---------|--------|--------|
| SET_MOTOR_SPEED | 0x10 | Set motor speed | speed (-100 to +100) | N/A |
| SET_SERVO_ANGLE | 0x11 | Set servo angle | angle (-90 to +90) | N/A |
| SET_MODE | 0x12 | Set control mode | 0=manual, 1=auto, 2=failsafe | N/A |
| EMERGENCY_STOP | 0x13 | E-stop | N/A | N/A |
| GET_STATUS | 0x14 | Request status | N/A | N/A |
| RESET | 0x15 | System reset | N/A | N/A |

---

## Example Communication Sequence

### Scenario 1: Manual Motor Control

```
Jetson sends:    $CMD,16,75,0,1001      (Start moving forward 75%)
STM32 responds:  @ACK,16
STM32 sends:     @TELEM,75,0,...,1002   (Telemetry with motor=75%)

Jetson sends:    $CMD,16,-50,0,1003     (Start reversing 50%)
STM32 responds:  @ACK,16
STM32 sends:     @TELEM,-50,0,...,1004  (Motor=-50%)

Jetson sends:    $CMD,19,0,0,1005       (Emergency stop)
STM32 responds:  @ACK,19
STM32 sends:     @TELEM,0,0,...,1006    (Motor=0, all stopped)
```

### Scenario 2: Autonomous Mode

```
Jetson sends:    $CMD,18,1,0,2001       (Enable autonomous mode)
STM32 responds:  @ACK,18
STM32 sends:     @TELEM,50,0,...,2002   (Auto starts moving)
STM32 sends:     @TELEM,50,15,...,2004  (Auto steers left)
STM32 sends:     @TELEM,0,-15,...,2006  (Collision detected, stop)

Jetson sends:    $CMD,19,0,0,2007       (Force emergency stop)
STM32 responds:  @ACK,19
STM32 sends:     @TELEM,0,0,...,2008    (All stopped)
```

---

## Implementation on Jetson

### Python Example

```python
#!/usr/bin/env python3
import serial
import time

class STM32Car:
    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.timestamp = 0
    
    def send_command(self, cmd_type, param1=0, param2=0):
        """Send command to STM32"""
        self.timestamp = int(time.time() * 1000) % 65536
        cmd = f"$CMD,{cmd_type},{param1},{param2},{self.timestamp}\n"
        self.ser.write(cmd.encode())
        print(f"→ {cmd.strip()}")
    
    def read_telemetry(self):
        """Read telemetry from STM32"""
        if self.ser.in_waiting:
            line = self.ser.readline().decode().strip()
            if line.startswith("@TELEM"):
                parts = line.split(',')
                return {
                    'motor': int(parts[1]),
                    'servo': int(parts[2]),
                    'accel': (int(parts[3]), int(parts[4]), int(parts[5])),
                    'gyro': (int(parts[6]), int(parts[7]), int(parts[8])),
                    'distance': int(parts[9]),
                    'collision': int(parts[10]),
                    'mode': int(parts[11])
                }
            print(f"← {line}")
        return None
    
    def set_motor_speed(self, speed):
        """Set motor speed (-100 to +100)"""
        self.send_command(0x10, speed)
    
    def set_servo_angle(self, angle):
        """Set servo angle (-90 to +90)"""
        self.send_command(0x11, angle)
    
    def set_autonomous(self):
        """Enable autonomous mode"""
        self.send_command(0x12, 1)
    
    def emergency_stop(self):
        """Emergency stop"""
        self.send_command(0x13)

# Usage
if __name__ == '__main__':
    car = STM32Car()
    
    # Forward 75%
    car.set_motor_speed(75)
    time.sleep(0.5)
    
    # Turn right 30°
    car.set_servo_angle(30)
    time.sleep(0.5)
    
    # Enable autonomous
    car.set_autonomous()
    
    # Monitor for 10 seconds
    for _ in range(100):
        telem = car.read_telemetry()
        if telem:
            print(f"Motor: {telem['motor']}%, Distance: {telem['distance']}cm")
        time.sleep(0.1)
    
    # Emergency stop
    car.emergency_stop()
```

### C/C++ Example (ROS2)

```cpp
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>
#include <serial/serial.h>

class STM32CarNode : public rclcpp::Node {
public:
    STM32CarNode() : Node("stm32_car_control") {
        // Open serial port
        ser_.setPort("/dev/ttyACM0");
        ser_.setBaudrate(115200);
        ser_.open();
        
        // Subscribe to command topics
        motor_sub_ = this->create_subscription<std_msgs::msg::Int16>(
            "motor_speed", 10,
            std::bind(&STM32CarNode::motor_callback, this, std::placeholders::_1)
        );
        
        // Timer for telemetry reading
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),
            std::bind(&STM32CarNode::read_telemetry, this)
        );
    }

private:
    serial::Serial ser_;
    uint16_t timestamp_ = 0;
    
    void motor_callback(const std_msgs::msg::Int16::SharedPtr msg) {
        if (msg->data >= -100 && msg->data <= 100) {
            send_command(0x10, msg->data);
        }
    }
    
    void send_command(int type, int param) {
        timestamp_ = (uint16_t)(std::chrono::system_clock::now()
            .time_since_epoch().count() / 1000000) % 65536;
        
        char buf[64];
        snprintf(buf, sizeof(buf), "$CMD,%d,%d,0,%u\n", type, param, timestamp_);
        ser_.write(buf);
    }
    
    void read_telemetry() {
        if (ser_.available()) {
            std::string line = ser_.readline();
            // Parse and publish telemetry
        }
    }
};
```

---

## Connection & Serial Settings

| Setting | Value |
|---------|-------|
| Port | /dev/ttyACM0 (Linux/Mac) or COMX (Windows) |
| Baud Rate | 115200 bps |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| Flow Control | None |

**Verify Connection**:
```bash
# Linux
ls -la /dev/ttyACM*

# Mac
ls -la /dev/tty.usbmodem*

# Windows (PowerShell)
Get-PnpDevice -Class Ports
```

---

## Testing from Linux/Mac Terminal

### Monitor Telemetry
```bash
cat /dev/ttyACM0
```

### Send Command
```bash
echo '$CMD,16,75,0,1001' > /dev/ttyACM0
```

### Full Test Script
```bash
#!/bin/bash
PORT=/dev/ttyACM0

# Motor forward
echo '$CMD,16,75,0,1001' > $PORT
sleep 1

# Servo right
echo '$CMD,17,45,0,1002' > $PORT
sleep 1

# E-stop
echo '$CMD,19,0,0,1003' > $PORT
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No telemetry received | Check serial port & baud rate |
| Command not acknowledged | Verify command format |
| Motor doesn't respond | Check `comm_handle_usb_rx()` called |
| Latency high | Reduce COMM_TX_INTERVAL_MS if needed |
| Garbled output | Use 115200 baud exactly |

---

## Performance Notes

- **Telemetry Rate**: 20 Hz (50ms interval) - adjustable
- **Latency**: ~5-10ms per command + processing
- **Throughput**: ~1-2 commands/sec sustained
- **Reliability**: ASCII format, self-synchronizing with $ and @

---

## Future Extensions

- [ ] Binary protocol for higher throughput
- [ ] Checksum validation (CRC)
- [ ] Compression for large telemetry
- [ ] Real-time kernel integration
- [ ] ROS2 bridge node
- [ ] Jetson Nano optimization
