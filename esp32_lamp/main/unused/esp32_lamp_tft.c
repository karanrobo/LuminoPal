#include "nvs_flash.h"
#include "esp32_wifi.h"
#include "esp32_tft.h"
#include "esp32_home_wifi_mock.h"
#include "esp32_lamp_identity.h"
#include "esp32_lamp_api.h"

#include "esp32_oled.h"

#include "driver/uart.h"
#include "esp_mac.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());

    storage_init();

    /*
     * Create device identity
     */

    char device_id[32] = {0};

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

 

    // char ssid[33];
    // char pass[65];

    // if (load_wifi_credentials(ssid, pass))
    // {
    //     ESP_LOGI(TAG, "Loaded saved WiFi: %s", ssid);

    //     wifi_config_t sta_config = {0};

    //     strcpy((char *)sta_config.sta.ssid, ssid);
    //     strcpy((char *)sta_config.sta.password, pass);

    //     ESP_ERROR_CHECK(
    //         esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    //     ESP_ERROR_CHECK(esp_wifi_connect());
    // }
    // else
    // {
    //     ESP_LOGI(TAG, "No saved WiFi credentials");
            wifi_init_softap();
            start_webserver();
    // }

    // VIS test
    lvgl_port_cfg_t lvgl_cfg =
        ESP_LVGL_PORT_INIT_CONFIG();

    ESP_ERROR_CHECK(
        lvgl_port_init(&lvgl_cfg));

    ESP_ERROR_CHECK(
        esp_tft_init());

    ESP_ERROR_CHECK(
        esp_oled_init());

    while (!wifi_connected())
    {
        ESP_LOGI(
            TAG,
            "Waiting for home WiFi...");

        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(
        TAG,
        "Home WiFi ready");

    if (storage_is_paired())
    {

        ESP_LOGI(
            TAG,
            "Checking existing pairing...");

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

    /*
     * Pairing process
     */

    if (!storage_is_paired())
    {

        ESP_LOGI(
            TAG,
            "Lamp not paired");

        while (!api_register_lamp(device_id))
        {

            ESP_LOGW(
                TAG,
                "Registration failed");

            vTaskDelay(
                pdMS_TO_TICKS(5000));
        }

        while (!storage_is_paired())
        {

            ESP_LOGI(
                TAG,
                "Waiting for user pairing");

            bool paired =
                api_check_pair_status(
                    device_id);

            if (paired)
            {

                ESP_LOGI(
                    TAG,
                    "Pair successful");

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

            esp_tft_update(
                "Pair Lamp",
                pair_code);

            vTaskDelay(
                pdMS_TO_TICKS(5000));
        }
    }

    ESP_LOGI(
        TAG,
        "Lamp ready");

    esp_oled_update(
        "Connected");

    esp_tft_update(
        "Lamp Ready",
        "Connected");
    esp_oled_update(
        "Connected");
    vTaskDelay(
        pdMS_TO_TICKS(1000));

   LampTask task = {0};

int refresh_counter = 0;

while (1)
{
    /*
     * Refresh task from Django every 10 seconds.
     */
    if (refresh_counter == 0)
    {
        task = api_get_tasks(device_id);
    }

    if (!task.paired)
    {
        esp_tft_update(
            "Pair Lamp",
            "Not paired"
        );

        esp_oled_update(
            "PAIR"
        );
    }
    else if (!task.has_task)
    {
        esp_tft_update(
            "No Tasks",
            "All done!"
        );

        esp_oled_update(
            "No deadline"
        );
    }
    else
    {
        esp_tft_show_task(
            task.title,
            task.description,
            lv_palette_main(
                LV_PALETTE_ORANGE
            )
        );

        if (task.has_timer)
        {
            time_t now;

            time(&now);

            int remaining =
                (int)(task.timer_end - now);

            if (remaining < 0)
            {
                remaining = 0;
            }

            int hours =
                remaining / 3600;

            int minutes =
                (remaining % 3600) / 60;

            int seconds =
                remaining % 60;

            char buffer[20];

            if (hours > 0)
            {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%02d:%02d:%02d",
                    hours,
                    minutes,
                    seconds
                );
            }
            else
            {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%02d:%02d",
                    minutes,
                    seconds
                );
            }

            esp_oled_update(buffer);
        }
        else if (task.has_deadline)
        {
            time_t now;

            time(&now);

            int remaining =
                (int)(task.deadline - now);

            if (remaining < 0)
            {
                remaining = 0;
            }

            int days =
                remaining / 86400;

            int hours =
                (remaining % 86400) / 3600;

            char buffer[20];

            snprintf(
                buffer,
                sizeof(buffer),
                "%dd %02dh",
                days,
                hours
            );

            esp_oled_update(buffer);
        }
        else
        {
            esp_oled_update(
                "No deadline"
            );
        }
    }

    refresh_counter++;

    if (refresh_counter >= 1)
    {
        refresh_counter = 0;
    }

    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
}