#include "esp32_lamp_api.h"

#include "esp_http_client.h"
#include "esp_log.h"

#include "cJSON.h"

#include "esp32_lamp_identity.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static const char *TAG = "LAMP_API";

#define HTTP_RESPONSE_SIZE 512

static char http_response[HTTP_RESPONSE_SIZE];
static size_t http_response_len = 0;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_CONNECTED:

        ESP_LOGI(TAG, "HTTP connected");

        http_response_len = 0;
        http_response[0] = '\0';

        break;

    case HTTP_EVENT_ON_HEADER:

        ESP_LOGD(TAG,
                 "%s: %s",
                 evt->header_key,
                 evt->header_value);

        break;

    case HTTP_EVENT_ON_DATA:

        ESP_LOGI(TAG,
                 "Received %d bytes",
                 evt->data_len);

        if (http_response_len + evt->data_len < HTTP_RESPONSE_SIZE)
        {
            memcpy(http_response + http_response_len,
                   evt->data,
                   evt->data_len);

            http_response_len += evt->data_len;
            http_response[http_response_len] = '\0';
        }
        else
        {
            ESP_LOGW(TAG, "HTTP response buffer full");
        }

        break;

    case HTTP_EVENT_ON_FINISH:

        ESP_LOGI(TAG,
                 "HTTP finished");

        ESP_LOGI(TAG,
                 "Response: %s",
                 http_response);

        break;

    case HTTP_EVENT_DISCONNECTED:

        ESP_LOGI(TAG,
                 "HTTP disconnected");

        break;

    default:
        break;
    }

    return ESP_OK;
}

bool api_register_lamp(const char *device_id)
{
    char json[128];

    snprintf(json,
             sizeof(json),
             "{\"device_id\":\"%s\"}",
             device_id);

    ESP_LOGI(TAG, "Registering lamp...");
    ESP_LOGI(TAG, "JSON: %s", json);

    esp_http_client_config_t config = {
        .url = "http://192.168.1.112:8000/webapp/lamp/register/",
        .event_handler = http_event_handler,
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    esp_http_client_set_method(client,
                               HTTP_METHOD_POST);

    esp_http_client_set_header(client,
                               "Content-Type",
                               "application/json");

    esp_http_client_set_post_field(client,
                                   json,
                                   strlen(json));

    esp_err_t err =
        esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "HTTP failed: %s",
                 esp_err_to_name(err));

        esp_http_client_cleanup(client);
        return false;
    }

    ESP_LOGI(TAG,
             "Status = %d",
             esp_http_client_get_status_code(client));

    cJSON *root = cJSON_Parse(http_response);

    if (!root)
    {
        ESP_LOGE(TAG, "JSON parse failed");

        esp_http_client_cleanup(client);
        return false;
    }

    cJSON *pair_code =
        cJSON_GetObjectItem(root,
                            "pair_code");

    if (cJSON_IsString(pair_code))
    {
        ESP_LOGI(TAG,
                 "Pair code: %s",
                 pair_code->valuestring);

        storage_save_pair_code(
            pair_code->valuestring);
    }

    cJSON *paired =
        cJSON_GetObjectItem(root,
                            "paired");

    if (paired)
    {
        ESP_LOGI(TAG,
                 "paired = %s",
                 cJSON_IsTrue(paired)
                     ? "true"
                     : "false");
    }

    cJSON_Delete(root);
    esp_http_client_cleanup(client);

    return true;
}

bool api_check_pair_status(const char *device_id)
{
    char url[256];

    snprintf(url,
             sizeof(url),
             "http://192.168.1.112:8000/webapp/lamp/status/?device_id=%s",
             device_id);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    esp_http_client_set_method(client,
                               HTTP_METHOD_GET);

    esp_err_t err =
        esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "Status request failed");

        esp_http_client_cleanup(client);
        return false;
    }

    ESP_LOGI(TAG,
             "Status = %d",
             esp_http_client_get_status_code(client));

    cJSON *root = cJSON_Parse(http_response);

    if (!root)
    {
        ESP_LOGE(TAG,
                 "JSON parse failed");

        esp_http_client_cleanup(client);
        return false;
    }

    cJSON *paired =
        cJSON_GetObjectItem(root,
                            "paired");

    if (cJSON_IsTrue(paired))
    {
        cJSON *token =
            cJSON_GetObjectItem(root,
                                "auth_token");

        if (cJSON_IsString(token))
        {
            storage_save_token(
                token->valuestring);

            storage_set_paired(true);

            ESP_LOGI(TAG,
                     "Lamp paired!");

            cJSON_Delete(root);
            esp_http_client_cleanup(client);

            return true;
        }
    }

    cJSON_Delete(root);
    esp_http_client_cleanup(client);

    return false;
}



LampTask api_get_tasks(
    const char *device_id
)
{

    LampTask task = {
        .paired = false,
        .has_task = false,
        .has_timer = false
    };


    char token[256] = {0};


    storage_get_token(
        token,
        sizeof(token)
    );


    if(strlen(token) == 0)
    {

        ESP_LOGE(
            TAG,
            "No auth token"
        );

        return task;
    }



    char url[256];


    snprintf(
        url,
        sizeof(url),
        "http://192.168.1.112:8000/webapp/lamp/get_tasks/?device_id=%s",
        device_id
    );



    esp_http_client_config_t config =
    {
        .url = url,
        .event_handler = http_event_handler
    };



    esp_http_client_handle_t client =
        esp_http_client_init(
            &config
        );



    esp_http_client_set_method(
        client,
        HTTP_METHOD_GET
    );


    /*
     * Send token
     */

    esp_http_client_set_header(
        client,
        "Authorization",
        token
    );



    esp_err_t err =
        esp_http_client_perform(
            client
        );



    if(err != ESP_OK)
    {

        ESP_LOGE(
            TAG,
            "Task request failed: %s",
            esp_err_to_name(err)
        );


        esp_http_client_cleanup(
            client
        );

        return task;
    }



    ESP_LOGI(
        TAG,
        "Task response: %s",
        http_response
    );



    cJSON *root =
        cJSON_Parse(
            http_response
        );



    if(root == NULL)
    {

        ESP_LOGE(
            TAG,
            "JSON parse failed"
        );


        esp_http_client_cleanup(
            client
        );


        return task;
    }



    /*
     * Check paired
     */

    cJSON *paired =
        cJSON_GetObjectItem(
            root,
            "paired"
        );


    if(cJSON_IsTrue(paired))
    {

        task.paired = true;

    }
    else
    {

        ESP_LOGW(
            TAG,
            "Lamp not paired"
        );


        cJSON_Delete(root);

        esp_http_client_cleanup(
            client
        );

        return task;

    }




    /*
     * Get task object
     */

    cJSON *json_task =
        cJSON_GetObjectItem(
            root,
            "task"
        );



    if(json_task == NULL ||
       cJSON_IsNull(json_task))
    {

        ESP_LOGI(
            TAG,
            "No current task"
        );


        task.has_task = false;


        cJSON_Delete(root);

        esp_http_client_cleanup(
            client
        );


        return task;

    }



    task.has_task = true;



    /*
     * Title
     */

    cJSON *title =
        cJSON_GetObjectItem(
            json_task,
            "title"
        );


    if(title &&
       cJSON_IsString(title))
    {

        strncpy(
            task.title,
            title->valuestring,
            sizeof(task.title)-1
        );

    }




    /*
     * Description
     */

    cJSON *description =
        cJSON_GetObjectItem(
            json_task,
            "description"
        );


    if(description &&
       cJSON_IsString(description))
    {

        strncpy(
            task.description,
            description->valuestring,
            sizeof(task.description)-1
        );

    }




    /*
     * Timer
     *
     * Django should send:
     *
     * "timer_end": 1785366000
     *
     */

    cJSON *timer =
        cJSON_GetObjectItem(
            json_task,
            "timer_end"
        );


    if(timer &&
       cJSON_IsNumber(timer))
    {

        task.has_timer = true;


        task.timer_end =
            (time_t)timer->valuedouble;


        ESP_LOGI(
            TAG,
            "Timer end: %lld",
            task.timer_end
        );

    }



    cJSON_Delete(root);


    esp_http_client_cleanup(
        client
    );


    return task;
}





