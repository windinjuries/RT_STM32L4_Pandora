#include "ap3216.h"
#include "st_i2c.h"
#include <rtthread.h>

#define AP3216C_I2C I2C3
static int ap3216c_write_reg(unsigned char reg, unsigned char cmd)
{
    int ret              = -1;
    unsigned char buf[2] = {0};

    buf[0] = reg;
    buf[1] = cmd;

    ret = st_i2c_transmit(AP3216C_I2C, AP3216C_ADDR, buf, 2, ST_I2C_OPT_START_END);
    if (ret < 0) {
        rt_kprintf("write cmd to ap3216c register failure.\n");
        return -1;
    }
    return 0;
}

static int ap3216c_read_reg(unsigned char reg, unsigned char *val)
{
    int ret              = -1;
    unsigned char buf[1] = {0};

    buf[0] = reg;                                                                        // send register address
    ret    = st_i2c_transmit(AP3216C_I2C, AP3216C_ADDR, buf, 1, ST_I2C_OPT_START_NOEND); // write register address to the device
    if (ret < 0) {
        rt_kprintf("write cmd to ap3216c register failure.\n");
        return -1;
    }

    ret = st_i2c_read(AP3216C_I2C, AP3216C_ADDR, buf, 1, ST_I2C_OPT_START_END); // read data from the register
    if (ret < 0) 
    {
        rt_kprintf("get the humidy failure.\n");
        return -1;
    }
    *val = buf[0];

    return 0;
}

void ap3216c_read_datas(ap3216c_data *pdata)
{
    unsigned char i = 0;
    unsigned char buf[6], val = 0;

    /* read all sensor data */
    for (i = 0; i < 6; i++) {
        rt_kprintf("read % data\n", i);
        ap3216c_read_reg(AP3216C_IRDATALOW + i, &val);
        buf[i] = val;
    }

    /* IR   */
    if (buf[0] & 0X80) {
        /* IR_OF位为1,则数据无效 */
        pdata->ir = 0;
    } else {
        pdata->ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03);
    }

    /* ALS ambient light sensor */
    pdata->als = ((unsigned short)buf[3] << 8) | buf[2];

    /* PS proximity sensor */
    if (buf[4] & 0x40) {
        /* IR_OF位为1,则数据无效 */
        pdata->ps = 0;
    } else {
        pdata->ps = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);
    }
}

void ap3216c_release(void)
{
    // close( fd );
}

int ap3216c_init(void)
{
    // init i2c
    int ret = st_i2c_init(AP3216C_I2C, AP3216C_ADDR);
    if (ret < 0) {
        rt_kprintf("%s %s i2c device open failure: %s\n", __FILE__, __FUNCTION__);
        return -1;
    }

    // reset sensor
    ap3216c_write_reg(AP3216C_SYSTEMCONG, 0x04);
    rt_thread_mdelay(2000);

    // enable ALS、PS+IR
    ap3216c_write_reg(AP3216C_SYSTEMCONG, 0X03);
    rt_thread_mdelay(2000);

    return 0;
}