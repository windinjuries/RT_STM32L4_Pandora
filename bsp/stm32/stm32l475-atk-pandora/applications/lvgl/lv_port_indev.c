/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */
#include <lvgl.h>
#include <rtdevice.h>
#include <drv_gpio.h>

#define BUTTON0_PIN        GET_PIN(D, 10)
#define BUTTON1_PIN        GET_PIN(D, 9)
#define BUTTON2_PIN        GET_PIN(D, 8)
#define BUTTON_WKUP_PIN    GET_PIN(C, 13)

lv_indev_t * button_indev;

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{
    switch(id)
    {
    case 0:
        if(rt_pin_read(BUTTON_WKUP_PIN) == PIN_HIGH)
            return true;
        break;
    case 1:
        if(rt_pin_read(BUTTON0_PIN) == PIN_LOW)
            return true;
        break;
    case 2:
        if(rt_pin_read(BUTTON1_PIN) == PIN_LOW)
            return true;
        break;
    case 3:
			if(rt_pin_read(BUTTON2_PIN) == PIN_LOW)
            return true;
        break;
    }

    return false;
}

static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    static uint8_t key_type_table[] = {LV_KEY_DOWN, LV_KEY_NEXT, LV_KEY_DOWN, LV_KEY_PREV};

    /*Check to buttons see which is being pressed*/
    for(i = 0; i < 4; i++)
    {
        /*Return the pressed button's ID*/
        if(button_is_pressed(i))
        {
            return key_type_table[i];
        }
    }

    /*No button pressed*/
    return -1;
}

void button_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    int btn_pr = button_get_pressed_id();
		static bool but_flag = true;
	  lv_indev_state_t encoder_act = LV_INDEV_STATE_RELEASED;
    int32_t encoder_diff = 0;

    if(btn_pr == LV_KEY_DOWN && but_flag)
		{
        encoder_act = LV_INDEV_STATE_PR;
				but_flag = false;
				rt_kprintf("down\n");
    }
    else if((btn_pr == LV_KEY_PREV) && but_flag)
    {
   	 	  encoder_diff--;
        but_flag = false;
			  rt_kprintf("prev\n");
    }
    else if((btn_pr == LV_KEY_NEXT) && but_flag)
		{
    	  encoder_diff++;
        but_flag = false;
				rt_kprintf("next\n");
    }
    else 
    {
			but_flag = true;
		}
    data->enc_diff = encoder_diff;
    data->state = encoder_act;
}

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;

    /* Initialize the on-board buttons */
    rt_pin_mode(BUTTON0_PIN, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON1_PIN, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON2_PIN, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON_WKUP_PIN, PIN_MODE_INPUT);

    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = button_read;

    /*Register the driver in LVGL and save the created input device object*/
    button_indev = lv_indev_drv_register(&indev_drv);
	
		lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(button_indev, g);

}
