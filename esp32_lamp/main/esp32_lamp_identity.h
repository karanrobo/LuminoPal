#ifndef LAMP_STORAGE_H
#define LAMP_STORAGE_H


#include "esp_err.h"
#include <stdbool.h>

esp_err_t storage_init();

void create_device_id(
    char *buffer
);

void storage_save_device_id(
    const char *id
);


void storage_get_device_id(
    char *id,
    size_t len
);



void storage_save_token(
    const char *token
);


bool storage_get_token(
    char *token,
    size_t len
);



bool storage_is_paired();


void storage_set_paired(
    bool value
);

void storage_save_pair_code(
    const char *code
);


void storage_get_pair_code(
    char *code,
    size_t len
);

void storage_clear_token(void);


#endif