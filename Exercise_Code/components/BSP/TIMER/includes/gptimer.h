#ifndef __GPTIMER_H__
#define __GPTIMER_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "led.h"

void gptimer_init(void);

#endif