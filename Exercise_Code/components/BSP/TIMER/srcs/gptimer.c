#include "gptimer.h"
// 声明句柄
static const char *TIMER_TAG = "GPTIMER";
static gptimer_handle_t gptimer_handle = NULL;
static QueueHandle_t gptimer_queue = NULL;
/**************************************************************************
 * @brief gptimer 回调函数
 * @param timer: gptimer句柄
 * @param *edata: gptimer事件数据
 * @param user_ctx: 用户自定义数据
 * @retval true: 任务唤醒
 *         false: 任务不唤醒
 **************************************************************************/
static IRAM_ATTR bool gptimer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    BaseType_t wake = pdFALSE;
    uint8_t sig = 1;
    xQueueSendFromISR(gptimer_queue, &sig, &wake);
    return wake == pdTRUE;
}
/***************************************************************************
 * @brief gptimer 任务
 ***************************************************************************/
static void gptimer_task(void *arg) {
    uint8_t sig;
    while(1) {
        if(xQueueReceive(gptimer_queue, &sig, portMAX_DELAY)) {
            ESP_LOGI(TIMER_TAG, "Received signal from GPTimer");
            // 处理信号的逻辑
            LED_TOGGLE();
        }

    }
}
/**************************************************************************
 * @brief 初始化GPTimer模块
 * @return void
 * @note 初始化GPTimer模块，包括创建定时器、注册回调函数、创建定时器任务等。
 **************************************************************************/
void gptimer_init(void) {

    gptimer_queue = xQueueCreate(10, sizeof(uint8_t));

    // 定时器结构体配置
    gptimer_config_t gptimer_conf = {
        .clk_src = GPTIMER_CLK_SRC_APB,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000 // 1MHz
    };
    gptimer_new_timer(&gptimer_conf, &gptimer_handle);

    // 注册定时器回调函数
    gptimer_event_callbacks_t gptimer_event_callbacks = {
        .on_alarm = gptimer_callback
    };
    gptimer_register_event_callbacks(gptimer_handle, &gptimer_event_callbacks, NULL);

    // 报警结构体配置
    gptimer_alarm_config_t alert_conf = {
        .alarm_count = 5000000,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true
    };
    gptimer_set_alarm_action(gptimer_handle, &alert_conf);

    gptimer_enable(gptimer_handle);
    gptimer_start(gptimer_handle);

    // 创建任务
    xTaskCreate(gptimer_task, "gptimer_task", 2048, NULL, 5, NULL);
}

