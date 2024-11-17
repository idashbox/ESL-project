#include "pwm_handler.h"
#include "nrfx_pwm.h"

static nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_individual_t pwm_values;
static nrf_pwm_sequence_t const pwm_seq =
{
    .values.p_individual = &pwm_values,
    .length              = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats             = 0,
    .end_delay           = 0
};

void pwm_init(void)
{
    nrfx_pwm_config_t pwm_config =
    {
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = PWM_TOP_VALUE,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };

    pwm_config.output_pins[0] = LED_1_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[1] = LED_2_R_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[2] = LED_2_G_PIN | NRFX_PWM_PIN_INVERTED;
    pwm_config.output_pins[3] = LED_2_B_PIN | NRFX_PWM_PIN_INVERTED;
    

    nrfx_pwm_init(&pwm, &pwm_config, NULL);
}

void pwm_start(void)
{
    nrfx_pwm_simple_playback(&pwm, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void pwm_set_duty_cycle(uint8_t channel, uint32_t duty_cycle)
{
    duty_cycle %= PWM_TOP_VALUE + 1;

    switch (channel)
    {
        case 0:
            pwm_values.channel_0 = duty_cycle; break;
        case 1:
            pwm_values.channel_1 = duty_cycle; break;
        case 2:
            pwm_values.channel_2 = duty_cycle; break;
        case 3:
            pwm_values.channel_3 = duty_cycle; break;
    }
}
