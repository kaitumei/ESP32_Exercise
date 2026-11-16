#ifndef __UART_H__
#define __UART_H__

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

// 定义串口编号和引脚

#define UART_NUM UART_NUM_0
#define UART_TX_PIN GPIO_NUM_43
#define UART_RX_PIN GPIO_NUM_44
#define BUFF_SIZE 1024


void uart_echo_process(void);
void uart_init(uint32_t buadrate);

#endif