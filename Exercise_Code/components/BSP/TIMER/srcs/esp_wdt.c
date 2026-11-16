#include "esp_wdt.h"

static const char *TAG = "WDT";
/*************************************
 * @brief 看门狗任务
 **************************************/
void IRAM_ATTR esp_task(void *arg) {
    esp_task_wdt_add(NULL);     // 订阅看门狗任务

    while(1)
    {
        esp_task_wdt_reset();
        ESP_LOGI(TAG,"喂狗");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*************************************
 * @brief 看门狗初始化
 **************************************/
void esp_wdt_init(void) {
    // 看门狗结构体配置
    esp_task_wdt_config_t wdt_conf = {
        .timeout_ms = 5000, // 设置看门狗超时时间为5秒
        .trigger_panic = true, // 设置触发看门狗时是否触发PANIC
        .idle_core_mask = 0
    };
    esp_task_wdt_reconfigure(&wdt_conf);

    xTaskCreate(esp_task, "esp_task", 1024*2, NULL, 5, NULL);
}