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

#define TAG "ST7735"

#define LCD_SPI_NUM     SPI2_HOST

#define LCD_GPIO_SCLK   14
#define LCD_GPIO_MOSI   13
#define LCD_GPIO_CS     15
#define LCD_GPIO_DC      4
#define LCD_GPIO_RST     2
#define LCD_GPIO_BL     17

#define LCD_BL_ON_LEVEL  1

#define LCD_H_RES      128
#define LCD_V_RES      160
#define LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS     8
#define LCD_PARAM_BITS   8
#define LCD_BITS_PER_PIXEL 16

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

static esp_err_t lcd_backlight_init(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_GPIO_BL,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&bk_gpio_config);
}

static esp_err_t lcd_spi_bus_init(void)
{
    const spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_GPIO_SCLK,
        .mosi_io_num = LCD_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
    };

    return spi_bus_initialize(LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);
}

static esp_err_t lcd_init(void)
{
    esp_err_t ret;

    ret = lcd_backlight_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Backlight init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = lcd_spi_bus_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_GPIO_DC,
        .cs_gpio_num = LCD_GPIO_CS,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &io_config, &lcd_io);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel IO init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_GPIO_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };

    ret = esp_lcd_new_panel_st7735(lcd_io, &panel_config, &lcd_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ST7735 panel init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_reset(lcd_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel reset failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_init(lcd_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // ret = esp_lcd_panel_invert_color(lcd_panel, true);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Invert color failed: %s", esp_err_to_name(ret));
    //     return ret;
    // }

    ret = esp_lcd_panel_disp_on_off(lcd_panel, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display on failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = gpio_set_level(LCD_GPIO_BL, LCD_BL_ON_LEVEL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Backlight on failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "ST7735 LCD init success");
    return ESP_OK;
}

static void lcd_fill_screen(esp_lcd_panel_handle_t panel, uint16_t color)
{
    uint16_t line[LCD_H_RES];

    for (int x = 0; x < LCD_H_RES; x++) {
        line[x] = color;
    }

    for (int y = 0; y < LCD_V_RES; y++) {
        esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_H_RES, y + 1, line);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(lcd_init());

    esp_lcd_panel_set_gap(lcd_panel, 2, 3);

    lcd_fill_screen(lcd_panel, 0xF800);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}