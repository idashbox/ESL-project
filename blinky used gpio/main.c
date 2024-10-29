#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)
#define LED_1_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_2_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_2_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_2_B_PIN NRF_GPIO_PIN_MAP(0, 12)

void hal_gpio_init_output(uint32_t pin_number)
{
    nrf_gpio_cfg_output(pin_number);
}

void hal_gpio_init_input(uint32_t pin_number, uint32_t pull_config)
{
    nrf_gpio_cfg_input(pin_number, pull_config);
}

void hal_gpio_set(uint32_t pin_number)
{
    nrf_gpio_pin_set(pin_number);
}

void hal_gpio_clear(uint32_t pin_number)
{
    nrf_gpio_pin_clear(pin_number);
}

void hal_gpio_toggle(uint32_t pin_number)
{
    nrf_gpio_pin_toggle(pin_number);
}

bool hal_gpio_read(uint32_t pin_number)
{
    return nrf_gpio_pin_read(pin_number);
}

void led_on(uint32_t pin_number)
{
    hal_gpio_clear(pin_number);
}

void led_off(uint32_t pin_number)
{
    hal_gpio_set(pin_number);
}

void led_toggle(uint32_t pin_number)
{
    hal_gpio_toggle(pin_number);
}

bool is_button_pressed()
{
    return !hal_gpio_read(BUTTON_PIN);
}

void initialize_gpio()
{
    hal_gpio_init_output(LED_1_PIN);
    hal_gpio_init_output(LED_2_R_PIN);
    hal_gpio_init_output(LED_2_G_PIN);
    hal_gpio_init_output(LED_2_B_PIN);
    hal_gpio_init_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
}

void handle_button_press(uint8_t *current_led, bool *led_on_flag, uint8_t *blinks, size_t blinks_size)
{
    if (*current_led == 0)
    {
        led_toggle(LED_1_PIN);
    }
    else if (*current_led == 1)
    {
        led_toggle(LED_2_R_PIN);
    }
    else if (*current_led == 2)
    {
        led_toggle(LED_2_G_PIN);
    }
    else
    {
        led_toggle(LED_2_B_PIN);
    }
    *led_on_flag = !*led_on_flag;
    nrf_delay_ms(500);

    if (*led_on_flag)
    {
        blinks[*current_led]--;
        if (blinks[*current_led] == 0)
        {
            *current_led = (*current_led + 1) % blinks_size;
            blinks[*current_led] = (*current_led == 0) ? 6 : (*current_led == 1) ? 5 : (*current_led == 2) ? 8 : 1;

            if (*current_led == 0)
            {
                led_off(LED_1_PIN);
            }
            else if (*current_led == 1)
            {
                led_off(LED_2_R_PIN);
            }
            else if (*current_led == 2)
            {
                led_off(LED_2_G_PIN);
            }
            else
            {
                led_off(LED_2_B_PIN);
            }
        }
    }
}

void turn_off_all_leds()
{
    led_off(LED_1_PIN);
    led_off(LED_2_R_PIN);
    led_off(LED_2_G_PIN);
    led_off(LED_2_B_PIN);
}

int main(void)
{
    initialize_gpio();

    uint8_t blinks[] = {6, 5, 8, 1};
    uint8_t current_led = 0;
    bool led_on_flag = false;
    size_t blinks_size = sizeof(blinks) / sizeof(blinks[0]);

    while (true)
    {
        if (is_button_pressed())
        {
            handle_button_press(&current_led, &led_on_flag, blinks, blinks_size);
        }
        else if (led_on_flag)
        {
            continue;
        }
        else
        {
            turn_off_all_leds();
        }
    }
}
