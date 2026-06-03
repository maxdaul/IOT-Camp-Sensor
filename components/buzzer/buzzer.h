#ifndef BUZZER_H
#define BUZZER_H

#include <stdbool.h>
#include <stdint.h> 

void buzzer_init(void);

void buzzer_on(void);

void buzzer_off(void);

void buzzer_beep(uint32_t duration_ms);

#endif