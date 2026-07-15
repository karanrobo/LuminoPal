#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_timer.h"


void vFunction1() {
    for (size_t i = 0; i < 5; i++)
    {
        printf("Function 1 %d: %lld\n", i, esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}


void vFunction2() {
    for (size_t i = 0; i < 5; i++)
    {
        printf("Function 2 %d: %lld\n", i, esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}


void app_main(void)
{
    xTaskCreate(vFunction1, "vFunction1", 2048, NULL, 5, NULL);
    xTaskCreate(vFunction2, "vFunction2", 2048, NULL, 5, NULL);
    
}
