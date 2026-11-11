#ifndef __EXIT_H__
#define __EXIT_H__

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "led.h"
/* BOOT --- GPIO0 */
#define BOOT_INT_GPIO_PIN GPIO_NUM_0
#define BOOT_DOWN gpio_get_level(GPIO_NUM_0)

void exit_init(void);

#endif