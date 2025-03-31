#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "pwm_handler.h"

#define LED_TURN_OFF 1
#define LED_TURN_ON 0

#define STEP_HUE_MODIFY 1
#define STEP_SAT_MODIFY 1
#define STEP_BRIGHT_MODIFY 1

typedef enum {
    MODE_HUE_MODIFY,
    MODE_SAT_MODIFY,
    MODE_BRIGHT_MODIFY,
    MODE_DISPLAY_COLOR
} controller_mode_t;

controller_mode_t current_mode;

typedef struct {
    uint8_t hue_mode_step;
    uint8_t sat_mode_step;
    uint8_t display_mode_step;
} steps_for_mode_t;

extern uint32_t hue;
extern uint32_t saturation;
extern uint32_t brightness;

void process_led_events(void);
void turn_on_led(uint8_t pin_number);
void turn_off_led(uint8_t pin_number);
void turn_on_RGB_LED(void);
void hsv_to_rgb(uint32_t h, uint32_t s, uint32_t v, uint8_t *r, uint8_t *g, uint8_t *b);            
void process_led_events(); 
void update_parameters(uint32_t h, uint32_t s, uint32_t v);

#endif