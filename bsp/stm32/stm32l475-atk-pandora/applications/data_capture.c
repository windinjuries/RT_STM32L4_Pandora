#include "rtthread.h"
#include "rtdevice.h"
#include "mpu6xxx.h"

#define MPU6XXX_DEVICE_NAME  "i2c3"

uint16_t value = 0;
/* Test function */
int mpu6xxx_poll()
{
    struct mpu6xxx_device *dev;
    struct mpu6xxx_3axes accel, gyro;
    int i;

    /* Initialize mpu6xxx, The parameter is RT_NULL, means auto probing for i2c*/
    dev = mpu6xxx_init(MPU6XXX_DEVICE_NAME, RT_NULL);

    if (dev == RT_NULL)
    {
        rt_kprintf("mpu6xxx init failed\n");
        return -1;
    }
    rt_kprintf("mpu6xxx init succeed\n");

    for(;;)
    {
        mpu6xxx_get_accel(dev, &accel);
        mpu6xxx_get_gyro(dev, &gyro);
        rt_kprintf("accel.x = %3d, accel.y = %3d, accel.z = %3d ", accel.x, accel.y, accel.z);
        rt_kprintf("gyro.x = %3d gyro.y = %3d, gyro.z = %3d\n", gyro.x, gyro.y, gyro.z);
				value = accel.x;
			
			
        rt_thread_mdelay(100);
    }

    mpu6xxx_deinit(dev);

    return 0;
}