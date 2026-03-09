/*
 * led.h
 *
 *  Created on: 08/03/2026
 *      Author: diosi
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include "stm32f4xx_hal.h"

void RGB_init();
void RGB_setColor(uint16_t r, uint16_t g, uint16_t b);
void RGB_setGreen();
void RGB_setRed();
void RGB_defuseBlink();
void RGB_off();

#endif
