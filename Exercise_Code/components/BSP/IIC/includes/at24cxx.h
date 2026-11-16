#ifndef __24CXX_H
#define __24CXX_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "myiic.h"
#include <string.h>
#include "esp_log.h"

/* 24C02 7位设备地址 */
#define AT_ADDR     (0x50)

#define AT24C01     127
#define AT24C02     255
#define AT24C04     511
#define AT24C08     1023
#define AT24C16     2047
#define AT24C32     4095
#define AT24C64     8191
#define AT24C128    16383
#define AT24C256    32767

/* 开发板使用的是24c02，所以定义EE_TYPE为AT24C02 */
#define EE_TYPE     AT24C02


void at24cxx_init(my_iic_obj_t self);
uint8_t at24cxx_check(void);
uint8_t at24cxx_read_one_byte(uint16_t addr);
void at24cxx_write_one_byte(uint16_t addr, uint8_t data);
void at24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen);
void at24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen);

#endif
