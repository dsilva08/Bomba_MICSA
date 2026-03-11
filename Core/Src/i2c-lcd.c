#include "i2c-lcd.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

#define SLAVE_ADDRESS_LCD 0x4E // Endereço 0x27 shiftado para a esquerda

void lcd_send_cmd (char cmd) {
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  // en=1, rs=0
	data_t[1] = data_u|0x08;  // en=0, rs=0
	data_t[2] = data_l|0x0C;  // en=1, rs=0
	data_t[3] = data_l|0x08;  // en=0, rs=0
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data) {
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  // en=1, rs=1
	data_t[1] = data_u|0x09;  // en=0, rs=1
	data_t[2] = data_l|0x0D;  // en=1, rs=1
	data_t[3] = data_l|0x09;  // en=0, rs=1
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *) data_t, 4, 100);
}

void lcd_init (void) {
	HAL_Delay(100); // estabilizar voltagem

	// 2. Sequência de Reset para Modo 4-bit
	lcd_send_cmd_nibble(0x30);
	HAL_Delay(5);
	lcd_send_cmd_nibble(0x30);
	HAL_Delay(1);
	lcd_send_cmd_nibble(0x30);
	HAL_Delay(10);
	lcd_send_cmd_nibble(0x20);  // define modo 4-bit
	HAL_Delay(10);

	// 3. Configurações agora usando a função padrão (que envia 2 nibbles)
	lcd_send_cmd(0x28); // 2 linhas, 5x8
	HAL_Delay(1);
	lcd_send_cmd(0x08); // Display off
	HAL_Delay(1);
	lcd_send_cmd(0x01); // Clear display
	HAL_Delay(5);
	lcd_send_cmd(0x06); // Entry mode
	HAL_Delay(1);
	lcd_send_cmd(0x0C); // Display on
}

void lcd_send_cmd_nibble(char cmd) {
    uint8_t data_t[2];
    data_t[0] = (cmd & 0xf0) | 0x0C; // EN=1, RS=0
    data_t[1] = (cmd & 0xf0) | 0x08; // EN=0, RS=0
    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, data_t, 2, 100);
}

void lcd_send_string (char *str) {
	while (*str) lcd_send_data (*str++);
}

void lcd_put_cur(int row, int col) {
    switch (row) {
        case 0: col |= 0x80; break;
        case 1: col |= 0xC0; break;
    }
    lcd_send_cmd (col);
}

void lcd_clear() {
	lcd_send_cmd (0x01);
	HAL_Delay(2);
}

void lcd_display_codigo(){
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("CODIGO: ");
}

void lcd_display_final_message(char* str){
	lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string(str);
	lcd_put_cur(1, 0);
	lcd_send_string("# = NOVO JOGO");
}

void lcd_display_tempo(int time_remaining){
    char buf[17];
    snprintf(buf, sizeof(buf), "Tempo: %d", time_remaining);
    lcd_put_cur(1, 0);
    lcd_send_string("                "); // 16 espaços
    lcd_put_cur(1, 0);
    lcd_send_string(buf);
}

void lcd_display_invalid_key_message(){
	lcd_put_cur(1,0);
	lcd_send_string("TECLA INVALIDA");
	HAL_Delay(1000);
}
