#ifndef __LCD_I2C_H__
#define __LCD_I2C_H__

#include "stm32f4xx_hal.h"
#include <stdio.h>

#define SLAVE_ADDRESS_LCD (0x27 << 1) // change this according to our setup

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_send_int(int num);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_clear(void);
void lcd_put_cursor(uint8_t row, uint8_t col);
#ifdef __cplusplus
}
#endif

#endif
