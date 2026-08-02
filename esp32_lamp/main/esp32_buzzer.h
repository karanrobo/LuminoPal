#ifndef BUZZER_H
#define BUZZER_H

#include <stdbool.h>
#include "driver/ledc.h"

#define BUZZER_GPIO GPIO_NUM_25

void buzzer_init(void);
void buzzer_set(bool on);
void buzzer_tone(uint32_t frequency, uint32_t duty);

#endif