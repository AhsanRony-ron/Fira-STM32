/* imu_mpu6500.c - MPU6500 driver (SPI) */
#include "drivers/imu_mpu6500.h"

/* Use existing SPI handle from main.c (CubeMX-generated). */
extern SPI_HandleTypeDef hspi1;

/* Local helpers to control CS */
static inline void mpu_ncs_low(void)  { HAL_GPIO_WritePin(MPU_NCS_GPIO_PORT, MPU_NCS_PIN, GPIO_PIN_RESET); }
static inline void mpu_ncs_high(void) { HAL_GPIO_WritePin(MPU_NCS_GPIO_PORT, MPU_NCS_PIN, GPIO_PIN_SET); }

/* Low-level read/write over SPI */
static HAL_StatusTypeDef mpu6500_read_regs(uint8_t reg, uint8_t *pBuf, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t tx = reg | 0x80; /* read */
    mpu_ncs_low();
    status = HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) { mpu_ncs_high(); return status; }
    status = HAL_SPI_Receive(&hspi1, pBuf, len, HAL_MAX_DELAY);
    mpu_ncs_high();
    return status;
}

static HAL_StatusTypeDef mpu6500_write_regs(uint8_t reg, uint8_t *pBuf, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t tx = reg & 0x7F; /* write */
    mpu_ncs_low();
    status = HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) { mpu_ncs_high(); return status; }
    status = HAL_SPI_Transmit(&hspi1, pBuf, len, HAL_MAX_DELAY);
    mpu_ncs_high();
    return status;
}

int mpu6500_init(void)
{
    HAL_Delay(50);
    uint8_t tmp;
    /* Reset */
    tmp = 0x80;
    if (mpu6500_write_regs(MPU6500_REG_PWR_MGMT_1, &tmp, 1) != HAL_OK) return -1;
    HAL_Delay(100);

    /* Wake up and set clock */
    tmp = 0x01;
    if (mpu6500_write_regs(MPU6500_REG_PWR_MGMT_1, &tmp, 1) != HAL_OK) return -1;
    HAL_Delay(10);

    tmp = 0x00; /* SMPLRT_DIV */
    if (mpu6500_write_regs(MPU6500_REG_SMPLRT_DIV, &tmp, 1) != HAL_OK) return -1;

    tmp = 0x03; /* CONFIG: DLPF_CFG=3 as example */
    if (mpu6500_write_regs(MPU6500_REG_CONFIG, &tmp, 1) != HAL_OK) return -1;

    tmp = 0x00; /* GYRO_CONFIG ±250dps */
    if (mpu6500_write_regs(MPU6500_REG_GYRO_CONFIG, &tmp, 1) != HAL_OK) return -1;

    tmp = 0x00; /* ACCEL_CONFIG ±2g */
    if (mpu6500_write_regs(MPU6500_REG_ACCEL_CONFIG, &tmp, 1) != HAL_OK) return -1;

    /* Configure INT pin behaviour: INT_ANYRD_2CLEAR */
    tmp = 0x10;
    mpu6500_write_regs(MPU6500_REG_INT_PIN_CFG, &tmp, 1);

    /* Enable DATA_READY interrupt */
    tmp = 0x01;
    mpu6500_write_regs(MPU6500_REG_INT_ENABLE, &tmp, 1);

    /* WHO_AM_I check */
    uint8_t who = 0;
    if (mpu6500_read_regs(MPU6500_REG_WHO_AM_I, &who, 1) != HAL_OK) return -1;
    return (int)who;
}

uint8_t mpu6500_whoami(void)
{
    uint8_t who = 0;
    if (mpu6500_read_regs(MPU6500_REG_WHO_AM_I, &who, 1) != HAL_OK) return 0x00;
    return who;
}

HAL_StatusTypeDef mpu6500_read_accel_gyro(int16_t *accel, int16_t *gyro, int16_t *temp)
{
    uint8_t buf[14];
    HAL_StatusTypeDef st = mpu6500_read_regs(MPU6500_REG_ACCEL_XOUT_H, buf, 14);
    if (st != HAL_OK) return st;

    accel[0] = (int16_t)((buf[0] << 8) | buf[1]);
    accel[1] = (int16_t)((buf[2] << 8) | buf[3]);
    accel[2] = (int16_t)((buf[4] << 8) | buf[5]);

    *temp = (int16_t)((buf[6] << 8) | buf[7]);

    gyro[0] = (int16_t)((buf[8] << 8) | buf[9]);
    gyro[1] = (int16_t)((buf[10] << 8) | buf[11]);
    gyro[2] = (int16_t)((buf[12] << 8) | buf[13]);

    return HAL_OK;
}
