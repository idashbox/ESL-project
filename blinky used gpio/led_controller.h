#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "pwm_handler.h"

void switch_led(void);             
void process_led_events(pwm_systick_t* pwm_systick); 

#endif
