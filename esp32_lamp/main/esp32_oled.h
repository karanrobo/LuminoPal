#ifndef ESP_OLED_H
#define ESP_OLED_H

#include "esp_err.h"
#include "esp32_lamp_api.h"

esp_err_t esp_oled_init(void);

void esp_oled_update(const char *text);

void esp_oled_update_task(LampTask task);

#endif