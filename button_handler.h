#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdbool.h>
#include "app_timer.h"

typedef struct {
    app_timer_id_t timer;
    bool is_working;
    uint32_t timeout_ticks;
} it_timer_t;


bool is_button_double_cliecked(void);
bool is_button_clamped(void);
void zeroing_click_counter();
void initialize_button_handler(void);

#endif
