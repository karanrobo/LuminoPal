#include "esp32_tft.h"

// static lvgl_port_display_cfg_t disp_cfg;
static const char *TAG = "EXAMPLE";
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;


esp_err_t lcd_backlight_init(void)
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

esp_err_t lcd_spi_bus_init(void)
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

esp_err_t lcd_init(void)
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
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
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

// void rotate_cb(lv_timer_t *timer)
// {
//     lv_obj_t *label = (lv_obj_t *)lv_timer_get_user_data(timer);

//     static int16_t angle = 0;

//     lv_obj_set_style_transform_rotation(
//         label,
//         angle,
//         0
//     );

//     angle += 50; // LVGL rotation uses 0.1 degree units
//     if (angle >= 3600) {
//         angle = 0;
//     }
// }

// void hue_shift_cb(lv_timer_t *timer)
// {
//     static int hue = 0;

//     lv_obj_t *label =
//         lv_timer_get_user_data(timer);


//     lv_color_t text =
//         lv_color_hsv_to_rgb(
//             hue,
//             100,
//             100
//         );


//     lv_color_t bg =
//         lv_color_hsv_to_rgb(
//             hue + 180,
//             80,
//             20
//         );


//     lv_obj_set_style_text_color(
//         label,
//         text,
//         0
//     );


//     lv_obj_set_style_bg_color(
//         lv_screen_active(),
//         bg,
//         0
//     );


//     hue++;

//     if(hue >= 360)
//         hue = 0;
// }

static void create_task_card(
    lv_obj_t *parent,
    const char *title,
    const char *status,
    lv_color_t status_color)
{
    lv_obj_t *card = lv_obj_create(parent);

    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, 30);

    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(card, 0, 0);

    // Horizontal layout
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        card,
        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_left(card, 0, 0);
    lv_obj_set_style_pad_right(card, 0, 0);

    lv_obj_t *task = lv_label_create(card);
    lv_label_set_text(task, title);

    lv_obj_set_style_text_color(task, lv_color_white(), 0);

    lv_obj_t *stat = lv_label_create(card);
    lv_label_set_text(stat, status);

    lv_obj_set_style_text_color(stat, status_color, 0);
}

void drawText(char *str) {
    ESP_ERROR_CHECK(lcd_init());
    esp_lcd_panel_set_gap(lcd_panel, 0, 0);

    /* Initialize LVGL port */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,

        .buffer_size = LCD_H_RES * 20,
        .double_buffer = true,

        .hres = LCD_H_RES,
        .vres = LCD_V_RES,

        .monochrome = false,

        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },

        .flags = {
            .buff_dma = true,
        },
    };
  
    lvgl_port_add_disp(&disp_cfg);
    lv_obj_t *screen = lv_screen_active();
    
    /* Create UI */
    lvgl_port_lock(0);

    // Active screen

lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
lv_obj_set_style_pad_all(screen, 8, 0);

// Root container
lv_obj_t *root = lv_obj_create(screen);
lv_obj_remove_style_all(root);

lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));

lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(
    root,
    LV_FLEX_ALIGN_START,
    LV_FLEX_ALIGN_START,
    LV_FLEX_ALIGN_START
);

lv_obj_set_style_pad_gap(root, 6, 0);

lv_obj_t *title = lv_label_create(root);

lv_label_set_text(title, "Today's Tasks");

lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_CYAN), 0);

create_task_card(
    root,
    "Study Math",
    "ACTIVE",
    lv_color_white());



lv_obj_t *footer = lv_label_create(root);

lv_label_set_text(
    footer,
    "Tasks: 3    Done: 1");

lv_obj_set_style_text_color(
    footer,
    lv_palette_lighten(LV_PALETTE_GREY, 2),
    0);
    lvgl_port_unlock();
}

// void initAndDrawTFT() {
//     ESP_ERROR_CHECK(lcd_init());
//     esp_lcd_panel_set_gap(lcd_panel, 0, 0);

//     /* Initialize LVGL port */
//     lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
//     ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
//     initPortDisplaySettings();

//     lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);

//     /* Create UI */
//     lvgl_port_lock(0);

//     lv_obj_t *label = lv_label_create(lv_screen_active());

//     lv_label_set_text(label, "Hello World!");
//     lv_obj_center(label);
//      lv_timer_create(
//         rotate_cb,
//         20,
//         label
//     );


//     lv_timer_create(
//         hue_shift_cb,
//         20,
//         label
//     );

//     lvgl_port_unlock();
// }

