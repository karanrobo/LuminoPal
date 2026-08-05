#include "esp32_buzzer.h"

#define BUZZER_TIMER       LEDC_TIMER_2
#define BUZZER_CHANNEL     LEDC_CHANNEL_2
#define BUZZER_MODE        LEDC_LOW_SPEED_MODE

void buzzer_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_MODE,
        .timer_num = BUZZER_TIMER,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = BUZZER_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&channel_config);
}


void buzzer_set(bool on)
{
    if (on)
    {
        ledc_set_duty(
            BUZZER_MODE,
            BUZZER_CHANNEL,
            512);   // 50% duty

        ledc_update_duty(
            BUZZER_MODE,
            BUZZER_CHANNEL);
    }
    else
    {
        ledc_set_duty(
            BUZZER_MODE,
            BUZZER_CHANNEL,
            0);

        ledc_update_duty(
            BUZZER_MODE,
            BUZZER_CHANNEL);
    }
}
void buzzer_tone(uint32_t frequency, uint32_t duration_ms)
{
    ledc_set_freq(
        BUZZER_MODE,
        BUZZER_TIMER,
        frequency);

    ledc_set_duty(
        BUZZER_MODE,
        BUZZER_CHANNEL,
        512);      // 50% duty

    ledc_update_duty(
        BUZZER_MODE,
        BUZZER_CHANNEL);

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    ledc_stop(
        BUZZER_MODE,
        BUZZER_CHANNEL,
        0);        // Completely stop PWM
}


