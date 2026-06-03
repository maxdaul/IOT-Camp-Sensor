#ifndef LED_H
#define LED_H

#include <stdint.h>

void led_init(void);

void led_set_color(uint8_t red, uint8_t green, uint8_t blue);

void led_set_red(void);
void led_set_green(void);
void led_set_blue(void);
void led_set_yellow(void);
void led_off(void);

#endif