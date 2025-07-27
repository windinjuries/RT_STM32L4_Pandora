#include "st_i2c.h"
#include "stm32l475xx.h"
#include "stm32l4xx_hal_i2c.h"



#define ICM_ADDR        (0x68 << 1)  // 左移1位用于HAL库
#define WHO_AM_I        0x75
#define PWR_MGMT_1      0x6B
#define SMPLRT_DIV      0x19
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H    0x3B

static I2C_TypeDef *i2c = I2C2;
void ICM20608_Init()
{
    uint8_t data[20] = {0};

    st_i2c_init(i2c, ICM_ADDR);
    // 唤醒设备（清除睡眠位）
    data = 0x00;
    st_i2c_transmit(i2c, ICM_ADDR, PWR_MGMT_1, 1, &data, 1, HAL_MAX_DELAY);

    // 设置采样率分频器
    data = 0x07; // 1kHz / (7 + 1) = 125Hz
    HAL_I2C_Mem_Write(i2c, ICM_ADDR, SMPLRT_DIV, 1, &data, 1, HAL_MAX_DELAY);

    // 配置陀螺仪 ±2000dps
    data = 0x18;
    HAL_I2C_Mem_Write(i2c, ICM_ADDR, GYRO_CONFIG, 1, &data, 1, HAL_MAX_DELAY);

    // 配置加速度 ±16g
    data = 0x18;
    HAL_I2C_Mem_Write(i2c, ICM_ADDR, ACCEL_CONFIG, 1, &data, 1, HAL_MAX_DELAY);
}

void ICM20608_ReadAll(I2C_HandleTypeDef *hi2c, int16_t *acc, int16_t *gyro, float *temp)
{
    uint8_t buffer[14];
    HAL_I2C_Mem_Read(i2c, ICM_ADDR, ACCEL_XOUT_H, 1, buffer, 14, HAL_MAX_DELAY);

    acc[0] = (int16_t)(buffer[0] << 8 | buffer[1]);
    acc[1] = (int16_t)(buffer[2] << 8 | buffer[3]);
    acc[2] = (int16_t)(buffer[4] << 8 | buffer[5]);

    *temp = ((int16_t)(buffer[6] << 8 | buffer[7]) / 326.8f) + 25.0f;

    gyro[0] = (int16_t)(buffer[8] << 8 | buffer[9]);
    gyro[1] = (int16_t)(buffer[10] << 8 | buffer[11]);
    gyro[2] = (int16_t)(buffer[12] << 8 | buffer[13]);
}