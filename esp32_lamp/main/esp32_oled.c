#include "esp32_oled.h"

#include "esp_log.h"

#include "driver/i2c_master.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "esp_lvgl_port.h"


#define TAG "ESP_OLED"


#define OLED_I2C_PORT        I2C_NUM_0

#define OLED_SDA_GPIO        21
#define OLED_SCL_GPIO        22

#define OLED_I2C_ADDR        0x3C

#define OLED_WIDTH           128
#define OLED_HEIGHT          64


static lv_display_t *oled_disp = NULL;

static esp_lcd_panel_handle_t oled_panel = NULL;

static i2c_master_bus_handle_t i2c_bus = NULL;

static esp_lcd_panel_io_handle_t io_handle = NULL;

static lv_obj_t *oled_label = NULL;


esp_err_t esp_oled_init(void)
{

    ESP_LOGI(TAG, "Initialising OLED");


    /*
     * Create I2C bus
     */

    i2c_master_bus_config_t bus_config = {

        .clk_source = I2C_CLK_SRC_DEFAULT,

        .i2c_port = OLED_I2C_PORT,

        .sda_io_num = OLED_SDA_GPIO,

        .scl_io_num = OLED_SCL_GPIO,

        .glitch_ignore_cnt = 7,

        .flags.enable_internal_pullup = true,

    };


    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &i2c_bus
        )
    );


    /*
     * Create LCD I2C IO
     */

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_I2C_ADDR,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .scl_speed_hz = 400000,
    };


    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_i2c(
            i2c_bus,
            &io_config,
            &io_handle
        )
    );


    /*
     * SSD1306 panel configuration
     */

    esp_lcd_panel_dev_config_t panel_config = {

        .bits_per_pixel = 1,
        .reset_gpio_num = -1,
    };


    ESP_ERROR_CHECK(
        esp_lcd_new_panel_ssd1306(
            io_handle,
            &panel_config,
            &oled_panel
        )
    );


    ESP_ERROR_CHECK(
        esp_lcd_panel_reset(oled_panel)
    );


    ESP_ERROR_CHECK(
        esp_lcd_panel_init(oled_panel)
    );

    ESP_ERROR_CHECK(
    esp_lcd_panel_mirror(
        oled_panel,
        false,   // horizontal flip
        true     // vertical flip
    )
);


    ESP_ERROR_CHECK(
        esp_lcd_panel_disp_on_off(
            oled_panel,
            true
        )
    );



    /*
     * Register with LVGL
     */

    const lvgl_port_display_cfg_t display_cfg = {

        .io_handle = io_handle,

        .panel_handle = oled_panel,

        .buffer_size = OLED_WIDTH * OLED_HEIGHT,

        .double_buffer = false,

        .hres = OLED_WIDTH,

        .vres = OLED_HEIGHT,

        .monochrome = true,

    };

oled_disp = lvgl_port_add_disp(&display_cfg);

if (oled_disp == NULL) {
    ESP_LOGE(TAG, "Failed to create LVGL display");
    return ESP_FAIL;
}

lvgl_port_lock(0);

lv_obj_t *screen = lv_display_get_screen_active(oled_disp);

oled_label = lv_label_create(screen);
lv_label_set_text(oled_label, "OLED Ready");
lv_obj_center(oled_label);

lvgl_port_unlock();

ESP_LOGI(TAG, "OLED initialised");

    return ESP_OK;
}



lv_display_t *esp_oled_get_display(void)
{
    return oled_disp;
}



esp_err_t esp_oled_set_power(bool on)
{
    if(oled_panel == NULL)
        return ESP_ERR_INVALID_STATE;


    return esp_lcd_panel_disp_on_off(
        oled_panel,
        on
    );
}

void esp_oled_update(const char *text) {

    lvgl_port_lock(0);


    lv_label_set_text(
        oled_label,
        text
    );


    lvgl_port_unlock();

}

void esp_oled_update_task(LampTask task)
{
    char buf[128];

    lvgl_port_lock(0);


    if (!task.paired)
    {
        snprintf(
            buf,
            sizeof(buf),
            "NOT PAIRED\n\nOpen app"
        );

        lv_label_set_text(
            oled_label,
            buf
        );

        lvgl_port_unlock();
        return;
    }


    if (!task.has_task)
    {
        snprintf(
            buf,
            sizeof(buf),
            "No task"
        );

        lv_label_set_text(
            oled_label,
            buf
        );

        lvgl_port_unlock();
        return;
    }
if (task.has_timer)
{
    int remaining = task.remaining_seconds;


    if (remaining < 0) {
        remaining = 0;
    }
  snprintf(
        buf,
        sizeof(buf),
        "%s\n"
        "-----------\n"
        "%s %02d:%02d",
        task.title,
        task.timer_running ? "TIME" : "PAUSE",
        remaining / 60,
        remaining % 60
    );

}
else
{
    snprintf(
        buf,
        sizeof(buf),
        "%s",
        task.title
    );
}

    lv_label_set_text(
        oled_label,
        buf
    );


    lvgl_port_unlock();
}