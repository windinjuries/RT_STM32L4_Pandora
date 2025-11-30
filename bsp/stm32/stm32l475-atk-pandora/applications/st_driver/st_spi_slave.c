#include <stdio.h>
#include <stdint.h>

#include "drivers/dev_pin.h"
#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_spi.h"
#include "main.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_gpio.h>

// SPI handle
SPI_HandleTypeDef hspi2;
// CRC handle
CRC_HandleTypeDef hcrc;                                                  
#define CS_PIN GET_PIN(C, 6)
#define SLAVE_ADDRESS 0x01
#define MAX_FRAME_LEN 256
uint8_t rxBuffer[MAX_FRAME_LEN];
uint8_t txBuffer[MAX_FRAME_LEN];
volatile uint16_t rxIndex = 0;
volatile uint8_t rxComplete = 0;
volatile uint8_t txInProgress = 0;

uint32_t cs_count = 0;
uint32_t cs_low_count = 0;
uint32_t cs_high_count = 0;
uint32_t crc_error_count = 0;
uint32_t crc_cal_count = 0;

uint8_t fast_table[MAX_FRAME_LEN];
uint8_t slow_table[MAX_FRAME_LEN];

typedef enum
{
    SPI_CMD_FAST_READ = 0x00,
    SPI_CMD_SLOW_READ,
} SPI_CMD;


void SPI2_CS_GPIO_Callback()
{
    rt_uint8_t level = rt_pin_read(CS_PIN);
    cs_count++;
    if (level == PIN_LOW) 
    {
        HAL_SPI_TransmitReceive_IT(&hspi2, fast_table, rxBuffer, MAX_FRAME_LEN);
        cs_low_count++;
    } 
    else 
    {
        HAL_SPI_Abort_IT(&hspi2);
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2) 
    {
        uint16_t res = HAL_CRC_Calculate(&hcrc, (uint32_t*)rxBuffer, MAX_FRAME_LEN);
        if(res != 0)
        {
            crc_error_count++;
        }
        crc_cal_count++;        

    }
}

uint32_t rx_count = 0;
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2) 
    {
				rx_count++;

    }
}

static int st_crc_init(void)
{
    // Modbus CRC16
    hcrc.Instance                     = CRC;
    hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;      // 不使用默认多项式
    hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;      // 不使用默认初始值
    hcrc.Init.GeneratingPolynomial    = 0x8005;                          // 设置Modbus多项式
    hcrc.Init.CRCLength               = CRC_POLYLENGTH_16B;              // 16位CRC
    hcrc.Init.InitValue               = 0xFFFF;                          // 设置初始值
    hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_BYTE;    // 按字节反转输入
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE; // 输出结果反转
    hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;      // 按字节处理

    if (HAL_CRC_Init(&hcrc) != HAL_OK) 
    {
        return -1;
    }
}

static int st_spi_slave_init(void)
{
    // SPI Init
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
    // CS Pin Init
    rt_pin_mode(CS_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(CS_PIN, PIN_IRQ_MODE_RISING_FALLING, SPI2_CS_GPIO_Callback, NULL);
    rt_pin_irq_enable(CS_PIN, PIN_IRQ_ENABLE);
		
    return 0;
}

void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi2);
}

extern uint32_t exti_count;
void st_spi_slave_poll(void)
{
    st_spi_slave_init();
    st_crc_init();

    /* prepare send data */
    for(int i = 0; i < MAX_FRAME_LEN - 2; i++)
    {
        fast_table[i] = 0xBB;
    }
    uint16_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)fast_table, MAX_FRAME_LEN -2);
		/* swap crc byte */
    fast_table[MAX_FRAME_LEN - 2] = crc & 0xFF;
		fast_table[MAX_FRAME_LEN - 1] = (crc >> 8) & 0xFF;

    crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)fast_table, MAX_FRAME_LEN);
    
    while(1)
    {
        rt_kprintf("spi cs fall count: %d / %d\n", cs_low_count, cs_count);
        rt_thread_mdelay(1000);
        rt_kprintf("spi slave error count: %d / %d\n", crc_error_count, crc_cal_count);
        rt_thread_mdelay(1000);
			  rt_kprintf("exti count: %d\n", exti_count);
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
