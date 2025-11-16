#include <stdio.h>
#include <stdint.h>

#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_spi.h"
#include "main.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_gpio.h>

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
    SPI_CMD_FAST_READ = 0x00,
    SPI_CMD_SLOW_READ,
} SPI_CMD;

void ProcessModbusFrame(uint8_t *frame, uint16_t length);

 void SPI2_CS_GPIO_Callback()

 {
    rt_uint8_t level = rt_pin_read(GET_PIN(B, 12)); // 读取当前电平
    
    if (level == PIN_HIGH)
    {
        HAL_SPI_Abort(&hspi2);
    }
    else // level == PIN_LOW
    {
		 HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer, 128); 
    }
 }

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2) 
    {
//        if(rxBuffer[0] == SPI_CMD_FAST_READ)
//        {
//            HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer + 1, 127); 
//        }
//				else
//				{
//						HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer + 1, 127); 
//				} 
//				HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer, 128); 
    }
		
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles SPI2 global interrupt.
  */
void SPI2_IRQHandler(void)
{
  /* USER CODE BEGIN SPI2_IRQn 0 */

  /* USER CODE END SPI2_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi2);
  /* USER CODE BEGIN SPI2_IRQn 1 */

  /* USER CODE END SPI2_IRQn 1 */
}

int st_spi_slave_init(void)
{
    hspi2.Instance            = SPI2;
    hspi2.Init.Mode           = SPI_MODE_SLAVE;
    hspi2.Init.Direction      = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize       = SPI_DATASIZE_8BIT;
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
		
		// CS
		rt_pin_mode(GET_PIN(B, 12), PIN_MODE_INPUT_PULLUP);
	
		rt_pin_attach_irq(GET_PIN(B, 12), PIN_IRQ_MODE_FALLING, SPI2_CS_GPIO_Callback, NULL);
	
		/* 使能引脚中断 */
		rt_pin_irq_enable(GET_PIN(B, 12), PIN_IRQ_ENABLE);
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
