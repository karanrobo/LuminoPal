#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7735.h"
#include "esp_lvgl_port.h"



#define LCD_SPI_NUM     SPI2_HOST

#define LCD_GPIO_SCLK   14
#define LCD_GPIO_MOSI   13
#define LCD_GPIO_CS     15
#define LCD_GPIO_DC      4
#define LCD_GPIO_RST     2
#define LCD_GPIO_BL     17

#define LCD_BL_ON_LEVEL  1

#define LCD_V_RES      128
#define LCD_H_RES     160
#define LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS     8
#define LCD_PARAM_BITS   8
#define LCD_BITS_PER_PIXEL 16

// test to match the received data



esp_err_t lcd_backlight_init(void);

esp_err_t lcd_spi_bus_init(void);

esp_err_t lcd_init(void);

// void rotate_cb(lv_timer_t *timer);

// void hue_shift_cb(lv_timer_t *timer);

void drawText(char *str);

// void initAndDrawTFT();