#ifndef ESP_OLED_H
#define ESP_OLED_H

#include "esp_err.h"

esp_err_t esp_oled_init(void);

void esp_oled_update(const char *text);


#endif