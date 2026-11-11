/* 控制RED灯亮灭逻辑，包括亮灭逻辑，翻转逻辑。  */
#ifndef __LED_H__
#define __LED_H__

#include "driver/gpio.h"

/* LED --- GPIO1 */
#define LED_GPIO_PIN GPIO_NUM_1

enum GPIO_OUTPUT_LEVEL
{
    PIN_RESET,      // 0
    PIN_SET         // 1
};

/* LED亮灭逻辑 */
#define LED(X) do \
{X ? \
    gpio_set_level(LED_GPIO_PIN, PIN_SET): \
    gpio_set_level(LED_GPIO_PIN, PIN_RESET); \
} while (0);

/* LED翻转逻辑 */
#define LED_TOGGLE() do \
{ \
    gpio_set_level(LED_GPIO_PIN, !gpio_get_level(LED_GPIO_PIN)); \
} while(0);

void led_init(void);

#endif