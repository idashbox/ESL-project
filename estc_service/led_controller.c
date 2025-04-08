#include "led_controller.h"
#include "pwm_handler.h"
#include "nrf_log.h"
#include "nrf_gpio.h"
#include "flash_storage.h"
#include <string.h>

uint32_t leds[] = {
    NRF_GPIO_PIN_MAP(0, 6),
    NRF_GPIO_PIN_MAP(0, 8),
    NRF_GPIO_PIN_MAP(1, 9),
    NRF_GPIO_PIN_MAP(0, 12)
};

void turn_off_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_OFF);
}

void turn_on_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_ON);
}

void display_selected_color(uint8_t r, uint8_t g, uint8_t b) {
    turn_on_led(1);
    turn_on_led(2);
    turn_on_led(3);

    pwm_set_duty_cycle(1, r);
    pwm_set_duty_cycle(2, g);
    pwm_set_duty_cycle(3, b);
}

void led_update(led_data_t *led_data)
{
    if (led_data->led_state == 0){
        pwm_set_duty_cycle(1, 0);
        pwm_set_duty_cycle(2, 0);
        pwm_set_duty_cycle(3, 0);
        return;
    }

    display_selected_color(led_data->color.r, led_data->color.g, led_data->color.b);
}
