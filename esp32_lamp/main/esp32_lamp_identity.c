#include "esp32_lamp_identity.h"

#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
#include "esp_mac.h"


static nvs_handle_t handle;



void create_device_id(
    char *buffer
)
{

    uint8_t mac[6];

    esp_read_mac(
        mac,
        ESP_MAC_WIFI_STA
    );


    sprintf(
        buffer,
        "LAMP_%02X%02X%02X",
        mac[3],
        mac[4],
        mac[5]
    );

}

esp_err_t storage_init()
{

    esp_err_t ret =
        nvs_flash_init();


    if(ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        nvs_flash_erase();
        ret=nvs_flash_init();
    }


    nvs_open(
        "lamp",
        NVS_READWRITE,
        &handle
    );


    return ret;
}



void storage_save_device_id(
    const char *id
)
{
    nvs_set_str(
        handle,
        "device_id",
        id
    );

    nvs_commit(handle);
}



void storage_get_device_id(
    char *id,
    size_t len
)
{
    size_t size=len;

    nvs_get_str(
        handle,
        "device_id",
        id,
        &size
    );
}



void storage_save_token(
    const char *token
)
{
    nvs_set_str(
        handle,
        "token",
        token
    );

    nvs_commit(handle);
}



bool storage_get_token(
    char *token,
    size_t size
)
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            "lamp",
            NVS_READONLY,
            &handle
        );


    if(err != ESP_OK)
    {
        return false;
    }


    err =
        nvs_get_str(
            handle,
            "token",
            token,
            &size
        );


    nvs_close(handle);


    if(err != ESP_OK)
    {
        token[0] = '\0';
        return false;
    }


    return true;
}



bool storage_is_paired()
{
    uint8_t value=0;


    nvs_get_u8(
        handle,
        "paired",
        &value
    );


    return value;
}



void storage_set_paired(
    bool value
)
{
    nvs_set_u8(
        handle,
        "paired",
        value
    );

    nvs_commit(handle);
}



void storage_save_pair_code(
    const char *code
)
{
    nvs_set_str(
        handle,
        "pair_code",
        code
    );

    nvs_commit(handle);
}



void storage_get_pair_code(
    char *code,
    size_t len
)
{
    size_t size = len;


    esp_err_t err =
        nvs_get_str(
            handle,
            "pair_code",
            code,
            &size
        );


    if(err != ESP_OK)
    {
        code[0] = '\0';
    }
}


void storage_clear_token(void)
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            "lamp",
            NVS_READWRITE,
            &handle
        );


    if(err != ESP_OK)
    {
        return;
    }



    err =
        nvs_erase_key(
            handle,
            "token"
        );


    if(err == ESP_OK)
    {

        nvs_commit(
            handle
        );

    }


    nvs_close(
        handle
    );
}