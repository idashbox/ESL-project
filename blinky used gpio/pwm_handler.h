#ifndef PWM_HANDLER_H 
#define PWM_HANDLER_H

#include <stdint.h>
#include "nrfx_systick.h"

#define MICROSECONDS_IN_SECOND 1000000

typedef struct {
    uint32_t frequency;
    uint32_t duty_cycle;
} pwm_signal_t;

typedef struct {
    uint8_t step_size;
    bool direction_flag;
} pwm_control_t;

typedef struct {
    pwm_signal_t signal;
    pwm_control_t control;
    bool output_state;
    nrfx_systick_state_t time_state;
} pwm_systick_t;

#define INIT_PWM_SYSTICK    \
{                           \
    .signal = {             \
        .frequency = 1000,  \
        .duty_cycle = 1,    \
    },                      \
    .control = {            \
        .step_size = 1,     \
        .direction_flag = false, \
    },                      \
    .output_state = false   \
}

void start_pwm_timer(pwm_systick_t* settings);
bool get_pwm_state(pwm_systick_t* settings);

#endif
