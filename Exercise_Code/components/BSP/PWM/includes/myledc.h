/* 控制LED的平滑亮灭 */
#ifndef __MYLEDC_H__
#define __MYLEDC_H__

#include "driver/gpio.h"
#include "driver/ledc.h"

#define LEDC_PWM_TIMER          LEDC_TIMER_0
#define LEDC_PWM_CH0_GPIO       GPIO_NUM_1
#define LEDC_PWM_CH0_CHANNEL    LEDC_CHANNEL_0


typedef struct myledc
{
    ledc_clk_cfg_t      clk_cfg;            // 时钟源
    ledc_timer_t        timer_num;          // LEDC定时器编号
    uint32_t            freq_hz;            // PWM输出频率
    ledc_timer_bit_t    duty_resolution;    // 占空比分辨率
    ledc_channel_t      channel;            // LEDC通道号
    uint32_t            duty;               // 占空比原始值
    int                 gpio_num;           // 输出PWM的GPIO
} ledc_config_t;


void ledc_init(ledc_config_t *ledc_config);
void ledc_pwm_set_duty(ledc_config_t *ledc_config, uint16_t duty);

#endif