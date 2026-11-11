#include "exit.h"

static const char *TAG = "ESP_EXIT";
// 定义一个队列用于处理按键中断
static QueueHandle_t exito_queue = NULL;


// 中断服务函数
void IRAM_ATTR exito_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    // 发送事件到队列
    xQueueSendFromISR(exito_queue,&gpio_num,NULL);
}

// 按键处理
void buntton_task(void *arg)
{
    uint32_t gpio_num;
    for(;;)
    {
        if(xQueueReceive(exito_queue, &gpio_num, portMAX_DELAY))
        {
            vTaskDelay(pdMS_TO_TICKS(20));

            if(BOOT_DOWN == 0)
            {
                ESP_LOGI(TAG, "Button pressed on GPIO %"PRIu32, gpio_num);
                LED_TOGGLE();
            }   

        }
    }

}

void exit_init(void)
{
    gpio_config_t gpio_conf = {
        .pin_bit_mask = 1ull << BOOT_INT_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&gpio_conf);
    // 创建一个队列，用于接收按键中断的GPIO编号
    exito_queue = xQueueCreate(10, sizeof(uint32_t));
    // 安装GPIO ISR服务
    gpio_install_isr_service(0);
    // 注册按键中断服务程序
    gpio_isr_handler_add(BOOT_INT_GPIO_PIN, exito_isr_handler, (void*)BOOT_INT_GPIO_PIN);

    gpio_intr_enable(BOOT_INT_GPIO_PIN);

    xTaskCreate(buntton_task, "button_task", 1024, NULL, 5, NULL);

    ESP_LOGI(TAG, "Button interrupt configured on GPIO %d", BOOT_INT_GPIO_PIN);
}
