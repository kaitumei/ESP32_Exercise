#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MAX30102 7-bit I2C 地址: 0x57 (8-bit 写地址 0xAE) */
#define MAX30102_I2C_ADDR          0x57

/* 寄存器地址 */
#define MAX30102_REG_INT_STATUS_1  0x00
#define MAX30102_REG_INT_STATUS_2  0x01
#define MAX30102_REG_INT_ENABLE_1  0x02
#define MAX30102_REG_INT_ENABLE_2  0x03
#define MAX30102_REG_FIFO_WR_PTR   0x04
#define MAX30102_REG_OVF_COUNTER   0x05
#define MAX30102_REG_FIFO_RD_PTR   0x06
#define MAX30102_REG_FIFO_DATA     0x07
#define MAX30102_REG_FIFO_CONFIG   0x08
#define MAX30102_REG_MODE_CONFIG   0x09
#define MAX30102_REG_SPO2_CONFIG   0x0A
#define MAX30102_REG_LED1_PA       0x0C
#define MAX30102_REG_LED2_PA       0x0D
#define MAX30102_REG_PILOT_PA      0x10
#define MAX30102_REG_MULTI_LED_1   0x11
#define MAX30102_REG_MULTI_LED_2   0x12
#define MAX30102_REG_TEMP_INT      0x1F
#define MAX30102_REG_TEMP_FRAC     0x20
#define MAX30102_REG_TEMP_CONFIG   0x21
#define MAX30102_REG_REV_ID        0xFE
#define MAX30102_REG_PART_ID       0xFF

/* 已知 PART_ID = 0x15 */
#define MAX30102_PART_ID_VALUE     0x15

/* MODE_CONFIG 位定义 */
#define MAX30102_MODE_SHDN         (1 << 7)
#define MAX30102_MODE_RESET        (1 << 6)
#define MAX30102_MODE_SPO2         (0x03)   /* Red + IR */
#define MAX30102_MODE_HR_ONLY      (0x02)   /* Red only */

typedef struct {
    void *bus_handle;          /* i2c_master_bus_handle_t */
    void *dev_handle;          /* i2c_master_dev_handle_t */
} max30102_handle_t;

/**
 * @brief 初始化 MAX30102
 *
 * 完成:
 *  1. 初始化 I2C 主机总线
 *  2. 挂载 MAX30102 从设备
 *  3. 软复位
 *  4. 校验 PART_ID
 *  5. 配置 FIFO / 模式 / SpO2 / LED 电流
 *
 * @param handle  驱动句柄
 * @return ESP_OK 成功，其他失败
 */
esp_err_t max30102_init(max30102_handle_t *handle);

/**
 * @brief 写寄存器
 */
esp_err_t max30102_write_reg(max30102_handle_t *handle, uint8_t reg, uint8_t value);

/**
 * @brief 读寄存器
 */
esp_err_t max30102_read_reg(max30102_handle_t *handle, uint8_t reg, uint8_t *value);

/**
 * @brief 连续读多个寄存器（仅适用于地址自增的寄存器，FIFO_DATA 除外）
 */
esp_err_t max30102_read_regs(max30102_handle_t *handle,
                             uint8_t reg,
                             uint8_t *buf,
                             size_t len);

/**
 * @brief 从 FIFO 读取一帧（6 字节：RED[3] + IR[3]，18-bit 左对齐）
 */
esp_err_t max30102_read_fifo(max30102_handle_t *handle, uint8_t fifo_data[6]);

/**
 * @brief 读取芯片温度（整数 + 小数，单位 ℃）
 */
esp_err_t max30102_read_temperature(max30102_handle_t *handle, float *temperature_c);

#ifdef __cplusplus
}
#endif