#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdbool.h>
#include "app_timer.h"

typedef struct {
    app_timer_id_t timer;
    bool is_working;
    uint32_t timeout_ticks;
} itq_timer_t;

bool is_button_pressed(void);
void initialize_button_handler(void);
void process_button_events(void);
void initialize_timers(void);

#endif
