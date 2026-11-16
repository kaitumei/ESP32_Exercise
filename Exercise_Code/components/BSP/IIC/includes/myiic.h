#ifndef __IIC_H__
#define __IIC_H__

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

/* IIC控制块 */
typedef struct my_iic_obj {
    i2c_master_bus_handle_t bus_handle; // I2C总线句柄（新驱动 API，v6.1 签名不变）
    gpio_num_t scl;                     // 时钟引脚
    gpio_num_t sda;                     // 数据引脚
    esp_err_t init_flag;                // 初始化标记（ESP_OK / ESP_FAIL）
} my_iic_obj_t;

extern my_iic_obj_t iic_master[I2C_NUM_MAX];

/* 引脚与相关参数定义（ESP32-S3 可用 GPIO） */
#define IIC0_SDA_GPIO_PIN               GPIO_NUM_41                                                     /* IIC0_SDA引脚 */
#define IIC0_SCL_GPIO_PIN               GPIO_NUM_42                                                     /* IIC0_SCL引脚 */
#define IIC1_SDA_GPIO_PIN               GPIO_NUM_5                                                      /* IIC1_SDA引脚 */
#define IIC1_SCL_GPIO_PIN               GPIO_NUM_4                                                      /* IIC1_SCL引脚 */
#define IIC_FREQ                        400000                                                          /* IIC通信频率 400kHz Fast-mode */
// 函数声明

my_iic_obj_t iic_init(uint8_t iic_port);

#endif