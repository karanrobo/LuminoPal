#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp32_wifi.h"
#include "esp32_lamp_identity.h"
#include "esp32_lamp_api.h"

#include "esp32_oled.h"
#include "esp32_ldr.h"

#include "esp32_pir.h"

#include "driver/uart.h"

#include "esp_mac.h"
#include "esp_log.h"

#include "esp32_tft.h"

#include "esp_err.h"

#include "esp32_ldr.h"
#include "esp32_pir.h"
#include "esp32_buttons.h"
#include "esp32_buzzer.h"

#include "esp_adc/adc_oneshot.h"

#include "driver/ledc.h"

#define LDR_ADC_GPIO 36
#define LDR_ADC_UNIT ADC_UNIT_1
#define LDR_ADC_CHANNEL ADC_CHANNEL_0
#define LDR_ADC_ATTEN ADC_ATTEN_DB_12

/*
 * LED PWM output
 */
#define LED_PWM_GPIO 26
#define LED_PWM_FREQUENCY 20000
#define LED_PWM_RESOLUTION LEDC_TIMER_8_BIT
#define LED_PWM_MAX_DUTY 255

#define UPDATE_PERIOD_MS 10

#define PIR_GPIO GPIO_NUM_27

static const char *TAG = "lighting";

static void wifi_task(void *pvParameters);
static void hardware_task(void *pvParameters);
static void timer_task(void *pvParameters);

static char device_id[32];


LampTask task = {0};

bool pause = false;

bool light = false;

bool setup = false;

static bool manual_override = false;

uint32_t inactivity_timer = 0; // milliseconds
uint32_t inactivity_timeout = 15000;

static adc_oneshot_unit_handle_t adc_handle;
static LightingController lighting_controller;

static void configure_adc(void);

static void configure_pwm(void);

static void set_pwm_duty(uint16_t duty);

void app_main(void)
{

    configure_adc();
    configure_pwm();

    pir_init();
    buttons_init();
    buzzer_init();

    Lighting_Init(
        &lighting_controller,
        300,
        3500,
        20,
        255,
        0.20f,
        true);

    xTaskCreate(
        hardware_task,
        "hardware_task",
        4096,
        NULL,
        5,
        NULL);

    ESP_ERROR_CHECK(nvs_flash_init());

    storage_init();

    /*
     * Create device identity
     */

    storage_get_device_id(
        device_id,
        sizeof(device_id) - 1);

    /*
     * First boot
     */
    if (strlen(device_id) == 0)
    {

        create_device_id(
            device_id);

        storage_save_device_id(
            device_id);
    }

    ESP_LOGI(
        TAG,
        "Device ID: %s",
        device_id);

    // VIS test
    lvgl_port_cfg_t lvgl_cfg =
        ESP_LVGL_PORT_INIT_CONFIG();

    ESP_ERROR_CHECK(
        lvgl_port_init(&lvgl_cfg));

    ESP_ERROR_CHECK(
        esp_oled_init());

    // char ssid[33];
    // char pass[65];

    // if (load_wifi_credentials(ssid, pass))
    // {
    //     ESP_LOGI(TAG, "Loaded saved WiFi: %s", ssid);
    //      esp_oled_update("Loading previous wifi");
    // wifi_config_t sta_config = {0};

    // strcpy((char *)sta_config.sta.ssid, ssid);
    // strcpy((char *)sta_config.sta.password, pass);

    // ESP_ERROR_CHECK(
    //     esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    // ESP_ERROR_CHECK(esp_wifi_connect());

    // else
    // {
    ESP_LOGI(TAG, "No saved WiFi credentials");
    wifi_init_softap();
    start_webserver();
    // }

    while (!wifi_connected())
    {
        ESP_LOGI(
            TAG,
            "Waiting for home WiFi...");
        esp_oled_update(
            "Connect Phone!");

        vTaskDelay(pdMS_TO_TICKS(5000));

        esp_oled_update(
            "Goto  \n http://192.168.4.1!");

        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    ESP_LOGI(
        TAG,
        "WiFi ready");
    esp_oled_update(
        "Wifi Connected");

    if (storage_is_paired())
    {

        ESP_LOGI(
            TAG,
            "Checking existing pairing...");
        esp_oled_update(
            "Checking if you paired already.");

        bool valid =
            api_check_pair_status(
                device_id);

        if (!valid)
        {

            ESP_LOGW(
                TAG,
                "Pairing invalid");

            storage_set_paired(
                false);

            storage_clear_token();
        }
    }
    else
    {
        esp_oled_update(
            "New pairing required");
    }

    /*
     * Pairing process
     */

    if (!storage_is_paired())
    {

        ESP_LOGI(
            TAG,
            "Lamp not paired");

        esp_oled_update(
            "Lamp not paired");

        while (!api_register_lamp(device_id))
        {

            ESP_LOGW(
                TAG,
                "Registration failed");
            esp_oled_update(
                "Registration failed. \n Trying again...");

            vTaskDelay(
                pdMS_TO_TICKS(5000));
        }

        while (!storage_is_paired())
        {

            ESP_LOGI(
                TAG,
                "Waiting for user pairing");
            esp_oled_update(
                "Waiting for user pairing");

            bool paired =
                api_check_pair_status(
                    device_id);

            if (paired)
            {

                ESP_LOGI(
                    TAG,
                    "Pair successful");
                esp_oled_update(
                    "Paired successfully!");

                break;
            }

            char pair_code[16] = {0};

            storage_get_pair_code(
                pair_code,
                sizeof(pair_code));

            ESP_LOGI(
                TAG,
                "Pair code: %s",
                pair_code);

            esp_oled_update(
                pair_code);

            vTaskDelay(
                pdMS_TO_TICKS(5000));
        }
    }

    ESP_LOGI(
        TAG,
        "Lamp ready");
   
    vTaskDelay(
        pdMS_TO_TICKS(1000));

    xTaskCreate(
        wifi_task,
        "wifi_task",
        8192,
        NULL,
        5,
        NULL);
    xTaskCreate(
    timer_task,
    "timer_task",
    2048,
    NULL,
    5,
    NULL
);

    setup = true;

    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

static void wifi_task(void *pvParameters)
{
    int counter = 5;
    while (1)
    {
        /*
         * Refresh task from Django every 1 seconds.
         */
        if (counter == 0) {
            task = api_get_tasks(device_id);
            counter = 5;
        }

        if (pause)
        {
            if (api_toggle_timer(device_id))
            {
                ESP_LOGI(
                    TAG,
                    "TASK: %d",
                    pause);
            }
            else
            {
                ESP_LOGI(TAG, "ERROR");
            }
        }

        esp_oled_update_task(task);
        counter--;
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}

static void timer_task(void *pvParameters)
{
    while(1)
    {
        if(task.timer_running)
        {
            if(task.remaining_seconds > 0)
            {
                task.remaining_seconds--;
            }

            if(task.remaining_seconds == 0)
            {
                task.timer_running = false;

                buzzer_tone(1000, 500);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void hardware_task(void *pvParameters)
{
    bool motion = false;

    bool manual_override = false;
    uint32_t manual_override_timer = 0;

    while (1)
    {
        bool lb = light_button();
        motion = pir_motion_detected();

        /*-----------------------------
         * Light Button (Highest Priority)
         *----------------------------*/
        if (lb)
        {
            ESP_LOGI(TAG, "Light button");
            
            buzzer_tone(1000, 100);   // 1000 Hz for 100 ms

            light = !light;

            if (light)
            {
                // User manually turned ON
                manual_override = true;
                manual_override_timer = 0;
                inactivity_timer = 0;
            }
            else
            {
                // User manually turned OFF
                manual_override = true;
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        /*-----------------------------
         * PIR Logic
         *----------------------------*/
        if (setup)
        {
            if (manual_override)
            {
                if (light)
                {
                    // Keep light ON during manual override period
                    manual_override_timer += 100;

                    if (manual_override_timer >= inactivity_timeout)
                    {
                        ESP_LOGI(TAG, "Manual override expired. PIR active.");

                        manual_override = false;
                        manual_override_timer = 0;
                        inactivity_timer = 0;
                    }
                }

                // If manually OFF, do nothing.
                // PIR remains disabled until user presses button again.
            }
            else
            {
                // PIR has control
                if (motion)
                {
                    inactivity_timer = 0;
                    light = true;
                }
                else
                {
                    inactivity_timer += 100;

                    if (inactivity_timer >= inactivity_timeout)
                    {
                        light = false;
                    }
                }
            }
        }

        /*-----------------------------
         * Pause Button
         *----------------------------*/
        if (pause_start_button())
        {
            ESP_LOGI(TAG, "Start button");

            pause = !pause;

            buzzer_tone(500, 100);   // 1000 Hz for 100 ms

            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        /*-----------------------------
         * Lighting Control
         *----------------------------*/
        int adc_raw;

        if (adc_oneshot_read(adc_handle,
                             LDR_ADC_CHANNEL,
                             &adc_raw) == ESP_OK)
        {
            uint16_t pwm = 0;

            if (light)
            {
                pwm = Lighting_Update(
                    &lighting_controller,
                    (uint16_t)adc_raw);
            }

            set_pwm_duty(pwm);
        }
        buzzer_set(false);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
static void configure_adc(void)
{
    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = LDR_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE};

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &unit_config,
            &adc_handle));

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = LDR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT};

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            LDR_ADC_CHANNEL,
            &channel_config));

    ESP_LOGI(
        TAG,
        "ADC configured: GPIO%d, ADC1 channel %d",
        LDR_ADC_GPIO,
        LDR_ADC_CHANNEL);
}

static void configure_pwm(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LED_PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LED_PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = LED_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config));

    ESP_LOGI(
        TAG,
        "PWM configured: GPIO%d, frequency=%d Hz",
        LED_PWM_GPIO,
        LED_PWM_FREQUENCY);
}

static void set_pwm_duty(uint16_t duty)
{
    if (duty > LED_PWM_MAX_DUTY)
    {
        duty = LED_PWM_MAX_DUTY;
    }

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            duty));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0));
}
