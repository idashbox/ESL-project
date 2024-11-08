#include <stdbool.h>
#include <stdint.h>

#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "nrfx_gpiote.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"
#include "led_controller.h"
#include "button_handler.h"
#include "pwm_handler.h"

#define LEDS_NUMBER 4

uint8_t blinks[] = {6, 5, 8, 1};

const uint32_t BUTTON_PIN = NRF_GPIO_PIN_MAP(1, 6);
const uint32_t LED_1_PIN = NRF_GPIO_PIN_MAP(0, 6);
const uint32_t LED_2_R_PIN = NRF_GPIO_PIN_MAP(0, 8);
const uint32_t LED_2_G_PIN = NRF_GPIO_PIN_MAP(1, 9);
const uint32_t LED_2_B_PIN = NRF_GPIO_PIN_MAP(0, 12);

const uint32_t leds[] = {LED_1_PIN, LED_2_R_PIN, LED_2_G_PIN, LED_2_B_PIN};

void init_led_controller(void)
{
    for(int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], 1);
    }
    NRF_LOG_INFO("LEDS init");
}

void logs_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void periph_init(void)
{
    nrfx_systick_init();
    app_timer_init();
    logs_init();
    initialize_button_handler();
    init_led_controller();
}

int main(void)
{
    periph_init();
    pwm_systick_t pwm_systick = INIT_PWM_SYSTICK;
    start_pwm_timer(&pwm_systick);
    initialize_timers();

    while (true)
    {
        process_button_events();
        process_led_events(&pwm_systick);

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }

    return 0;
}
