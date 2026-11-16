#include "xl9555.h"
#include <string.h>
#include <stdbool.h>                 /* 使用 bool 类型 */
#include "esp_system.h"              /* esp_restart() */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

my_iic_obj_t xl9555_i2c_master;
static i2c_master_dev_handle_t xl9555_dev_handle = NULL;
static bool xl9555_failed = false;

/*************************************************
 * @brief       读取 XL9555 的 16 位 IO 值
 * @param       *data：读取数据的存储区
 * @param       len：读取数据的大小
 * @retval      ESP_OK：读取成功；其他：读取失败
 **************************************************/
esp_err_t xl9555_read_byte(uint8_t *data, size_t len)
{
    uint8_t memaddr_buf[1];

    if (xl9555_dev_handle == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    memaddr_buf[0] = XL9555_INPUT_PORT0_REG;

    return i2c_master_transmit_receive(xl9555_dev_handle,
                                       memaddr_buf, sizeof(memaddr_buf),
                                       data, len,
                                       100);
}
/*************************************************
 * @brief       向 XL9555 写入 16 位 IO 值
 * @param       reg：寄存器地址
 * @param       data：要写入的数据
 * @param       len：要写入数据的大小
 * @retval      ESP_OK：写入成功；其他：写入失败
 *************************************************/
esp_err_t xl9555_write_byte(uint8_t reg, uint8_t *data, size_t len)
{
    uint8_t buffer[1 + len];

    if (xl9555_dev_handle == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    buffer[0] = reg;

    if (len > 0)
    {
        memcpy(&buffer[1], data, len);
    }

    return i2c_master_transmit(xl9555_dev_handle,
                               buffer, sizeof(buffer),
                               100);
}
/*************************************************
 * @brief       控制某个 IO 的电平
 * @param       pin：控制的IO口
 * @param       val：电平
 * @retval      temp：返回所有IO状态
 *************************************************/
uint16_t xl9555_pin_write(uint16_t pin, int val)
{
    uint8_t w_data[2];
    uint16_t temp = 0x0000;

    xl9555_read_byte(w_data, 2);

    if (pin <= GBC_KEY_IO)
    {
        if (val)
        {
            w_data[0] |= (uint8_t)(0xFF & pin);
        }
        else
        {
            w_data[0] &= ~(uint8_t)(0xFF & pin);
        }
    }
    else
    {
        if (val)
        {
            w_data[1] |= (uint8_t)(0xFF & (pin >> 8));
        }
        else
        {
            w_data[1] &= ~(uint8_t)(0xFF & (pin >> 8));
        }
    }

    temp = ((uint16_t)w_data[1] << 8) | w_data[0];

    xl9555_write_byte(XL9555_OUTPUT_PORT0_REG, w_data, 2);

    return temp;
}
/***************************************************
 * @brief       获取某个IO状态
 * @param       pin     : 要获取状态的IO
 * @retval      此IO口的值(状态, 0/1)
 **************************************************/
int xl9555_pin_read(uint16_t pin)
{
    uint16_t ret;
    uint8_t r_data[2];

    xl9555_read_byte(r_data, 2);

    ret = r_data[1] << 8 | r_data[0];

    return (ret & pin) ? 1 : 0;
}

/****************************************************
 * @brief       XL9555 的 IO 配置
 * @param       config_value：IO配置输入或者输出
 * @retval      返回设置的数值
 ***************************************************/
uint16_t xl9555_ioconfig(uint16_t config_value)
{
    /* P00、P01、P14、P15、P16、P17 为输入，其他引脚为输出
     * 对应二进制：1111 0000 0000 0011，即 0xF003（0 输出，1 输入）*/
    uint8_t data[2];
    esp_err_t err;
    int retry = 3;

    data[0] = (uint8_t)(0xFF & config_value);
    data[1] = (uint8_t)(0xFF & (config_value >> 8));

    do
    {
        err = xl9555_write_byte(XL9555_CONFIG_PORT0_REG, data, 2);

        if (err != ESP_OK)
        {
            retry--;
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGE("IIC", "%s configure %X failed, ret: %d", __func__, config_value, err);
            xl9555_failed = true;

            if ((retry <= 0) && xl9555_failed)
            {
                vTaskDelay(pdMS_TO_TICKS(5000));
                esp_restart();
            }
        }
        else
        {
            xl9555_failed = false;
            break;
        }

    } while (retry);

    return config_value;
}

/****************************************************
 * @brief       初始化XL9555
 * @param       self：传入的 I2C 控制块（若无效则内部重新初始化 I2C0）
 * @retval      无
 ***************************************************/
void xl9555_init(my_iic_obj_t self)
{
    uint8_t r_data[2];

    if ((self.init_flag == ESP_FAIL) || (self.bus_handle == NULL))
    {
        self = iic_init(I2C_NUM_0);        /* 初始化IIC */
    }

    xl9555_i2c_master = self;

    gpio_config_t gpio_init_struct = {0};

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_init_struct.mode = GPIO_MODE_INPUT;
    gpio_init_struct.pin_bit_mask = (1ull << XL9555_INT_IO);
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&gpio_init_struct);     /* 配置 XL9555_INT 引脚 */

    /* 在 I2C 总线上注册 XL9555 设备
     * v6.1：dev_addr_length 用 I2C_ADDR_BIT_LEN_7，
     *       device_address 直接填 7 位地址 0x20，不再左移。 */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = XL9555_ADDR,
        .scl_speed_hz = IIC_FREQ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(self.bus_handle, &dev_cfg, &xl9555_dev_handle));

    /* 上电先读取一次清除中断标志 */
    xl9555_read_byte(r_data, 2);

    xl9555_ioconfig(0xF003);
    xl9555_pin_write(BEEP_IO, 1);
    xl9555_pin_write(SPK_EN_IO, 1);
}

/****************************************************
 * @brief       按键扫描函数
 * @param       mode:0->不连续;1->连续
 * @retval      keyval
 ***************************************************/
uint8_t xl9555_key_scan(uint8_t mode)
{
    uint8_t keyval = 0;
    static uint8_t key_up = 1;                                          /* 按键按松开标志 */

    if (mode)
    {
        key_up = 1;                                                     /* 支持连按 */
    }

    if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0))  /* 按键松开标志为1, 且有任意一个按键按下了 */
    {
        vTaskDelay(pdMS_TO_TICKS(10));                                  /* 去抖动 */
        key_up = 0;

        if (KEY0 == 0)
        {
            keyval = KEY0_PRES;
        }

        if (KEY1 == 0)
        {
            keyval = KEY1_PRES;
        }

        if (KEY2 == 0)
        {
            keyval = KEY2_PRES;
        }

        if (KEY3 == 0)
        {
            keyval = KEY3_PRES;
        }
    }
    else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && KEY3 == 1)          /* 没有任何按键按下, 标记按键松开 */
    {
        key_up = 1;
    }

    return keyval;                                                      /* 返回键值 */
}