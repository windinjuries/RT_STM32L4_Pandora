#include "st_i2c.h"
#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include <rtthread.h>
#include <stdint.h>

static int gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USER CODE BEGIN I2C1_MspInit 0 */

    /* USER CODE END I2C1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PC0     ------> I2C3_SCL
    PC1     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    return 0;
}

int st_i2c_init(I2C_TypeDef *I2Cx, uint8_t address)
{
    __IO uint32_t tmpreg = 0;

    gpio_init();

    /* Peripheral clock enable */

    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN);

    __HAL_RCC_I2C3_CLK_ENABLE();

    /* disable I2Cx */
    while (tmpreg != 0U) {
        CLEAR_BIT(I2Cx->CR1, I2C_CR1_PE);
        tmpreg = READ_BIT(I2Cx->CR1, I2C_CR1_PE);
    }

    /* Configure I2C3 */
    I2Cx->TIMINGR = 0x00303D5B; // Configure timing for 100kHz at 80MHz
    I2Cx->OAR1    = I2C_ADDRESSINGMODE_7BIT |
                 (address << 1); // Set 7-bit addressing mode and device address
    I2Cx->CR2 = 0;               // No specific configuration needed for CR2
    SET_BIT(I2Cx->CR1, I2C_CR1_PE);
    return 0;
}

int st_i2c_transmit(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data,
                    uint8_t length, uint32_t xfer_option)
{
    uint8_t xfer_count = 0;

    xfer_count = length;
    /* Wait until I2C is ready */
    uint32_t timeout = 10000;
    while (READ_BIT(I2Cx->ISR, I2C_ISR_BUSY) && timeout--) {
        if (timeout == 0)
            return -1;
    }
    /* Set address */

    if (length > 0U) {
        /* Preload TX register */
        /* Write data to TXDR */
        I2Cx->TXDR = *data++;
        xfer_count--;
    }
    if (xfer_option == ST_I2C_OPT_START_NOEND) {
        I2Cx->CR2 = ((length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk) |
                    (address & 0xFF) << 1;

    } else {
        I2Cx->CR2 = ((length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk) |
                    (address & 0xFF) << 1 | I2C_CR2_AUTOEND;
    }

    /* Generate START condition */
    SET_BIT(I2Cx->CR2, I2C_CR2_START);

    while (xfer_count > 0) {
        /* Wait for TXIS flag to be set */
        while (!READ_BIT(I2Cx->ISR, I2C_ISR_TXIS));

        /* Send the data byte */
        I2Cx->TXDR = *data++;
        xfer_count--;
    }
    timeout = 20000;
    if (xfer_option == ST_I2C_OPT_START_NOEND) {
        while (!READ_BIT(I2Cx->ISR, I2C_ISR_TC)) {
            timeout--;
            if (timeout <= 0) {
                rt_kprintf("transmit recv TC fail\n");
                return -1;
            }
        }
    } else {
        while (!READ_BIT(I2Cx->ISR, I2C_ISR_STOPF)) {
            timeout--;
            if (timeout <= 0) {
                rt_kprintf("transmit clear STOPF fail\n");
                return -1;
            }
        }
        SET_BIT(I2Cx->ICR, I2C_ICR_STOPCF);
    }

    /* Clear Configuration Register 2 */
    /* Clear CR2 register */
    CLEAR_BIT(I2Cx->CR2, (I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
                          I2C_CR2_AUTOEND | I2C_CR2_RD_WRN));
    return 0;
}

int st_i2c_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data,
                uint8_t length, uint32_t xfer_option)
{
    uint8_t xfer_count = 0;
    xfer_count         = length;
    /* Wait until I2C is ready */
    uint32_t timeout = 10000;
    //    while (READ_BIT(I2Cx->ISR, I2C_ISR_BUSY) && timeout--)
    //    {
    //        if(timeout == 0) return -1;
    //    }

    /* Set address */
    I2Cx->CR2 = ((length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk) |
                (address & 0xFF) << 1 | I2C_CR2_RD_WRN |
                I2C_CR2_AUTOEND; // Set address and enable auto-end

    /* Generate START condition */
    SET_BIT(I2Cx->CR2, I2C_CR2_START);

    while (xfer_count > 0) {
        /* Wait for TXIS flag to be set */
        while (!READ_BIT(I2Cx->ISR, I2C_ISR_RXNE));

        /* Send the data byte */
        *data = I2Cx->RXDR;
        data++;
        xfer_count--;
    }
    timeout = 20000;
    while (!READ_BIT(I2Cx->ISR, I2C_ISR_STOPF)) {
        timeout--;
        if (timeout <= 0) {
            rt_kprintf("clear STOPF fail\n");
            return -1;
        }
    }
    SET_BIT(I2Cx->ICR, I2C_ICR_STOPCF);
    return 0;
}

int st_i2c_deinit(I2C_TypeDef *I2Cx)
{
    uint32_t tmpreg = 0;

    return 0;
}
