#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include "st_i2c.h"
#include <stdint.h>

static int gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USER CODE BEGIN I2C1_MspInit 0 */

    /* USER CODE END I2C1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PC0     ------> I2C1_SCL
    PC1     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

int st_i2c_init(I2C_TypeDef *I2Cx, uint8_t address)
{
    __IO uint32_t tmpreg = 0;
    /* Peripheral clock enable */

    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN);

    /* disable I2Cx */
    while (tmpreg != 0U);
    {
        CLEAR_BIT(I2Cx->CR1, I2C_CR1_PE);
        tmpreg = READ_BIT(I2Cx->CR1, I2C_CR1_PE);
    }

    /* Configure I2C3 */
    I2Cx->TIMINGR = 0x00303D5B;                               // Configure timing for 100kHz at 80MHz
    I2Cx->OAR1    = I2C_ADDRESSINGMODE_7BIT | (address << 1); // Set 7-bit addressing mode and device address
    I2Cx->CR2     = 0;                                        // No specific configuration needed for CR2
    SET_BIT(I2Cx->CR1, I2C_CR1_PE);
    return 0;
}

int st_i2c_transmit(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data, uint8_t length, uint32_t xfer_option)
{
    uint8_t xfer_count = 0;

    xfer_count = length;
    /* Wait until I2C is ready */
    while (READ_BIT(I2Cx->ISR, I2C_ISR_BUSY));

    if (length > 0) {
        I2Cx->TXDR = data;

        /* Increment Buffer pointer */
        hi2c->pBuffPtr++;

        hi2c->XferCount--;
        hi2c->XferSize--;
    }

    /* Set address */
    if (xfer_option == ST_I2C_OPT_START_NOEND) {
        I2Cx->CR2 |= (length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk | address & 0xFF;

    } else {
        I2Cx->CR2 |= (length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk | address & 0xFF | I2C_CR2_AUTOEND;
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
    return 0;
}

int st_i2c_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data, uint8_t length, uint32_t xfer_option)
{
    uint8_t xfer_count = 0;
    xfer_count         = length;
    /* Wait until I2C is ready */
    while (READ_BIT(I2Cx->ISR, I2C_ISR_BUSY));

    /* Set address */
    I2Cx->CR2 |= (length << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk | address & 0xFF | I2C_CR2_AUTOEND; // Set address and enable auto-end

    /* Generate START condition */
    SET_BIT(I2Cx->CR2, I2C_CR2_START);

    while (xfer_count > 0) {
        /* Wait for TXIS flag to be set */
        while (!READ_BIT(I2Cx->ISR, I2C_ISR_TXIS));

        /* Send the data byte */
        I2Cx->TXDR = *data++;
        xfer_count--;
    }
    return 0;
}

int st_i2c_deinit(I2C_TypeDef *I2Cx)
{
    return 0;
}
