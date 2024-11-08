#include "pwm_handler.h"
#include "nrfx_systick.h"

void start_pwm_timer(pwm_systick_t* pwm_systick)
{
    nrfx_systick_get(&pwm_systick->time_state);
}

void update_direction_flag(pwm_systick_t* pwm_systick)
{
    if (pwm_systick->signal.duty_cycle == MICROSECONDS_IN_SECOND / pwm_systick->signal.frequency && !pwm_systick->control.direction_flag)
    {
        pwm_systick->control.direction_flag = true;
    }
    if (pwm_systick->signal.duty_cycle == 0 && pwm_systick->control.direction_flag)
    {
        pwm_systick->control.direction_flag = false;
    }
}

void update_duty_cycle(pwm_systick_t* pwm_systick)
{
    if (!pwm_systick->output_state && pwm_systick->control.direction_flag)
    {
        pwm_systick->signal.duty_cycle += -pwm_systick->control.step_size;
    }
    if (pwm_systick->output_state && !pwm_systick->control.direction_flag)
    {
        pwm_systick->signal.duty_cycle += pwm_systick->control.step_size;
    }
}

bool get_pwm_state(pwm_systick_t* pwm_systick)
{
    if (!nrfx_systick_test(&pwm_systick->time_state, pwm_systick->output_state ? MICROSECONDS_IN_SECOND / pwm_systick->signal.frequency - pwm_systick->signal.duty_cycle : pwm_systick->signal.duty_cycle))
        return pwm_systick->output_state;

    pwm_systick->output_state = !pwm_systick->output_state;

    update_direction_flag(pwm_systick);
    update_duty_cycle(pwm_systick);

    start_pwm_timer(pwm_systick);

    return pwm_systick->output_state;
}
