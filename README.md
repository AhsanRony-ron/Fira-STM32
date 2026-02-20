# Fira-STM32

Firmware STM32 untuk platform robot **Xierra (Autonomous Car)**, di-generate menggunakan **STM32CubeMX** dan di-build dengan **PlatformIO**.

---

## Daftar Isi

- [Informasi Board](#informasi-board)
- [Konfigurasi Clock](#konfigurasi-clock)
- [Peripheral yang Dikonfigurasi](#peripheral-yang-dikonfigurasi)
  - [SPI1](#spi1)
  - [TIM1 – PWM Motor DC](#tim1--pwm-motor-dc)
  - [TIM2 – Encoder (32-bit)](#tim2--encoder-32-bit)
  - [TIM3 – Encoder (16-bit)](#tim3--encoder-16-bit)
  - [TIM4 – PWM Motor DC](#tim4--pwm-motor-dc)
  - [TIM5 – PWM Servo](#tim5--pwm-servo)
  - [GPIO Output](#gpio-output)
  - [USB CDC (Virtual COM Port)](#usb-cdc-virtual-com-port)
- [Peta Pin (Pin Mapping)](#peta-pin-pin-mapping)
- [Struktur Proyek](#struktur-proyek)
- [Build & Upload](#build--upload)

---

## Informasi Board

| Parameter     | Nilai                         |
|---------------|-------------------------------|
| MCU           | STM32F411CEU6                 |
| Board         | WeAct BlackPill F411CE        |
| Framework     | STM32Cube (HAL)               |
| Build System  | PlatformIO                    |
| Clock Sumber  | HSE (External Crystal 25 MHz) |
| SYSCLK        | 84 MHz                        |

---

## Konfigurasi Clock

Clock dikonfigurasi menggunakan PLL dari sumber HSE 25 MHz:

```
HSE (25 MHz)
  └─ PLL: PLLM=25, PLLN=336, PLLP=DIV4
       └─ SYSCLK = 84 MHz
            ├─ HCLK  (AHB  / 1)  = 84 MHz
            ├─ PCLK1 (APB1 / 2)  = 42 MHz  → TIM2, TIM3, TIM4, TIM5 timer clock = 84 MHz
            └─ PCLK2 (APB2 / 1)  = 84 MHz  → TIM1, SPI1
```

> **Catatan:** Timer clock APB1 = 2× PCLK1 = 84 MHz karena divider ≠ 1.

---

## Peripheral yang Dikonfigurasi

### SPI1

| Parameter         | Nilai                          |
|-------------------|--------------------------------|
| Mode              | Master                         |
| Direction         | Full-Duplex (2 Lines)          |
| Data Size         | 8-bit                          |
| CPOL / CPHA       | Low / 1 Edge (Mode 0)          |
| NSS               | Software                       |
| Baud Rate         | PCLK2 / 32 = **2.625 Mbps**    |
| Bit Order         | MSB First                      |

**Pin:**

| GPIO | Fungsi    |
|------|-----------|
| PA5  | SPI1_SCK  |
| PA6  | SPI1_MISO |
| PA7  | SPI1_MOSI |

---

### TIM1 – PWM Motor DC

| Parameter    | Nilai                                          |
|--------------|------------------------------------------------|
| Mode         | PWM Generation                                 |
| Prescaler    | 0                                              |
| Period (ARR) | 4199                                           |
| Frekuensi    | 84 MHz / (0+1) / (4199+1) = **20 kHz**         |
| Channel      | CH1, CH2, CH3 (Pulse awal = 0)                 |

**Pin:**

| GPIO | Fungsi    |
|------|-----------|
| PA8  | TIM1_CH1  |
| PA9  | TIM1_CH2  |
| PA10 | TIM1_CH3  |

---

### TIM2 – Encoder (32-bit)

| Parameter    | Nilai                       |
|--------------|-----------------------------|
| Mode         | Encoder TI1 & TI2 (TI12)   |
| Bit Width    | 32-bit (Period = 0xFFFFFFFF)|
| IC1 Filter   | 5                           |
| IC2 Filter   | 0                           |

**Pin:**

| GPIO | Fungsi    |
|------|-----------|
| PA15 | TIM2_CH1  |
| PB3  | TIM2_CH2  |

---

### TIM3 – Encoder (16-bit)

| Parameter    | Nilai                    |
|--------------|--------------------------|
| Mode         | Encoder TI1 & TI2 (TI12)|
| Bit Width    | 16-bit (Period = 65535)  |
| IC1 Filter   | 5                        |
| IC2 Filter   | 5                        |

**Pin:**

| GPIO | Fungsi    |
|------|-----------|
| PB4  | TIM3_CH1  |
| PB5  | TIM3_CH2  |

---

### TIM4 – PWM Motor DC

| Parameter    | Nilai                                   |
|--------------|-----------------------------------------|
| Mode         | PWM Generation                          |
| Prescaler    | 0                                       |
| Period (ARR) | 4199                                    |
| Frekuensi    | 84 MHz / (0+1) / (4199+1) = **20 kHz** |
| Channel      | CH1 (Pulse awal = 0)                    |

**Pin:**

| GPIO | Fungsi    |
|------|-----------|
| PB6  | TIM4_CH1  |

---

### TIM5 – PWM Servo

| Parameter    | Nilai                                          |
|--------------|------------------------------------------------|
| Mode         | PWM Generation                                 |
| Prescaler    | 83                                             |
| Period (ARR) | 19999                                          |
| Frekuensi    | 84 MHz / (83+1) / (19999+1) = **50 Hz**        |
| Channel      | CH1, CH2, CH3 (Pulse = **1500** → 1.5 ms, posisi netral servo) |
|              | CH4 (Pulse = 0)                                |

> Periode 50 Hz (20 ms) adalah standar untuk sinyal kendali servo RC.  
> Pulse antara **1000 µs – 2000 µs** mengontrol posisi sudut servo.

**Pin:**

| GPIO      | Fungsi    |
|-----------|-----------|
| PA0 (WKUP)| TIM5_CH1  |
| PA1       | TIM5_CH2  |
| PA2       | TIM5_CH3  |
| PA3       | TIM5_CH4  |

---

### GPIO Output

| GPIO | Mode          | Keterangan         |
|------|---------------|--------------------|
| PB0  | Output Push-Pull | General purpose output |
| PB1  | Output Push-Pull | General purpose output |

---

### USB CDC (Virtual COM Port)

USB Device dikonfigurasi sebagai **CDC (Communication Device Class)** yang mengemulasi port serial (Virtual COM Port / VCP). Saat ini inisialisasi USB (`MX_USB_DEVICE_Init()`) dikomentari di `main.c` dan dapat diaktifkan sesuai kebutuhan.

Buffer yang tersedia:
- `UserRxBufferFS[]` – buffer penerimaan data dari host
- `UserTxBufferFS[]` – buffer pengiriman data ke host

---

## Peta Pin (Pin Mapping)

| GPIO      | Peripheral   | Fungsi                     |
|-----------|--------------|----------------------------|
| PA0 (WKUP)| TIM5_CH1     | PWM Servo 1                |
| PA1       | TIM5_CH2     | PWM Servo 2                |
| PA2       | TIM5_CH3     | PWM Servo 3                |
| PA3       | TIM5_CH4     | PWM Servo 4                |
| PA5       | SPI1_SCK     | SPI Clock                  |
| PA6       | SPI1_MISO    | SPI MISO                   |
| PA7       | SPI1_MOSI    | SPI MOSI                   |
| PA8       | TIM1_CH1     | PWM Motor DC 1             |
| PA9       | TIM1_CH2     | PWM Motor DC 2             |
| PA10      | TIM1_CH3     | PWM Motor DC 3             |
| PA11      | USB_DM       | USB Data-                  |
| PA12      | USB_DP       | USB Data+                  |
| PA15      | TIM2_CH1     | Encoder 1 – Phase A        |
| PB0       | GPIO_Output  | Output digital             |
| PB1       | GPIO_Output  | Output digital             |
| PB3       | TIM2_CH2     | Encoder 1 – Phase B        |
| PB4       | TIM3_CH1     | Encoder 2 – Phase A        |
| PB5       | TIM3_CH2     | Encoder 2 – Phase B        |
| PB6       | TIM4_CH1     | PWM Motor DC 4             |

---

## Struktur Proyek

```
Fira-STM32/
├── platformio.ini              # Konfigurasi PlatformIO (board, platform, framework)
├── include/
│   ├── main.h                  # Deklarasi fungsi dan define aplikasi
│   ├── stm32f4xx_hal_conf.h    # Konfigurasi modul HAL yang diaktifkan
│   ├── stm32f4xx_it.h          # Deklarasi interrupt handler
│   ├── usb_device.h            # Header inisialisasi USB Device
│   ├── usbd_cdc_if.h           # Header antarmuka CDC (Virtual COM Port)
│   ├── usbd_conf.h             # Konfigurasi low-level USB Device
│   └── usbd_desc.h             # Deskriptor USB Device
├── src/
│   ├── main.c                  # Entry point, inisialisasi peripheral, infinite loop
│   ├── stm32f4xx_hal_msp.c     # MSP Init: konfigurasi GPIO & clock tiap peripheral
│   ├── stm32f4xx_it.c          # Implementasi interrupt handler (SysTick, dll.)
│   ├── system_stm32f4xx.c      # Konfigurasi sistem clock awal (CMSIS)
│   ├── usb_device.c            # Inisialisasi USB Device stack
│   ├── usbd_cdc_if.c           # Implementasi callback CDC (RX/TX)
│   ├── usbd_conf.c             # Low-level USB HAL (PCD callbacks)
│   └── usbd_desc.c             # Deskriptor string & device USB
└── USB_DEVICE/                 # Salinan file USB dari CubeMX (referensi)
    └── App/
        ├── usb_device.c / .h
        ├── usbd_cdc_if.c / .h
        ├── usbd_desc.c / .h
```

### Modul HAL yang Diaktifkan

Dari `stm32f4xx_hal_conf.h`, modul yang aktif:

| Modul               | Keterangan                     |
|---------------------|--------------------------------|
| `HAL_MODULE_ENABLED`| Core HAL (GPIO, RCC, Flash...) |
| `HAL_SPI_MODULE_ENABLED` | SPI driver                |
| `HAL_TIM_MODULE_ENABLED` | Timer driver              |
| `HAL_PCD_MODULE_ENABLED` | USB Peripheral (Device)   |

---

## Build & Upload

### Prasyarat

- [PlatformIO IDE](https://platformio.org/) (ekstensi VS Code) atau PlatformIO CLI
- Driver USB STM32 (DFU atau ST-Link)

### Perintah

```bash
# Build proyek
pio run

# Upload ke board (pastikan board terhubung)
pio run --target upload

# Monitor serial (jika USB CDC aktif)
pio device monitor --baud 115200
```

### Konfigurasi PlatformIO (`platformio.ini`)

```ini
[env:blackpill_f411ce]
platform = ststm32
board    = blackpill_f411ce
framework = stm32cube
```

---

## Menambahkan Kode Pengguna

STM32CubeMX menandai area yang aman untuk kode pengguna dengan pasangan komentar:

```c
/* USER CODE BEGIN <tag> */
// Tambahkan kode di sini
/* USER CODE END <tag> */
```

Kode di luar area ini akan **tertimpa** jika file di-regenerasi menggunakan CubeMX. Selalu tulis logika aplikasi di dalam blok `USER CODE`.
