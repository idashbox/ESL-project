#include "led_controller.h"
#include "nrf_gpio.h"
#include "button_handler.h"
#include "nrf_log.h"

#define LEDS_NUMBER 4

extern uint8_t blinks[];
extern uint32_t leds[];

#define LED_TURN_OFF 1
#define LED_TURN_ON 0

static uint8_t active_led = 0;
static uint8_t blinks_passed = 0;

void turn_off_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_OFF);
}

void turn_on_led(uint8_t pin_number)
{
    nrf_gpio_pin_write(leds[pin_number], LED_TURN_ON);
}

void switch_led(void)
{
    if (blinks_passed >= blinks[active_led])
    {
        turn_off_led(active_led);
        blinks_passed = 0;
        active_led = (active_led + 1) % LEDS_NUMBER;

        switch_led();
    }
}

void process_led_events(pwm_systick_t* pwm_systick)
{
    if (is_button_pressed())
    {
        NRF_LOG_INFO("LEDS events is active.");
        nrf_gpio_pin_write(leds[active_led], get_pwm_state(pwm_systick));

        if (pwm_systick->signal.duty_cycle == 0)
        {
            ++blinks_passed;
        }
    }
    else
    {
        turn_off_led(active_led);
    }

    switch_led();
}
