#include "led_controller.h"
#include "pwm_handler.h"
#include "nrf_log.h"
#include "nrf_gpio.h"
#include "flash_storage.h"
#include <string.h>

extern uint32_t leds[];

uint32_t hue = (int) 360 * 0.81;
uint32_t saturation = 100;
uint32_t brightness = 100;

static uint32_t duty_cycle = 0;

controller_mode_t current_mode = MODE_DISPLAY_COLOR;
steps_for_mode_t steps_for_mode = {
    .hue_mode_step = 1,
    .sat_mode_step = 10,
    .display_mode_step = 0
};

void turn_off_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_OFF);
}

void turn_on_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_ON);
}

void update_parameters(uint32_t h, uint32_t s, uint32_t v){
    hue = h;
    saturation = s;
    brightness = v;
}

void hsv_to_rgb(uint32_t h, uint32_t s, uint32_t v, uint8_t *r, uint8_t *g, uint8_t *b) {
    const int sector = (h / 60) % 6;
    const int v_min = (((v * (PWM_TOP_VALUE - s)) << 8) / PWM_TOP_VALUE) >> 8;
    const int a = ((((v - v_min) * (h % 60)) << 8) / 60) >> 8;
    const int vinc = v_min + a;
    const int v_dec = v - a;

    switch (sector)
    {
        case 0: 
            *r = v;
            *g = vinc;
            *b = v_min;
            return;
        case 1: 
            *r = v_dec;
            *g = v;
            *b = v_min;
            return;
        case 2: 
            *r = v_min;
            *g = v;
            *b = vinc;
            return;
        case 3: 
            *r = v_min;
            *g = v_dec;
            *b = v;
            return;
        case 4: 
            *r = vinc;
            *g = v_min;
            *b = v;
            return;
        default:
            *r = v;
            *g = v_min;
            *b = v_dec;
            return;
    }
}

void display_selected_color(void) {
    uint8_t r, g, b;
    hsv_to_rgb(hue, saturation, brightness, &r, &g, &b);

    NRF_LOG_INFO("Displaying color R:%d G:%d B:%d", r, g, b);

    pwm_set_duty_cycle(1, r);
    pwm_set_duty_cycle(2, g);
    pwm_set_duty_cycle(3, b);
}

void modify_duty_cycle_for_LED1()
{
    uint8_t step;
    if (current_mode == MODE_HUE_MODIFY){
        step = steps_for_mode.hue_mode_step;
    }
    else if (current_mode == MODE_SAT_MODIFY){
        step = steps_for_mode.sat_mode_step;
    }
    else if (current_mode == MODE_DISPLAY_COLOR){
        step = steps_for_mode.display_mode_step;
    }
    else{
        pwm_set_duty_cycle(0, 100);
        return;
    }

    pwm_set_duty_cycle(0, duty_cycle);
    duty_cycle += step;

    if (duty_cycle >= PWM_TOP_VALUE || duty_cycle <= 0)
    {
        step = -step;
    }
}

void modify_hsv()
{
    uint32_t parameter = 0;
    uint32_t step_modify;
    uint32_t max_parameters_value = 0;
    static bool increasing = true;
    char name_of_parameter[20];

    if (current_mode == MODE_HUE_MODIFY){
        parameter = hue;
        step_modify = STEP_HUE_MODIFY;
        max_parameters_value = 360;
        strcpy(name_of_parameter, "Hue");
    }
    else if (current_mode == MODE_SAT_MODIFY){
        parameter = saturation;
        step_modify = STEP_SAT_MODIFY;
        max_parameters_value = 100;
        strcpy(name_of_parameter, "Saturation");
    }
    else if (current_mode == MODE_BRIGHT_MODIFY){
        parameter = brightness;
        step_modify = STEP_BRIGHT_MODIFY;
        max_parameters_value = 100;
        strcpy(name_of_parameter, "Bright");
    }

    if (increasing) {
        parameter += step_modify;
        if (parameter >= max_parameters_value) {
            parameter = max_parameters_value;
            increasing = false;
        }
    } else {
        parameter -= step_modify;
        if (parameter <= 0) {
            parameter = 0;
            increasing = true;
        }
    }

    if (current_mode == MODE_HUE_MODIFY){
        hue = parameter;
    }
    else if (current_mode == MODE_SAT_MODIFY){
        saturation = parameter;
    }
    else if (current_mode == MODE_BRIGHT_MODIFY){
        brightness= parameter;
    }
    display_selected_color();

    NRF_LOG_INFO("%S is %d", name_of_parameter, parameter);
}

void process_led_events(void)
{
    uint8_t led_state;
    uint32_t saved_hue, saved_saturation, saved_brightness;

    // Загружаем сохранённое состояние и цвет
    load_led_state_and_color(&led_state, &saved_hue, &saved_saturation, &saved_brightness);

    // Если светодиод включён
    if (led_state)
    {
        // Отображаем сохранённый цвет на светодиодах
        uint8_t r, g, b;
        hsv_to_rgb(saved_hue, saved_saturation, saved_brightness, &r, &g, &b);
        pwm_set_duty_cycle(1, r);
        pwm_set_duty_cycle(2, g);
        pwm_set_duty_cycle(3, b);
        NRF_LOG_INFO("LED turned ON with color: R:%d G:%d B:%d", r, g, b);
    }
    else
    {
        // Выключаем светодиод
        pwm_set_duty_cycle(1, 0);
        pwm_set_duty_cycle(2, 0);
        pwm_set_duty_cycle(3, 0);
        NRF_LOG_INFO("LED turned OFF");
    }
}