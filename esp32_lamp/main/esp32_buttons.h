#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include "driver/gpio.h"


#define START_BUTTON_GPIO    GPIO_NUM_32 // resume/start
#define RESET_BUTTON_GPIO    GPIO_NUM_33

/**
 * @brief Initialize button GPIOs.
 */
void buttons_init(void);

/**
 * @brief Returns true once when the Start button is pressed.
 */
bool pause_start_button(void);

/**
 * @brief Returns true once when the Reset button is pressed.
 */
bool light_button(void);



#endif