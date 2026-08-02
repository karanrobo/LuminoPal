#ifndef ESP_32_LDR_LIGHTING_H
#define ESP_32_LDR_LIGHTING_H
 
#include <stdbool.h>
#include <stdint.h>
 
typedef struct
{
    uint16_t adc_min;
    uint16_t adc_max;
 
    uint16_t pwm_min;
    uint16_t pwm_max;
 
    float filter_alpha;
    float filtered_adc;
 
    bool invert_response;
    bool initialized;
 
} LightingController;
 
 
void Lighting_Init(
    LightingController *controller,
    uint16_t adc_min,
    uint16_t adc_max,
    uint16_t pwm_min,
    uint16_t pwm_max,
    float filter_alpha,
    bool invert_response
);
 
 
uint16_t Lighting_Update(
    LightingController *controller,
    uint16_t adc_raw
);
 
 
uint16_t Lighting_GetFilteredADC(
    const LightingController *controller
);
 
#endif
 