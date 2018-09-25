#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>


#include "mpu6050.h"


static struct task_struct *task;
static int data = 0x55;

struct mpu6050_data {
    struct i2c_client *drv_client;
    int accel_values[3];
    int gyro_values[3];
    int temperature;
};

int GLOBAL_VARIABLE = 0;
EXPORT_SYMBOL(GLOBAL_VARIABLE);


static struct mpu6050_data g_mpu6050_data;

    /*Read data*/
static int mpu6050_read_data(void){
    
    int temp;
    struct i2c_client *drv_client = g_mpu6050_data.drv_client;

    if (drv_client == 0)
        return -ENODEV;

    /* read data accel */
    g_mpu6050_data.accel_values[0] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_ACCEL_XOUT_H));
    g_mpu6050_data.accel_values[1] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_ACCEL_YOUT_H));
    g_mpu6050_data.accel_values[2] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_ACCEL_ZOUT_H));
    /* read data gyro */
    g_mpu6050_data.gyro_values[0] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_GYRO_XOUT_H));
    g_mpu6050_data.gyro_values[1] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_GYRO_YOUT_H));
    g_mpu6050_data.gyro_values[2] = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_GYRO_ZOUT_H));
    /* Temperature in degrees C =
     * (TEMP_OUT Register Value  as a signed quantity)/340 + 36.53
     */
    /*read data temperature*/
    temp = (s16)((u16)i2c_smbus_read_word_swapped(drv_client, REG_TEMP_OUT_H));
    g_mpu6050_data.temperature = (temp + 12420 + 170) / 340;
 

    GLOBAL_VARIABLE= g_mpu6050_data.gyro_values[0];

    /*print data accel, gyro, temperature  using dev_info*/
    dev_info(&drv_client->dev, "sensor data read:\n");
    dev_info(&drv_client->dev, "ACCEL[X,Y,Z] = [%d, %d, %d]\n",
        g_mpu6050_data.accel_values[0],
        g_mpu6050_data.accel_values[1],
        g_mpu6050_data.accel_values[2]);
    dev_info(&drv_client->dev, "GYRO[X,Y,Z] = [%d, %d, %d]\n",
        g_mpu6050_data.gyro_values[0],
        g_mpu6050_data.gyro_values[1],
        g_mpu6050_data.gyro_values[2]);
    dev_info(&drv_client->dev, "TEMP = %d\n",
        g_mpu6050_data.temperature);

    return 0;
}



int thread_function(void *data){
    while(!kthread_should_stop()){

        mpu6050_read_data();

        msleep(1500);
        schedule();
    }

    return 0;
}


/*Create probe*/
static int mpu6050_probe(struct i2c_client *drv_client, const struct i2c_device_id *id)
{   
    int ret;
    
    dev_info(&drv_client->dev,
        "i2c client address is 0x%X\n", drv_client->addr);

    /*Read who_am_i register: mpu6050*/
    ret = i2c_smbus_read_byte_data(drv_client, REG_WHO_AM_I);
    
    if(IS_ERR_VALUE(ret)){
        dev_err(&drv_client->dev,
        "mpu6050: i2c_smbus_read_byte_data() failed with error:%d\n", ret);
        return ret;
    } 
    
    if(ret != MPU6050_WHO_AM_I){
        dev_err(&drv_client->dev,
            "wrong i2c device found: expected 0x%X, found 0x%X\n",
            MPU6050_WHO_AM_I, ret);
        return -1;  
    }
    
    dev_info(&drv_client->dev, 
        "mpu6050: i2c mpu6050 device found, WHO_AM_I register value = 0x%X\n", ret);
    
    
    /* Setup the device */
    i2c_smbus_write_byte_data(drv_client, REG_CONFIG, 0);
    i2c_smbus_write_byte_data(drv_client, REG_GYRO_CONFIG, 0);
    i2c_smbus_write_byte_data(drv_client, REG_ACCEL_CONFIG, 0);
    i2c_smbus_write_byte_data(drv_client, REG_FIFO_EN, 0);
    i2c_smbus_write_byte_data(drv_client, REG_INT_PIN_CFG, 0);
    i2c_smbus_write_byte_data(drv_client, REG_INT_ENABLE, 0);
    i2c_smbus_write_byte_data(drv_client, REG_USER_CTRL, 0);
    i2c_smbus_write_byte_data(drv_client, REG_PWR_MGMT_1, 0);
    i2c_smbus_write_byte_data(drv_client, REG_PWR_MGMT_2, 0);
    
    g_mpu6050_data.drv_client = drv_client;

    dev_info(&drv_client->dev,
        "mpu6050: i2c driver prode\n");

    return 0;

}

static int mpu6050_remove(struct i2c_client *dvr_client)
{
    printk(KERN_INFO "mpu6050:i2c driver removed\n");
    return 0;
}

static const struct i2c_device_id mpu6050_idtable []={
    {"mpu6050",0},
    {}
};
MODULE_DEVICE_TABLE(i2c, mpu6050_idtable);

static struct i2c_driver mpu6050_i2c_driver={
    .driver={
        .name="mpu6050",
    },

    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .id_table = mpu6050_idtable,
};



static int mpu6050_init(void)
{
    int ret;
    
    /* Create i2c driver*/
    ret = i2c_add_driver(&mpu6050_i2c_driver);
    if(ret){
        printk(KERN_ERR "mpu6050:failed to add new i2c driver:%d\n", ret);
        return ret; 
    }
    printk(KERN_INFO "mpu6050:i2c driver created\n");
    printk(KERN_INFO "mpu6050: module loaded\n");

     task = kthread_run(&thread_function,(void *)data,"Mpu5060thread"); 


    return 0;
}

static void mpu6050_exit(void){   
    i2c_del_driver(&mpu6050_i2c_driver);
    printk(KERN_INFO "mpu6050: i2c driver deleted\n");

    printk(KERN_INFO "mpu6050: module exited\n");
    kthread_stop(task);
}

module_init(mpu6050_init);
module_exit(mpu6050_exit);

MODULE_DESCRIPTION("mpu6050 I2C");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");