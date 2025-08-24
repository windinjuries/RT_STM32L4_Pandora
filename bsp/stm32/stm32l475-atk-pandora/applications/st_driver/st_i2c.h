#ifndef ST_I2C_H__
#define ST_I2C_H__

#include "stm32l475xx.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
typedef enum {
    ST_I2C_OPT_START_END,
    ST_I2C_OPT_START_NOEND,
    ST_I2C_OPT_NOSTART_END,
}option;




int st_i2c_init(I2C_TypeDef *I2Cx, uint8_t address);

int st_i2c_transmit(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data, uint8_t length, uint32_t xfer_option);

int st_i2c_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t *data, uint8_t length, uint32_t xfer_option);

int st_i2c_deinit(I2C_TypeDef *I2Cx);


#endif /* ST_I2C_H__ */

