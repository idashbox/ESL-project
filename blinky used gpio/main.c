#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_timer.h"

#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)
#define LED_1_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_2_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_2_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_2_B_PIN NRF_GPIO_PIN_MAP(0, 12)

uint32_t led_pins[] = {LED_1_PIN, LED_2_R_PIN, LED_2_G_PIN, LED_2_B_PIN};

APP_TIMER_DEF(double_click_timer);
volatile bool button_double_click_detected = false;
volatile bool is_blinking = false;
uint8_t current_led = -1;
uint8_t blinks[] = {6, 5, 8, 1};
uint8_t remaining_blinks = 0;

#define PWM_MAX_DUTY_CYCLE_PERCENT 100
#define PWM_MIN_DUTY_CYCLE_PERCENT 0
#define PWM_PERIOD_MS 1000
#define TIMER_TIMEOUT_MS APP_TIMER_TICKS(500)
#define DEBOUNCE_DELAY_MS 50
#define BLINK_DELAY_MS 10

void logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("Log system initialized.");
}

void led_on(uint32_t pin_number)
{
    NRF_LOG_INFO("Turning on LED at pin %d", pin_number);
    nrf_gpio_pin_clear(pin_number);
}

void led_off(uint32_t pin_number)
{
    NRF_LOG_INFO("Turning off LED at pin %d", pin_number);
    nrf_gpio_pin_set(pin_number);
}

void turn_off_all_leds()
{
    NRF_LOG_INFO("Turning off all LEDs");
    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); i++)
    {
        led_off(led_pins[i]);
    }
}

bool is_button_pressed()
{
    return !nrf_gpio_pin_read(BUTTON_PIN);
}

void configure_leds()
{
    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); i++)
    {
        nrf_gpio_cfg_output(led_pins[i]);
    }
}

void configure_button()
{
    nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
}

void initialize_gpio()
{
    NRF_LOG_INFO("Initializing GPIO pins");
    configure_leds();
    configure_button();
}

void double_click_timeout_handler(void *p_context)
{
    button_double_click_detected = false;
    NRF_LOG_INFO("Double-click timer expired");
}

void handle_button_press()
{
    if (button_double_click_detected) {
        is_blinking = !is_blinking;
        NRF_LOG_INFO("Button double-click detected, toggling blinking to %d", is_blinking);
        if (!is_blinking) {
            led_off(led_pins[current_led]);
        }
        button_double_click_detected = false;
    } else {
        button_double_click_detected = true;
        NRF_LOG_INFO("Button single click detected, starting double-click timer");
        app_timer_start(double_click_timer, TIMER_TIMEOUT_MS, NULL);
    }
}

void update_pwm(uint8_t led_index, uint8_t *duty_cycle, bool *increasing)
{
    uint32_t on_time = (*duty_cycle * PWM_PERIOD_MS) / PWM_MAX_DUTY_CYCLE_PERCENT;
    uint32_t off_time = PWM_PERIOD_MS - on_time; 

    led_on(led_pins[led_index]);
    nrf_delay_us(on_time);
    led_off(led_pins[led_index]);
    nrf_delay_us(off_time);

    if (*increasing) {
        (*duty_cycle)++;
        if (*duty_cycle >= PWM_MAX_DUTY_CYCLE_PERCENT) {
            *increasing = false;
            NRF_LOG_INFO("Duty cycle reached %d%%, switching direction to decreasing", PWM_MAX_DUTY_CYCLE_PERCENT);
        }
    } else {
        (*duty_cycle)--;
        if (*duty_cycle <= PWM_MIN_DUTY_CYCLE_PERCENT) {
            *increasing = true;
            remaining_blinks--;
            NRF_LOG_INFO("Duty cycle reached %d%%, switching direction to increasing, remaining blinks: %d", PWM_MIN_DUTY_CYCLE_PERCENT, remaining_blinks);
        }
    }
}

int main(void)
{
    logs_init();
    NRF_LOG_INFO("Starting the main application");
    
    initialize_gpio();
    turn_off_all_leds();
    app_timer_init();
    app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, double_click_timeout_handler);

    uint8_t duty_cycle = 0;
    bool increasing = true;

    while (true)
    {
        if (is_button_pressed())
        {
            nrf_delay_ms(DEBOUNCE_DELAY_MS);
            handle_button_press();
        }

        if (is_blinking)
        {
            if (remaining_blinks > 0)
            {
                update_pwm(current_led, &duty_cycle, &increasing);
                nrf_delay_ms(BLINK_DELAY_MS);
            }
            else
            {
                led_off(led_pins[current_led]);
                current_led = (current_led + 1) % (sizeof(blinks) / sizeof(blinks[0]));
                remaining_blinks = blinks[current_led];
                NRF_LOG_INFO("Moving to the next LED, index: %d, blinks: %d", current_led, remaining_blinks);
            }
        }
        else
        {
            led_off(led_pins[current_led]);
        }

        NRF_LOG_PROCESS();
    }
}
