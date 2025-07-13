#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include <stdint.h>


static int gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USER CODE BEGIN I2C1_MspInit 0 */

    /* USER CODE END I2C1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


}

int st_i2c_init(I2C_TypeDef *I2Cx, uint8_t address)
{
    __IO uint32_t tmpreg = 0;
    /* Peripheral clock enable */

    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C3EN);

    /* disable I2Cx */
    while(tmpreg != 0U);
    {
        CLEAR_BIT(I2Cx->CR1, I2C_CR1_PE);
        tmpreg = READ_BIT(I2Cx->CR1, I2C_CR1_PE);
    }

    /* Configure I2C3 */
    I2Cx->TIMINGR = 0x00303D5B; // Configure timing for 100kHz at 80MHz
    I2Cx->OAR1 = I2C_ADDRESSINGMODE_7BIT | (address << 1); // Set 7-bit addressing mode and device address
    I2Cx->CR2 = 0; // No specific configuration needed for CR2
    return 0;
}

int st_i2c_write(I2C_TypeDef *I2Cx, uint8_t address, uint8_t data)
{
    /* Wait until I2C is ready */
    while (READ_BIT(I2Cx->ISR, I2C_ISR_BUSY));

    /* Generate START condition */
    SET_BIT(I2Cx->CR2, I2C_CR2_START);

    /* Wait for START condition to be sent */
    while (!READ_BIT(I2Cx->ISR, I2C_ISR_TXIS));

    /* Send the device address with write direction */
    I2Cx->TXDR = (address << 1) & ~I2C_OAR1_OA1;

    /* Wait for address to be sent */
    while (!READ_BIT(I2Cx->ISR, I2C_ISR_ADDR));

    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_FLAG(I2Cx, I2C_FLAG_ADDR);

    /* Send the data byte */
    I2Cx->TXDR = data;

    /* Wait for data to be transmitted */
    while (!READ_BIT(I2Cx->ISR, I2C_ISR_TC));

    /* Generate STOP condition */
    SET_BIT(I2Cx->CR2, I2C_CR2_STOP);
 
     return 0;
}
{


}

int st_i2c_read()
{

}

int st_i2c_deinit()
{

}








