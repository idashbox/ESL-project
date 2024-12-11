#include "button_handler.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"

extern uint32_t BUTTON_PIN;

bool is_button_double_cliecked_g = false;
static bool first_click_passed = false;

APP_TIMER_DEF(debounce_timer_id);
APP_TIMER_DEF(double_click_timer_id);

static it_timer_t debounce_timer;
static it_timer_t double_click_timer;

uint32_t click_counter = 0;

bool is_button_once_pressed(void)
{
    return !nrf_gpio_pin_read(BUTTON_PIN);
}

bool is_button_double_cliecked(void)
{
    bool flag_for_button_double_cliecked = is_button_double_cliecked_g;
    if (is_button_double_cliecked_g){
        is_button_double_cliecked_g = !is_button_double_cliecked_g;
    }
    return flag_for_button_double_cliecked;
}

void zeroing_click_counter(){
    click_counter = 0;
}

bool is_button_clamped(void)
{
    return is_button_once_pressed() && click_counter >= 2;
}

static void start_timer(it_timer_t* timer, void* context)
{
    timer->is_working = true;
    app_timer_start(timer->timer, timer->timeout_ticks, context);
}

static void stop_timer(it_timer_t* timer)
{
    timer->is_working = false;
    app_timer_stop(timer->timer);
}

static void handle_first_click(void)
{
    click_counter = 1;
    NRF_LOG_INFO("Click is 1");
    start_timer(&debounce_timer, &first_click_passed);
    start_timer(&double_click_timer, NULL);
}

static void handle_second_click(void)
{
    start_timer(&debounce_timer, &is_button_double_cliecked_g);
}


static void interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    NRF_LOG_INFO("Interrupt_handler triggered");

    if (debounce_timer.is_working)
        stop_timer(&debounce_timer);

    if (first_click_passed)
        handle_second_click();
    else
        handle_first_click();
}

static void init_timer(it_timer_t* timer, app_timer_timeout_handler_t handler, app_timer_id_t timer_id, uint32_t timeout_ms)
{
    timer->timer = timer_id;
    app_timer_create(&timer->timer, APP_TIMER_MODE_SINGLE_SHOT, handler);
    timer->is_working = false;
    timer->timeout_ticks = APP_TIMER_TICKS(timeout_ms);
}

static void debounce_handler(void* context)
{
    NRF_LOG_INFO("Debounce_handler enter, is_button_double_cliecked_g: %d.", is_button_double_cliecked_g);

    bool* context_bool = (bool*) context;

    if (!is_button_once_pressed())
        *context_bool = !*context_bool;
    if (context == &is_button_double_cliecked_g && !is_button_once_pressed())
    {
        first_click_passed = false;
        stop_timer(&double_click_timer);
    }

    NRF_LOG_INFO("Debounce_handler out, is_button_double_cliecked_g: %d.", is_button_double_cliecked_g);

    debounce_timer.is_working = false;
}

static void double_click_handler(void* context)
{
    NRF_LOG_INFO("Double_click_handler.");

    first_click_passed = false;
    click_counter = 2;
    NRF_LOG_INFO("Clicks is 2");

    double_click_timer.is_working = false;
}

void initialize_button_handler(void)
{
    nrfx_gpiote_init();

    nrfx_gpiote_in_config_t btn_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    btn_cfg.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON_PIN, &btn_cfg, interrupt_handler);

    nrfx_gpiote_in_event_enable(BUTTON_PIN, true);

    init_timer(&debounce_timer, debounce_handler, debounce_timer_id, 50);
    init_timer(&double_click_timer, double_click_handler, double_click_timer_id, 700);
}
