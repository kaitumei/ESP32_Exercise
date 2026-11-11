#ifndef __KEY_H__
#define __KEY_H__

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* BOOT --- GPIO0 */
#define BOOT_GPIO_PIN GPIO_NUM_0

/* 读取IO电平判断按键状态 */
#define BOOT_STATUS gpio_get_level(BOOT_GPIO_PIN)

/* 按键按下返回值，用做后续处理 */
enum KEY_PRES{
    KEY_NONE,
    BOOT_VAL
};

void key_init(void);
uint8_t key_scan(uint8_t mode);

#endif