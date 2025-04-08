#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "pwm_handler.h"

#define LED_TURN_OFF 1
#define LED_TURN_ON 0

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

typedef struct {
    rgb_t color;
    uint8_t led_state;
} led_data_t;

void led_update(led_data_t *led_data);
void display_selected_color(uint8_t r, uint8_t g, uint8_t b);
void turn_on_led(uint8_t pin_number);
void turn_off_led(uint8_t pin_number);

#endif