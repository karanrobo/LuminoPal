#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "nvs.h"
#include "nvs_flash.h"

#ifndef ESP_WIFI_H
#define ESP_WIFI_H

#define ESP_WIFI_SSID "LuminoPal"
#define ESP_WIFI_PASS "test_esp"
#define ESP_WIFI_CHANNEL 1
#define MAX_STA_CONN 2


static volatile bool g_wifi_connected = false;



void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);

void wifi_init_softap();

void start_webserver();

void save_wifi_credentials(const char *ssid, const char *password);

bool load_wifi_credentials(
    char *ssid,
    char *pass
);

bool wifi_connected(void);

#endif
