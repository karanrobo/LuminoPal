#include "esp32_pir.h"
 
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
 
#define PIR_GPIO 27
 
static const char *TAG = "pir";
 
void pir_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = 1ULL << PIR_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
 
    ESP_ERROR_CHECK(gpio_config(&config));
 
    ESP_LOGI(TAG, "PIR configured on GPIO%d", PIR_GPIO);
}
 
bool pir_motion_detected(void)
{
    return gpio_get_level(PIR_GPIO) == 1;
}