#include "stdio.h"
#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include <stdint.h>

SPI_HandleTypeDef hspi1;

/* Modbus相关变量 */
#define SLAVE_ADDRESS 0x01
#define MAX_FRAME_LEN 256
uint8_t rxBuffer[MAX_FRAME_LEN];
uint8_t txBuffer[MAX_FRAME_LEN];
volatile uint16_t rxIndex = 0;
volatile uint8_t rxComplete = 0;
volatile uint8_t txInProgress = 0;

void ProcessModbusFrame(uint8_t *frame, uint16_t length);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_4) 
    { 
        // CS引脚
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) 
        {
            // CS下降沿：开始接收数据
            rxIndex = 0;
            HAL_SPI_Receive_IT(&hspi1, &rxBuffer[rxIndex], 1);
        } 
        else 
        {
            // CS上升沿：接收完成，处理数据
            rxComplete = 1;
            HAL_SPI_Abort(&hspi1); // 停止SPI接收
        }
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) 
    {
        if (rxIndex < MAX_FRAME_LEN) 
        {
            rxIndex++;
            HAL_SPI_Receive_IT(&hspi1, &rxBuffer[rxIndex], 1); // 继续接收
        } 
        else 
        {
            // 缓冲区溢出
            HAL_SPI_Abort(&hspi1);
        }
    }
}

void process(void)
{
    if (rxComplete && !txInProgress) 
    {
        rxComplete = 0;
        ProcessModbusFrame();
        rxIndex = 0;
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
