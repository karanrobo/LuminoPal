idf_component_register(
    SRCS
        "esp32_lamp.c"
        "esp32_wifi.c"
        "esp32_tft.c"
        "esp32_oled.c"
        "esp32_lamp_identity.c"
        "esp32_lamp_api.c"
        "esp32_pir.c"
        "esp32_ldr.c"
        "esp32_buttons.c"
        "esp32_buzzer.c"   
    INCLUDE_DIRS "."
)