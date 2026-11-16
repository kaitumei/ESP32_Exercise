#include "myledc.h"

/********************************************************
 * @brief       百分比占空比 --> LEDC计数值
 * @param       percent 百分比占空比,范围0-100
 * @param       bits LEDC计数值的位数，如10、12、16
 * @retval      LEDC计数值
 *********************************************************/
static uint32_t ledc_percent_to_duty(uint32_t percent, uint8_t bits){
    uint32_t max_duty = (1U << bits) - 1;
    return (max_duty * percent) / 100;
}
/*********************************************************
 * @brief       ledc初始化函数
 * @param       *ledc_config：LEDC配置结构体
 * @retval      无
 *********************************************************/
void ledc_init(ledc_config_t *ledc_config)
{
    ledc_config->duty = ledc_percent_to_duty(ledc_config->duty, ledc_config->duty_resolution);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = ledc_config->duty_resolution,
        .clk_cfg          = ledc_config->clk_cfg,
        .freq_hz          = ledc_config->freq_hz,
        .timer_num        = ledc_config->timer_num,
        .deconfigure      = false
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .intr_type        = LEDC_INTR_DISABLE,
        .channel          = ledc_config->channel,
        .duty             = ledc_config->duty,
        .gpio_num         = ledc_config->gpio_num,
        .hpoint           = 0,
        .timer_sel        = ledc_config->timer_num,
        .sleep_mode       = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD
    };
    ledc_channel_config(&ledc_channel);
}
/***********************************************************
 * @brief       LEDC PWM 设置占空比
 * @param       *ledc_config LEDC 配置结构体指针
 * @param       duty 占空比，范围 0-100
 * @retval      无
 ***********************************************************/
void ledc_pwm_set_duty(ledc_config_t *ledc_config, uint16_t duty)
{
    ledc_config->duty = ledc_percent_to_duty(duty, ledc_config->duty_resolution);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_config->channel, ledc_config->duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_config->channel);
}
