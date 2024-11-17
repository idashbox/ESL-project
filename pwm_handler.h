#ifndef PWM_HANDLER_H
#define PWM_HANDLER_H

#include <stdint.h>
#include "nrfx_gpiote.h"
#include "nrfx_pwm.h"
#include "nrf_gpio.h"


#define PWM_TOP_VALUE 100
#define PWM_MIN_VALUE 0
#define LED_1_PIN NRF_GPIO_PIN_MAP(0, 6)
#define LED_2_R_PIN NRF_GPIO_PIN_MAP(0, 8)
#define LED_2_G_PIN NRF_GPIO_PIN_MAP(1, 9)
#define LED_2_B_PIN NRF_GPIO_PIN_MAP(0, 12)

void pwm_init();
void pwm_start();
void pwm_stop();
void pwm_set_duty_cycle(uint8_t channel, uint32_t duty_cycle);

#endif