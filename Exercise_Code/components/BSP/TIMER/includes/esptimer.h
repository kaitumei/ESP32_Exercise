#ifndef __ESPTIMER_H__
#define __ESPTIMER_H__

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "led.h"

void esptimer_init(uint64_t tps);

#endif