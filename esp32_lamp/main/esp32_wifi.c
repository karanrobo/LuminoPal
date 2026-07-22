#include "esp32_wifi.h"
#include "esp_http_server.h"
#include "esp_http_client.h"


static const char *TAG = "Basic soft-AP";

static const char html_page[] =
"<!DOCTYPE html>"
"<html>"
"<body>"
"<h2>WiFi Setup</h2>"
"<form method=\"POST\" action=\"/connect\" enctype='text/plain'>"
"SSID:<br>"
"<input name=\"ssid\"><br><br>"
"Password:<br>"
"<input type=\"password\" name=\"password\"><br><br>"
"<input type=\"submit\" value=\"Connect\">"
"</form>"
"</body>"
"</html>";

void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch(event_id)
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


    if(event_base == IP_EVENT &&
       event_id == IP_EVENT_STA_GOT_IP)
    {
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

    int len = httpd_req_recv(req, buf, sizeof(buf)-1);

    if (len <= 0) return ESP_FAIL;

    buf[len] = '\0';

    printf("POST data: %s\n", buf);

    ESP_LOGI(TAG,"Received: %s",buf);

 char ssid[33] = {0};
    char password[65] = {0};
    /*
     * Parse:
     * ssid=mywifi&password=mypass
     */

    char *ssid_ptr =
        strstr(buf,"ssid=");


    char *pass_ptr =
        strstr(buf,"password=");



    if(ssid_ptr && pass_ptr)
    {

        ssid_ptr += 5;


        int ssid_len =
            pass_ptr - ssid_ptr;


        memcpy(
            ssid,
            ssid_ptr,
            ssid_len
        );


        strcpy(
            password,
            pass_ptr + 9
        );


        ESP_LOGI(TAG,
                 "SSID:%s PASS:%s",
                 ssid,
                 password);



        /*
         * Configure STA
         */
wifi_config_t sta_config = {0};
      
sta_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;

sta_config.sta.failure_retry_cnt = 5;

sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        strcpy(
            (char *)sta_config.sta.ssid,
            ssid
        );


        strcpy(
            (char *)sta_config.sta.password,
            password
        );



        ESP_ERROR_CHECK(
            esp_wifi_set_config(
                WIFI_IF_STA,
                &sta_config
            )
        );



        ESP_ERROR_CHECK(
            esp_wifi_connect()
        );

    }

    httpd_resp_sendstr(
        req,
        "Connecting..."
    );

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
        .handler = root_get_handler
    };

    httpd_register_uri_handler(server, &root);

    httpd_uri_t connect = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_post_handler
    };

    httpd_register_uri_handler(server, &connect);
}


void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );


    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .required = true,
            },
        },
    };


    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_APSTA)
    );


    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_AP,
            &wifi_config
        )
    );


    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(TAG,
             "WiFi AP started. SSID:%s password:%s channel:%d",
             ESP_WIFI_SSID,
             ESP_WIFI_PASS,
             ESP_WIFI_CHANNEL);
}