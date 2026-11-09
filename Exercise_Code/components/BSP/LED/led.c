/* LED相关代码 */
#include "led.h"

/**********************************************
 * @brief LED小灯初始化函数
 * @param  无
 * @return  无
 ***********************************************/
void led_init(void)
{
    gpio_config_t gpio_init_conf = {0};
    gpio_init_conf.pin_bit_mask = 1ull << LED_GPIO_PIN;
    gpio_init_conf.mode = GPIO_MODE_INPUT_OUTPUT;        // 设置推挽输出 + 输入 
    gpio_init_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // 启用上拉电阻
    gpio_init_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_init_conf.intr_type = GPIO_INTR_DISABLE;        // 禁用GPIO中断
    gpio_config(&gpio_init_conf);

    LED(1);

}