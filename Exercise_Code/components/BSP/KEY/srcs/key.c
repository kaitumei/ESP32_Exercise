/* key相关代码 */
#include "key.h"

void key_init(void)
{
    gpio_config_t gpio_init_conf = {
        .pin_bit_mask = 1ull << BOOT_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,        // 设置GPIO为输入 
        .pull_up_en = GPIO_PULLUP_ENABLE,      // 启用上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE        // 禁用GPIO中断
    };
    gpio_config(&gpio_init_conf);
}

uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_state = 1;  // 按键松开标志
    uint8_t key_val = 0;

    if(mode)
    {
        key_state = 1;
    }

    if(key_state && (BOOT_STATUS == 0))
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        key_state = 0;

        if(BOOT_STATUS == 0)
        {
            key_val = BOOT_VAL;
        }
    }else if(BOOT_STATUS == 1)
    {
        key_state = 1;
        key_val = KEY_NONE;
    }
    
    return key_val;

}
