/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#include "data_capture.h"

volatile __attribute__((section(".ext_ram"))) unsigned char value_ram[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

volatile __attribute__((section(".crc"))) unsigned int crc32 = 0x12345678;
/* defined the LED0 pin: PE7 */
#define LED0_PIN    GET_PIN(E, 7)

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    tid1 = rt_thread_create("mpu6xxx", mpu6xxx_poll, RT_NULL, 4096, 20, 10);
    tid2 = rt_thread_create("ap3216c", ap3216_poll, RT_NULL, 4096, 20, 10);
    rt_thread_startup(tid1);
    rt_thread_startup(tid2);

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

