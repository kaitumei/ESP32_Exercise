#include "esptimer.h"

/* 中断回调函数 */ 
static void esp_callback(void* arg) {
    LED_TOGGLE();
}
/**************************************************
 * @brief 初始化ESP Timer
 * @param tps 每秒的周期数，单位为Hz
 * @return void
 * @note 该函数会创建一个ESP Timer，并启动一个周期
 **************************************************/
void esptimer_init(uint64_t tps) {
    esp_timer_handle_t esp_timer_handle;
    esp_timer_create_args_t esp_timer_create_args = {
        .callback = esp_callback,
        .arg = NULL
    };
    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);

    esp_timer_start_periodic(esp_timer_handle, tps);
}