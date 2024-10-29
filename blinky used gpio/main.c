#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define BUTTON_PIN NRF_GPIO_PIN_MAP(1, 6)
#define LED_1_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_2_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_2_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_2_B_PIN NRF_GPIO_PIN_MAP(0, 12)

void led_on(uint32_t pin_number)
{
    nrf_gpio_pin_clear(pin_number);
}

void led_off(uint32_t pin_number)
{
    nrf_gpio_pin_set(pin_number);
}

void led_toggle(uint32_t pin_number)
{
    nrf_gpio_pin_toggle(pin_number);
}

bool is_button_pressed()
{
    return !nrf_gpio_pin_read(BUTTON_PIN);
}

int main(void)
{
    nrf_gpio_cfg_output(LED_1_PIN);
    nrf_gpio_cfg_output(LED_2_R_PIN);
    nrf_gpio_cfg_output(LED_2_G_PIN);
    nrf_gpio_cfg_output(LED_2_B_PIN);
    nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);

    uint8_t blinks[] = {6, 5, 8, 1};
    uint8_t current_led = 0;
    bool led_on_flag = false;

    while (true)
    {
        if (is_button_pressed())
        {
            if (current_led == 0)
            {
                led_toggle(LED_1_PIN);
            }
            else if (current_led == 1)
            {
                led_toggle(LED_2_R_PIN);
            }
            else if (current_led == 2)
            {
                led_toggle(LED_2_G_PIN);
            }
            else
            {
                led_toggle(LED_2_B_PIN);
            }
            led_on_flag = !led_on_flag;
            nrf_delay_ms(500);

            if (led_on_flag)
            {
                blinks[current_led]--;
                if (blinks[current_led] == 0)
                {
                    current_led = (current_led + 1) % (sizeof(blinks) / sizeof(blinks[0]));
                    blinks[current_led] = (current_led == 0) ? 6 : (current_led == 1) ? 5 : (current_led == 2) ? 8 : 1;

                    if (current_led == 0)
                    {
                        led_off(LED_1_PIN);
                    }
                    else if (current_led == 1)
                    {
                        led_off(LED_2_R_PIN);
                    }
                    else if (current_led == 2)
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
        else if (led_on_flag)
        {
            continue;
        }
        else
        {
            led_off(LED_1_PIN);
            led_off(LED_2_R_PIN);
            led_off(LED_2_G_PIN);
            led_off(LED_2_B_PIN);
        }
    }
}
