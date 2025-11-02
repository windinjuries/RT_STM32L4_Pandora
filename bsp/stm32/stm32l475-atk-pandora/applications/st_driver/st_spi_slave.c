#include <stdio.h>
#include <stdint.h>

#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_spi.h"

SPI_HandleTypeDef hspi2;

/* Modbus相关变量 */
#define SLAVE_ADDRESS 0x01
#define MAX_FRAME_LEN 256
uint8_t rxBuffer[MAX_FRAME_LEN];
uint8_t txBuffer[MAX_FRAME_LEN];
volatile uint16_t rxIndex = 0;
volatile uint8_t rxComplete = 0;
volatile uint8_t txInProgress = 0;

uint8_t fast_table[128];
uint8_t slow_table[128];

typedef enum
{
    SPI_CMD_FAST_READ = 0,
    SPI_CMD_SLOW_READ,
} SPI_CMD;

void ProcessModbusFrame(uint8_t *frame, uint16_t length);

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     if (GPIO_Pin == GPIO_PIN_4) 
//     { 
//         // CS引脚
//         if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) 
//         {
//             // CS下降沿：开始接收数据
//             rxIndex = 0;
//             HAL_SPI_Receive_IT(&hspi1, &rxBuffer[rxIndex], 1);
//         } 
//         else 
//         {
//             // CS上升沿：接收完成，处理数据
//             rxComplete = 1;
//             HAL_SPI_Abort(&hspi1); // 停止SPI接收
//         }
//     }
// }

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2) 
    {
        if(rxBuffer[0] == SPI_CMD_FAST_READ)
        {
            HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer + 1, 128); 
            rxIndex = 0;
        }
    }
}

int st_spi_slave_init(void)
{
    hspi2.Instance            = SPI2;
    hspi2.Init.Mode           = SPI_MODE_SLAVE;
    hspi2.Init.Direction      = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize       = SPI_DATASIZE_4BIT;
    hspi2.Init.CLKPolarity    = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase       = SPI_PHASE_1EDGE;
    hspi2.Init.NSS            = SPI_NSS_SOFT;
    hspi2.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode         = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial  = 7;
    hspi2.Init.CRCLength      = SPI_CRC_LENGTH_DATASIZE;
    hspi2.Init.NSSPMode       = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK) 
    {
        return -1;
    }

    HAL_SPI_Receive_IT(&hspi2, &rxBuffer[0], 1);
		return 0;
}

void st_spi_slave_poll(void)
{
    /* prepare data */
    for(int i = 0; i < 128; i++)
    {
        fast_table[i] = i;
    }
    st_spi_slave_init();
    while(1)
    {

    }
}

void ProcessModbusFrame(uint8_t *frame, uint16_t length)
{
    uint32_t slave = frame[0];
    uint8_t functionCode = frame[1];
    if(functionCode == 0x00) //test
    {
        txBuffer[0] = slave;
        txBuffer[1] = functionCode;
        txBuffer[2] = 0xAA; //test data
        uint16_t txLength = 3;
        txInProgress = 1;
        txInProgress = 0;
    }
}
