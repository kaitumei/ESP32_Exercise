#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "max30102.h"

void app_main(void)
{
    max30102_handle_t max30102 = {0};
    ESP_ERROR_CHECK(max30102_init(&max30102));

    uint8_t fifo[6];
    uint32_t frame_cnt = 0;

    while (1) {
        esp_err_t ret = max30102_read_fifo(&max30102, fifo);
        if (ret == ESP_OK) {
            /* 18-bit 左对齐：高 18 位有效，低 2 位无效 */
            uint32_t red = ((uint32_t)fifo[0] << 16) |
                           ((uint32_t)fifo[1] << 8)  |
                           (uint32_t)fifo[2];
            red &= 0x03FFFF;

            uint32_t ir  = ((uint32_t)fifo[3] << 16) |
                           ((uint32_t)fifo[4] << 8)  |
                           (uint32_t)fifo[5];
            ir &= 0x03FFFF;

            ESP_LOGI("APP", "[%lu] raw=%02X %02X %02X %02X %02X %02X  RED=%lu IR=%lu",
                     frame_cnt++,
                     fifo[0], fifo[1], fifo[2],
                     fifo[3], fifo[4], fifo[5],
                     red, ir);
        } else {
            ESP_LOGW("APP", "read fifo failed: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}