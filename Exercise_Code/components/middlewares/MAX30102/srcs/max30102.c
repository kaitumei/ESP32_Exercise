#include "max30102.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MAX30102";

/* ===================== 可配置参数 ===================== */

#define I2C_MASTER_SCL_IO          GPIO_NUM_5
#define I2C_MASTER_SDA_IO          GPIO_NUM_4
#define I2C_MASTER_FREQ_HZ         100000      /* MAX30102 支持 400 kHz */
#define I2C_MASTER_TIMEOUT_MS      1000

/* FIFO 配置：
 * SMP_AVE = 4, FIFO_ROLLOVER_EN = 1, FIFO_A_FULL = 15
 * 0b010_1_1111 = 0x5F
 */
#define MAX30102_FIFO_CONFIG_VAL   0x5F

/* SpO2 配置：
 * SPO2_ADC_RGE = 4096nA, SPO2_SR = 100Hz, LED_PW = 411us (18-bit)
 * 0b001_000_11 = 0x23
 */
#define MAX30102_SPO2_CONFIG_VAL   0x23

/* LED 电流：0x24 ≈ 7.2mA，可按实际佩戴情况调整 */
#define MAX30102_LED_CURRENT       0x24

/* ===================================================== */

static esp_err_t max30102_reset(max30102_handle_t *handle)
{
    esp_err_t ret = max30102_write_reg(handle, MAX30102_REG_MODE_CONFIG,
                                       MAX30102_MODE_RESET);
    if (ret != ESP_OK) {
        return ret;
    }

    /* 复位后需要等待，RESET 位自动清零 */
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t mode = 0;
    ret = max30102_read_reg(handle, MAX30102_REG_MODE_CONFIG, &mode);
    if (ret != ESP_OK) {
        return ret;
    }

    if (mode & MAX30102_MODE_RESET) {
        ESP_LOGE(TAG, "reset bit still set, reset failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "reset done, MODE_CONFIG=0x%02X", mode);
    return ESP_OK;
}

static esp_err_t max30102_check_part_id(max30102_handle_t *handle)
{
    uint8_t part_id = 0;
    esp_err_t ret = max30102_read_reg(handle, MAX30102_REG_PART_ID, &part_id);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "PART_ID = 0x%02X", part_id);

    if (part_id != MAX30102_PART_ID_VALUE) {
        ESP_LOGE(TAG, "unexpected PART_ID 0x%02X, check wiring or chip model",
                 part_id);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

static esp_err_t max30102_config_registers(max30102_handle_t *handle)
{
    esp_err_t ret;

    /* 1. FIFO 配置 */
    ret = max30102_write_reg(handle, MAX30102_REG_FIFO_CONFIG,
                             MAX30102_FIFO_CONFIG_VAL);
    if (ret != ESP_OK) return ret;

    /* 2. 清空 FIFO 指针（可选但推荐） */
    ret = max30102_write_reg(handle, MAX30102_REG_FIFO_WR_PTR, 0x00);
    if (ret != ESP_OK) return ret;
    ret = max30102_write_reg(handle, MAX30102_REG_OVF_COUNTER, 0x00);
    if (ret != ESP_OK) return ret;
    ret = max30102_write_reg(handle, MAX30102_REG_FIFO_RD_PTR, 0x00);
    if (ret != ESP_OK) return ret;

    /* 3. SpO2 配置：ADC 范围、采样率、脉冲宽度 */
    ret = max30102_write_reg(handle, MAX30102_REG_SPO2_CONFIG,
                             MAX30102_SPO2_CONFIG_VAL);
    if (ret != ESP_OK) return ret;

    /* 4. LED 电流：RED 和 IR */
    ret = max30102_write_reg(handle, MAX30102_REG_LED1_PA,
                             MAX30102_LED_CURRENT);
    if (ret != ESP_OK) return ret;
    ret = max30102_write_reg(handle, MAX30102_REG_LED2_PA,
                             MAX30102_LED_CURRENT);
    if (ret != ESP_OK) return ret;

    /* 5. 可选：PILOT LED 电流 */
    ret = max30102_write_reg(handle, MAX30102_REG_PILOT_PA,
                             MAX30102_LED_CURRENT);
    if (ret != ESP_OK) return ret;

    /* 6. 多模式插槽配置（仅 SpO2 模式下可保持默认，这里显式写为 RED + IR） */
    ret = max30102_write_reg(handle, MAX30102_REG_MULTI_LED_1, 0x21); /* SLOT1=RED, SLOT2=IR */
    if (ret != ESP_OK) return ret;
    ret = max30102_write_reg(handle, MAX30102_REG_MULTI_LED_2, 0x00); /* SLOT3/4 不使用 */
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "registers configured");
    return ESP_OK;
}

/* ===================== 公开 API ===================== */

esp_err_t max30102_write_reg(max30102_handle_t *handle, uint8_t reg, uint8_t value)
{
    if (handle == NULL || handle->dev_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[2] = {reg, value};
    return i2c_master_transmit((i2c_master_dev_handle_t)handle->dev_handle,
                               buf, sizeof(buf), I2C_MASTER_TIMEOUT_MS);
}

esp_err_t max30102_read_reg(max30102_handle_t *handle, uint8_t reg, uint8_t *value)
{
    if (handle == NULL || handle->dev_handle == NULL || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(
        (i2c_master_dev_handle_t)handle->dev_handle,
        &reg, 1,
        value, 1,
        I2C_MASTER_TIMEOUT_MS);
}

esp_err_t max30102_read_regs(max30102_handle_t *handle,
                             uint8_t reg,
                             uint8_t *buf,
                             size_t len)
{
    if (handle == NULL || handle->dev_handle == NULL || buf == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(
        (i2c_master_dev_handle_t)handle->dev_handle,
        &reg, 1,
        buf, len,
        I2C_MASTER_TIMEOUT_MS);
}

esp_err_t max30102_read_fifo(max30102_handle_t *handle, uint8_t fifo_data[6])
{
    /*
     * FIFO_DATA 是特殊寄存器：连续读取时地址指针不会自增，
     * 每次读都会弹出 FIFO 中的一帧。因此直接连续读 6 字节即可。
     */
    return max30102_read_regs(handle, MAX30102_REG_FIFO_DATA, fifo_data, 6);
}

esp_err_t max30102_read_temperature(max30102_handle_t *handle, float *temperature_c)
{
    if (handle == NULL || temperature_c == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    /* 触发一次温度转换 */
    ret = max30102_write_reg(handle, MAX30102_REG_TEMP_CONFIG, 0x01);
    if (ret != ESP_OK) return ret;

    /* 等待转换完成（典型 30~50 ms） */
    vTaskDelay(pdMS_TO_TICKS(50));

    uint8_t temp_int = 0, temp_frac = 0;
    ret = max30102_read_reg(handle, MAX30102_REG_TEMP_INT, &temp_int);
    if (ret != ESP_OK) return ret;

    ret = max30102_read_reg(handle, MAX30102_REG_TEMP_FRAC, &temp_frac);
    if (ret != ESP_OK) return ret;

    *temperature_c = (float)temp_int + (float)temp_frac * 0.0625f;
    return ESP_OK;
}

esp_err_t max30102_init(max30102_handle_t *handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    /* ---------- 1. I2C 主机总线配置 ---------- */
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ret = i2c_new_master_bus(&bus_cfg, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(ret));
        return ret;
    }
    handle->bus_handle = bus_handle;

    /* ---------- 2. 挂载 MAX30102 从设备 ---------- */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MAX30102_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    i2c_master_dev_handle_t dev_handle;
    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s", esp_err_to_name(ret));
        i2c_del_master_bus(bus_handle);
        handle->bus_handle = NULL;
        return ret;
    }
    handle->dev_handle = dev_handle;

    /* ---------- 3. 可选：探测设备是否在线 ---------- */
    ret = i2c_master_probe(bus_handle, MAX30102_I2C_ADDR, I2C_MASTER_TIMEOUT_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MAX30102 not found on bus, check wiring/address");
        goto fail;
    }

    /* ---------- 4. 软复位 ---------- */
    ret = max30102_reset(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "reset failed");
        goto fail;
    }

    /* ---------- 5. 校验 PART_ID ---------- */
    ret = max30102_check_part_id(handle);
    if (ret != ESP_OK) {
        goto fail;
    }

    /* ---------- 6. 配置工作参数（此时芯片仍处于关断模式） ---------- */
    ret = max30102_config_registers(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "register config failed");
        goto fail;
    }

    /* ---------- 7. 进入 SpO2 模式（RED + IR） ---------- */
    ret = max30102_write_reg(handle, MAX30102_REG_MODE_CONFIG,
                             MAX30102_MODE_SPO2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "enter SpO2 mode failed");
        goto fail;
    }

    ESP_LOGI(TAG, "MAX30102 initialized successfully");
    return ESP_OK;

fail:
    if (handle->dev_handle) {
        i2c_master_bus_rm_device((i2c_master_dev_handle_t)handle->dev_handle);
        handle->dev_handle = NULL;
    }
    if (handle->bus_handle) {
        i2c_del_master_bus((i2c_master_bus_handle_t)handle->bus_handle);
        handle->bus_handle = NULL;
    }
    return ret;
}