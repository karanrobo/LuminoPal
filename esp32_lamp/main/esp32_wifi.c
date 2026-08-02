#include "esp32_wifi.h"
#include "esp_http_server.h"
#include "esp_http_client.h"

const size_t response_size = 1024;

static const char *TAG = "Basic soft-AP";
static const char html_page[] =
"<!DOCTYPE html>\n"
"<html>\n"
"<script>\n"
"function scanWifi() {\n"
"    fetch('/scan')\n"
"    .then(response => response.json())\n"
"    .then(networks => {\n"
"        let ssidBox = document.getElementById('ssid');\n"
"        \n"
"        if (networks.length > 0) {\n"
"            ssidBox.value = networks[0];\n"
"        }\n"
"    });\n"
"}\n"
"</script>\n"
"<body>\n"
"<h2>WiFi Setup</h2>\n"
"<form method=\"POST\" action=\"/connect\" enctype=\"text/plain\">\n"
"SSID:<br>\n"
"<input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"WiFi name\">\n"
"<button type=\"button\" onclick=\"scanWifi()\">Scan</button>\n"
"<br><br>\n"
"Password:<br>\n"
"<input type=\"password\" name=\"password\" placeholder=\"Password\"><br><br>\n"
"<input type=\"submit\" value=\"Connect\">\n"
"</form>\n"
"</body>\n"
"</html>";


void wifi_event_handler(void *arg,
                        esp_event_base_t event_base,
                        int32_t event_id,
                        void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "STA connected");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            wifi_event_sta_disconnected_t *event =
                (wifi_event_sta_disconnected_t *)event_data;

            ESP_LOGI(TAG,
                     "STA disconnected, reason: %d",
                     event->reason);

            break;
        }

        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "AP started");
            break;
        }
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP)
    {
        g_wifi_connected = true;

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,
                 "Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));
    }
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t connect_post_handler(httpd_req_t *req)
{
    char buf[256];

    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);

    if (len <= 0)
        return ESP_FAIL;

    buf[len] = '\0';

    printf("POST data: %s\n", buf);

    ESP_LOGI(TAG, "Received: %s", buf);

    char ssid[33] = {0};
    char password[65] = {0};
    /*
     * Parse:
     * ssid=mywifi&password=mypass
     */

    char *ssid_ptr =
        strstr(buf, "ssid=");

    char *pass_ptr =
        strstr(buf, "password=");

    if (ssid_ptr && pass_ptr)
    {

        ssid_ptr += 5;

        int ssid_len =
            pass_ptr - ssid_ptr;

        memcpy(
            ssid,
            ssid_ptr,
            ssid_len);

        strcpy(
            password,
            pass_ptr + 9);
        password[strcspn(password, "\r\n")] = '\0';
        ssid[strcspn(ssid, "\r\n")] = '\0';

        ESP_LOGI(TAG, "SSID:%s PASS:%s", ssid, password);

        printf("works: %s %s", ssid, password);

        /*
         * Configure STA
         */
        wifi_config_t sta_config = {0};

        strcpy(
            (char *)sta_config.sta.ssid,
            ssid);

        strcpy(
            (char *)sta_config.sta.password,
            password);

        ESP_ERROR_CHECK(
            esp_wifi_set_config(
                WIFI_IF_STA,
                &sta_config));

        vTaskDelay(pdMS_TO_TICKS(100));

        ESP_ERROR_CHECK(
            esp_wifi_connect());

        save_wifi_credentials(
            ssid,
            password);

        httpd_resp_sendstr(
            req,
            "Connecting...");
    }

    return ESP_OK;
}

static esp_err_t scan_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Scan requested");
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false};

    ESP_ERROR_CHECK(
        esp_wifi_scan_start(&scan_config, false));

    ESP_LOGI(TAG, "Scan finished");
    uint16_t ap_num = 20;

    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * 20);
    char *response = malloc(response_size);

    ESP_ERROR_CHECK(
        esp_wifi_scan_get_ap_records(
            &ap_num,
            ap_records));

    
    if (!ap_records || !response)
    {
        free(ap_records);
        free(response);
        return ESP_ERR_NO_MEM;
    }

    int offset = 0;

    offset += snprintf(
        response + offset,
        response_size - offset,
        "[");

    for (int i = 0; i < ap_num; i++)
    {
        offset += snprintf(
            response + offset,
            response_size - offset,
            "\"%s\"%s",
            ap_records[i].ssid,
            (i == ap_num - 1) ? "" : ",");
    }

    snprintf(
        response + offset,
        response_size - offset,
        "]");

    httpd_resp_set_type(
        req,
        "application/json");

    httpd_resp_send(
        req,
        response,
        HTTPD_RESP_USE_STRLEN);

    free(ap_records);
    free(response);
    return ESP_OK;
}

httpd_handle_t server = NULL;

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_start(&server, &config);

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler};

    httpd_register_uri_handler(server, &root);


    httpd_uri_t scan = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = scan_get_handler};

    httpd_register_uri_handler(server, &scan);

    httpd_uri_t connect = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_post_handler};

    httpd_register_uri_handler(server, &connect);
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .required = true,
            },
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_APSTA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_AP,
            &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_LOGI(TAG,
             "WiFi AP started. SSID:%s password:%s",
             ESP_WIFI_SSID,
             ESP_WIFI_PASS);
}


bool wifi_connected(void)
{
    return g_wifi_connected;
}

void save_wifi_credentials(
    const char *ssid,
    const char *password)
{
    nvs_handle_t handle;

    nvs_open(
        "wifi",
        NVS_READWRITE,
        &handle);

    nvs_set_str(
        handle,
        "ssid",
        ssid);

    nvs_set_str(
        handle,
        "pass",
        password);

    nvs_commit(handle);

    nvs_close(handle);
}



bool load_wifi_credentials(
    char *ssid,
    char *pass)
{
    nvs_handle_t handle;

    if (
        nvs_open(
            "wifi",
            NVS_READONLY,
            &handle) != ESP_OK)
    {
        return false;
    }

    size_t s1 = 33;
    size_t s2 = 65;

    if (
        nvs_get_str(handle, "ssid", ssid, &s1) ||
        nvs_get_str(handle, "pass", pass, &s2))
    {
        nvs_close(handle);
        return false;
    }

    nvs_close(handle);

    return true;
}