/*
 * led.c
 *
 *  Created on: 08/03/2026
 *      Author: diosi
 */


#include "led.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"
#include "tim.h"

extern TIM_HandleTypeDef htim3; // verde e azul
extern TIM_HandleTypeDef htim4; // vermelho

#define BUZZER_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_7

void RGB_init()
{
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // vermelho
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // verde
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // azul
}

void RGB_setColor(uint16_t r, uint16_t g, uint16_t b)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, r);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, g);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, b);
}

void RGB_setGreen()
{
    RGB_setColor(0,1000,0);
}

void RGB_setRed()
{
    RGB_setColor(1000,0,0);
}

void RGB_defuseBlink(int time_remaining)
{
	static int blink_count = 0;
	static uint32_t last_blink = 0;
	static int state = 0;

	int interval = (time_remaining*1000) / 50;

	if (interval < 100)
		interval = 100;

	if (blink_count == 0 || HAL_GetTick() - last_blink >= interval){
		if (state){
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
			RGB_setRed();
			HAL_Delay(50);
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
			blink_count++;
		} else{
			RGB_off();
		}

		state = !state;
		last_blink = HAL_GetTick();
	}
}

void RGB_off()
{
    RGB_setColor(0,0,0);
}
