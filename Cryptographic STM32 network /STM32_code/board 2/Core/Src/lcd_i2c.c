#include "lcd_i2c.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>


extern I2C_HandleTypeDef hi2c1; // or whatever I2C you used


void lcd_send_cmd(char cmd) {
    // send high nibble then low nibble + toggling enable pin  // RS=0
	  char data_u, data_l;
	  data_u = (cmd&0xf0);
	  data_l = ((cmd<<4)&0xf0);
	  uint8_t data_t[4];
	  data_t[0] = data_u|0x0C;  //en=1, rs=0 -> bxxxx1100
	  data_t[1] = data_u|0x08;  //en=0, rs=0 -> bxxxx1000
	  data_t[2] = data_l|0x0C;  //en=1, rs=0 -> bxxxx1100
	  data_t[3] = data_l|0x08;  //en=0, rs=0 -> bxxxx1000
	  HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data(char data) {
    // same as lcd_send_cmd but with RS=1
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1 -> bxxxx1101
	data_t[1] = data_u|0x09;  //en=0, rs=1 -> bxxxx1001
	data_t[2] = data_l|0x0D;  //en=1, rs=1 -> bxxxx1101
	data_t[3] = data_l|0x09;  //en=0, rs=1 -> bxxxx1001
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_init(void) {
    // initialize LCD in 4-bit mode, send config commands
	  HAL_Delay(50);  // wait for >40ms
	  lcd_send_cmd (0x30);
	  HAL_Delay(5);  // wait for >4.1ms
	  lcd_send_cmd (0x30);
	  HAL_Delay(1);  // wait for >100us
	  lcd_send_cmd (0x30);
	  HAL_Delay(10);
	  lcd_send_cmd (0x20);  // 4bit mode
	  HAL_Delay(10);

	  // display initialisation
	  lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	  HAL_Delay(1);
	  lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	  HAL_Delay(1);
	  lcd_send_cmd (0x01);  // clear display
	  HAL_Delay(2);
	  lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	  HAL_Delay(1);
	  lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string(char *str) {
    while (*str) lcd_send_data(*str++);
}

void lcd_send_int(int num)
{

	  char numChar[5];
	  sprintf(numChar, "%d", num);
	  lcd_send_string (numChar);
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    HAL_Delay(2);
}

void lcd_put_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_send_cmd(addr);
}
