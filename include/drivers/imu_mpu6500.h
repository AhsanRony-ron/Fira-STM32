/* imu_mpu6500.h - MPU6500 driver (SPI) - minimal API, no USB/printf */
#ifndef IMU_MPU6500_H
#define IMU_MPU6500_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* MPU6500 registers (ringkasan) */
#define MPU6500_REG_WHO_AM_I     0x75
#define MPU6500_REG_PWR_MGMT_1   0x6B
#define MPU6500_REG_SMPLRT_DIV   0x19
#define MPU6500_REG_CONFIG       0x1A
#define MPU6500_REG_GYRO_CONFIG  0x1B
#define MPU6500_REG_ACCEL_CONFIG 0x1C
#define MPU6500_REG_ACCEL_XOUT_H 0x3B
#define MPU6500_REG_GYRO_XOUT_H  0x43
#define MPU6500_REG_INT_PIN_CFG  0x37
#define MPU6500_REG_INT_ENABLE   0x38
#define MPU6500_REG_INT_STATUS   0x3A

/* Pin definitions - sesuaikan dengan boardmu jika perlu */
#define MPU_NCS_GPIO_PORT  GPIOB
#define MPU_NCS_PIN        GPIO_PIN_1
#define MPU_INT_PORT       GPIOB
#define MPU_INT_PIN        GPIO_PIN_0

/* API */
int  mpu6500_init(void); /* return who_am_i (>0) or negative on error */
uint8_t mpu6500_whoami(void);
HAL_StatusTypeDef mpu6500_read_accel_gyro(int16_t *accel, int16_t *gyro, int16_t *temp);

/* Convenience: container for last sample */
typedef struct {
    int16_t accel[3];
    int16_t gyro[3];
    int16_t temp_raw;
    uint32_t timestamp_ms;
} imu_sample_t;

#endif /* IMU_MPU6500_H */
