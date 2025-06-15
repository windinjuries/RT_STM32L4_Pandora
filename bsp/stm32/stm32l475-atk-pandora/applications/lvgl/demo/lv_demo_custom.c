
#include <lvgl.h>
#include <drv_lcd.h>
#include <rtthread.h>
static int counter = 0;
extern uint16_t value;
lv_obj_t * label;
void timer_cb(lv_timer_t * timer) {
    lv_label_set_text_fmt(label, "count: %d", value);
    if(counter > 100) counter = 0;
}

void lv_demo_custom(void)
{

   label = lv_label_create(lv_scr_act());


lv_timer_create(timer_cb, 250, NULL);  // 250ms 更新周期
}