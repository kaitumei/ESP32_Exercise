#include "myiic.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* 为IIC0和IIC1分别定义IIC控制块结构体 */
my_iic_obj_t iic_master[I2C_NUM_MAX];  
/*******************************************************
 * @brief       初始化IIC
 * @param       iic_port I2C编号(I2C_NUM_0 / I2C_NUM_1)
 * @retval      IIC0 / IIC1 控制块
 ********************************************************/
my_iic_obj_t iic_init(uint8_t iic_port)
{
    uint8_t i;

    if(iic_port == I2C_NUM_0)
        i = 0;
    else
        i = 1;

    iic_master[i].bus_handle = NULL;
    iic_master[i].init_flag = ESP_FAIL;

    if(iic_port == I2C_NUM_0)
    {
        iic_master[i].scl = IIC0_SCL_GPIO_PIN;
        iic_master[i].sda = IIC0_SDA_GPIO_PIN;
    }
    else if(iic_port == I2C_NUM_1)
    {
        iic_master[i].scl = IIC1_SCL_GPIO_PIN;
        iic_master[i].sda = IIC1_SDA_GPIO_PIN;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = iic_port,
        .sda_io_num = iic_master[i].sda,
        .scl_io_num = iic_master[i].scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags.enable_internal_pullup = true,
    };

    iic_master[i].init_flag = i2c_new_master_bus(&bus_cfg, &iic_master[i].bus_handle);
    if (iic_master[i].init_flag != ESP_OK)
    {
        printf("%s, ret: %d (%s)\n", __func__, iic_master[i].init_flag,
               esp_err_to_name(iic_master[i].init_flag));

        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    return iic_master[i];
}
