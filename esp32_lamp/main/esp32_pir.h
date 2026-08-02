#ifndef ESP32_PIR_H
#define ESP32_PIR_H
 
#include <stdbool.h>
 
void pir_init(void);
bool pir_motion_detected(void);
 
#endif