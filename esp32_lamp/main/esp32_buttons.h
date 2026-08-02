#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include "driver/gpio.h"


#define START_BUTTON_GPIO    GPIO_NUM_32
#define RESET_BUTTON_GPIO    GPIO_NUM_33

/**
 * @brief Initialize button GPIOs.
 */
void buttons_init(void);

/**
 * @brief Returns true once when the Start button is pressed.
 */
bool start_button_pressed(void);

/**
 * @brief Returns true once when the Reset button is pressed.
 */
bool reset_button_pressed(void);



#endif