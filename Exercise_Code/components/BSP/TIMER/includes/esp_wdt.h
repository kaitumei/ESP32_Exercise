#ifndef __ESP_WDT__
#define __ESP_WDT__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "led.h"
#include "key.h"

void esp_wdt_init(void);

#endif