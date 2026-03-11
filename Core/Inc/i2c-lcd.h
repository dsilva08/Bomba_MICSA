/*
 * i2c-lcd.h
 *
 *  Created on: 05/03/2026
 *      Author: diosi
 */

#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include "stm32f4xx_hal.h"

void lcd_init();
void lcd_send_cmd(char cmd);
void lcd_send_cmd_nibble(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_put_cur(int row, int col);
void lcd_clear();
void lcd_display_codigo();
void lcd_display_final_message();
void lcd_display_tempo(int time_remaining);
void lcd_display_invalid_key_message();

#endif /* I2C_LCD_H_ */
