#include "at24cxx.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

my_iic_obj_t at24cxx_master;

/*
 * 对于 24C16 及以下型号，高位地址通过改变设备地址实现。
 * 新驱动要求设备地址在注册时固定，因此最多注册 8 个设备句柄
 * （24C16 需要 8 个，24C04/08 需要 2/4 个，24C01/02 只需要 1 个）。
 */
static i2c_master_dev_handle_t at24cxx_dev[8] = {0};
static uint8_t at24cxx_dev_count = 0;

/**********************************************************************
 * @brief       初始化IIC接口并注册AT24CXX设备
 * @param       self：传入的IIC初始化参数
 **********************************************************************/
void at24cxx_init(my_iic_obj_t self)
{
    at24cxx_master = self;

    if (self.init_flag == ESP_FAIL || self.bus_handle == NULL)
    {
        at24cxx_master = iic_init(I2C_NUM_0);
    }

    if (EE_TYPE > AT24C16)
    {
        /* 24C32 及以上：地址为 2 字节，设备地址固定为 0x50 */
        at24cxx_dev_count = 1;

        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = AT_ADDR,
            .scl_speed_hz    = IIC_FREQ,
        };
        ESP_ERROR_CHECK(i2c_master_bus_add_device(at24cxx_master.bus_handle,
                                                  &dev_cfg,
                                                  &at24cxx_dev[0]));
    }
    else
    {
        /* 24C16 及以下：高位地址占用设备地址的 bit1~bit3 */
        at24cxx_dev_count = (EE_TYPE / 256) + 1;

        for (uint8_t i = 0; i < at24cxx_dev_count; i++)
        {
            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address  = AT_ADDR + i,
                .scl_speed_hz    = IIC_FREQ,
            };
            ESP_ERROR_CHECK(i2c_master_bus_add_device(at24cxx_master.bus_handle,
                                                      &dev_cfg,
                                                      &at24cxx_dev[i]));
        }
    }
}

/* 根据 addr 选择正确的设备句柄（仅对 24C16 及以下有效） */
static i2c_master_dev_handle_t at24cxx_get_dev(uint16_t addr)
{
    if (EE_TYPE > AT24C16)
    {
        return at24cxx_dev[0];
    }
    else
    {
        uint8_t idx = addr / 256;
        if (idx >= at24cxx_dev_count)
        {
            idx = at24cxx_dev_count - 1;
        }
        return at24cxx_dev[idx];
    }
}

/***********************************************************************
 * @brief       在AT24CXX指定地址读出一个数据
 * @param       addr：开始读数的地址
 * @retval      读到的数据
 * @note        v6.1 中 i2c_master_transmit_receive 签名不变（第 6 参数仍为超时 ms）。
 *              v6.0 起，从机 NACK 会返回 ESP_ERR_INVALID_RESPONSE，
 *              而非旧版的 ESP_ERR_INVALID_STATE，上层按 != ESP_OK 处理即可。
 **********************************************************************/
uint8_t at24cxx_read_one_byte(uint16_t addr)
{
    uint8_t data = 0;
    i2c_master_dev_handle_t dev = at24cxx_get_dev(addr);
    uint8_t reg_buf[2];
    size_t  reg_len;

    if (EE_TYPE > AT24C16)
    {
        reg_buf[0] = (uint8_t)(addr >> 8);
        reg_buf[1] = (uint8_t)(addr & 0xFF);
        reg_len = 2;
    }
    else
    {
        /* 24C16 及以下：只发送 1 字节低地址，高位由设备句柄体现 */
        reg_buf[0] = (uint8_t)(addr & 0xFF);
        reg_len = 1;
    }

    esp_err_t err = i2c_master_transmit_receive(dev,
                                                reg_buf, reg_len,
                                                &data, 1,
                                                100);
    if (err != ESP_OK)
    {
        ESP_LOGE("24CXX", "read addr 0x%04X failed: %s", addr, esp_err_to_name(err));
    }

    return data;
}
/***********************************************************************
 * @brief       在AT24CXX指定地址写入一个数据
 * @param       addr：写入数据的目的地址
 * @param       data：要写入的数据
 **********************************************************************/
void at24cxx_write_one_byte(uint16_t addr, uint8_t data)
{
    i2c_master_dev_handle_t dev = at24cxx_get_dev(addr);
    uint8_t buf[3];
    size_t  len;

    if (EE_TYPE > AT24C16)
    {
        buf[0] = (uint8_t)(addr >> 8);
        buf[1] = (uint8_t)(addr & 0xFF);
        buf[2] = data;
        len = 3;
    }
    else
    {
        buf[0] = (uint8_t)(addr & 0xFF);
        buf[1] = data;
        len = 2;
    }

    esp_err_t err = i2c_master_transmit(dev, buf, len, 100);
    if (err != ESP_OK)
    {
        ESP_LOGE("24CXX", "write addr 0x%04X failed: %s", addr, esp_err_to_name(err));
    }

    /* EEPROM 写入较慢，必须延时等待内部写周期完成 */
    vTaskDelay(pdMS_TO_TICKS(10));
}
/***********************************************************************
 * @brief       检查AT24CXX是否正常
 * @retval      0: 成功；1: 失败
 **********************************************************************/
uint8_t at24cxx_check(void)
{
    uint8_t temp;
    uint16_t addr = EE_TYPE;

    temp = at24cxx_read_one_byte(addr);
    if (temp == 0x55)
    {
        return 0;
    }
    else
    {
        at24cxx_write_one_byte(addr, 0x55);
        temp = at24cxx_read_one_byte(addr);   /* 注意：原代码此处误写为 255，已修正为 addr */

        if (temp == 0x55)
        {
            return 0;
        }
    }

    return 1;
}
/***********************************************************************
 * @brief       在AT24CXX里面的指定地址开始读出指定个数的数据
 **********************************************************************/
void at24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        *pbuf++ = at24cxx_read_one_byte(addr++);
    }
}
/***********************************************************************
 * @brief       在AT24CXX里面的指定地址开始写入指定个数的数据
 **********************************************************************/
void at24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        at24cxx_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
    }
}