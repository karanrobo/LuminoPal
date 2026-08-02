#include "esp32_ldr.h"
 
static float clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
 
    if (value > maximum)
    {
        return maximum;
    }
 
    return value;
}
 
void Lighting_Init(
    LightingController *controller,
    uint16_t adc_min,
    uint16_t adc_max,
    uint16_t pwm_min,
    uint16_t pwm_max,
    float filter_alpha,
    bool invert_response
)
{
    if (controller == 0)
    {
        return;
    }
 
    controller->adc_min = adc_min;
    controller->adc_max = adc_max;
    controller->pwm_min = pwm_min;
    controller->pwm_max = pwm_max;
 
    controller->filter_alpha =
        clamp_float(filter_alpha, 0.0f, 1.0f);
 
    controller->invert_response = invert_response;
    controller->filtered_adc = 0.0f;
    controller->initialized = false;
}
 
uint16_t Lighting_Update(
    LightingController *controller,
    uint16_t adc_raw
)
{
    float normalized;
    float pwm;
 
    if (controller == 0)
    {
        return 0;
    }
 
    /*
     * Initialise the filter using the first measurement.
     * This prevents a large startup transient.
     */
    if (!controller->initialized)
    {
        controller->filtered_adc = (float)adc_raw;
        controller->initialized = true;
    }
    else
    {
        /*
         * Exponential moving-average filter:
         *
         * filtered =
         * alpha × new sample
         * + (1 - alpha) × previous filtered value
         */
        controller->filtered_adc =
            controller->filter_alpha * (float)adc_raw +
            (1.0f - controller->filter_alpha) *
            controller->filtered_adc;
    }
 
    /*
     * Protect against an invalid calibration range.
     */
    if (controller->adc_max <= controller->adc_min)
    {
        return controller->pwm_min;
    }
 
    normalized =
        (controller->filtered_adc - (float)controller->adc_min) /
        ((float)controller->adc_max -
         (float)controller->adc_min);
 
    normalized = clamp_float(normalized, 0.0f, 1.0f);
 
    /*
     * For most adaptive-lighting systems:
     * darker environment -> brighter lamp.
     */
    if (controller->invert_response)
    {
        normalized = 1.0f - normalized;
    }
 
    pwm =
        (float)controller->pwm_min +
        normalized *
        ((float)controller->pwm_max -
         (float)controller->pwm_min);
 
    return (uint16_t)(pwm + 0.5f);
}
 
uint16_t Lighting_GetFilteredADC(
    const LightingController *controller
)
{
    if (controller == 0)
    {
        return 0;
    }
 
    return (uint16_t)(controller->filtered_adc + 0.5f);
}
