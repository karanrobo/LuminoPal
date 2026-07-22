#include "nvs_flash.h"
#include "esp32_wifi.h"
#include "esp32_tft.h"
#include "esp32_home_wifi_mock.h"
#include "esp32_finger_print.h"

#include "driver/uart.h"
#include "esp_mac.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

#define ENROLL_TEST 1

static uint8_t esp_chip_id[6];
static char mac_address[20];

static char default_address[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static char default_password[4] = {0x00, 0x00, 0x00, 0x00};


void fingerprint_enroll(uint16_t id)
{
    char buffer1[1] = {0x01};
    char buffer2[1] = {0x02};

    char page_id[2];

    page_id[0] = (id >> 8) & 0xFF;
    page_id[1] = id & 0xFF;


    ESP_LOGI(TAG, "Place finger");


    // Capture first fingerprint
    if (GenImg(default_address) != 0x00)
    {
        ESP_LOGE(TAG, "First fingerprint capture failed");
        return;
    }


    // Convert to CharBuffer1
    if (Img2Tz(default_address, buffer1) != 0x00)
    {
        ESP_LOGE(TAG, "First template conversion failed");
        return;
    }


    ESP_LOGI(TAG, "Remove finger");


    // Wait until finger removed
    while (GenImg(default_address) == 0x00)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
    }


    ESP_LOGI(TAG, "Place same finger again");


    // Capture second fingerprint
    if (GenImg(default_address) != 0x00)
    {
        ESP_LOGE(TAG, "Second fingerprint capture failed");
        return;
    }


    // Convert to CharBuffer2
    if (Img2Tz(default_address, buffer2) != 0x00)
    {
        ESP_LOGE(TAG, "Second template conversion failed");
        return;
    }


    // Merge templates
    if (RegModel(default_address) != 0x00)
    {
        ESP_LOGE(TAG, "Model creation failed");
        return;
    }


    // Store fingerprint
    if (Store(default_address, buffer1, page_id) != 0x00)
    {
        ESP_LOGE(TAG, "Fingerprint storage failed");
        return;
    }


    ESP_LOGI(TAG, "Fingerprint enrolled successfully. ID=%d", id);
}



void fingerprint_search()
{
    char buffer_id[1] = {0x01};
    char start_page[2] = {0x00, 0x00};
    char page_num[2] = {0x00, 0xFF};


   ESP_LOGI(TAG, "Remove finger");
vTaskDelay(pdMS_TO_TICKS(3000));

ESP_LOGI(TAG, "Place same finger again");


    if (Img2Tz(default_address, buffer_id) != 0x00)
    {
        ESP_LOGE(TAG, "Template conversion failed");
        return;
    }


    uint8_t result = Search(
        default_address,
        buffer_id,
        start_page,
        page_num
    );


   ESP_LOGI(TAG, "Search response code: 0x%02X", result);

if(result == 0x00)
{
    ESP_LOGI(TAG, "Search command successful");
}
else
{
    ESP_LOGE(TAG, "Search failed");
}
}



void app_main(void)
{
    ESP_ERROR_CHECK(
        nvs_flash_init()
    );

    // finger test
    r307_init();
    ESP_ERROR_CHECK(
        esp_efuse_mac_get_default(esp_chip_id)
    );

    sprintf(mac_address,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            esp_chip_id[0],
            esp_chip_id[1],
            esp_chip_id[2],
            esp_chip_id[3],
            esp_chip_id[4],
            esp_chip_id[5]);


    ESP_LOGI(TAG, "MAC Address: %s", mac_address);



    uint8_t confirmation_code =
        VfyPwd(default_address, default_password);



    if(confirmation_code != 0x00)
    {
        ESP_LOGE(TAG,
                 "Fingerprint module failed. Code: 0x%02X",
                 confirmation_code);

        return;
    }


    ESP_LOGI(TAG, "R307 fingerprint module detected");


    // Optional diagnostic
    ReadSysPara(default_address);



#if ENROLL_TEST

    fingerprint_enroll(1);

#endif



    while(1)
    {
        fingerprint_search();

        vTaskDelay(
            pdMS_TO_TICKS(3000)
        );
    }

    // drawText("Hello World");


    //  while(1)
    // {


    //     vTaskDelay(
    //         pdMS_TO_TICKS(3000)
    //     );
    // }

}