#include "esp32_buttons.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define DEBOUNCE_MS    50

static bool last_start_reading = true;
static bool stable_start_state = true;
static int64_t last_start_change = 0;

static bool last_reset_reading = true;
static bool stable_reset_state = true;
static int64_t last_reset_change = 0;

void buttons_init(void)
{
    gpio_config_t config = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =
            (1ULL << START_BUTTON_GPIO) |
            (1ULL << RESET_BUTTON_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&config);
}

static bool button_pressed(gpio_num_t gpio,
                           bool *last_reading,
                           bool *stable_state,
                           int64_t *last_change)
{
    bool reading = gpio_get_level(gpio);
    int64_t now = esp_timer_get_time() / 1000;

    if (reading != *last_reading) {
        *last_change = now;
    }

    if ((now - *last_change) > DEBOUNCE_MS) {
        if (reading != *stable_state) {
            *stable_state = reading;

            if (*stable_state == false) {
                *last_reading = reading;
                return true;
            }
        }
    }

    *last_reading = reading;
    return false;
}

bool pause_start_button(void)
{
    return button_pressed(
        START_BUTTON_GPIO,
        &last_start_reading,
        &stable_start_state,
        &last_start_change);
}

bool light_button(void)
{
    return button_pressed(
        RESET_BUTTON_GPIO,
        &last_reset_reading,
        &stable_reset_state,
        &last_reset_change);
}