#include "button_handler.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"

extern uint32_t BUTTON_PIN;

static bool is_active = false;
static bool first_click_passed = false;

APP_TIMER_DEF(debounce_timer_id);
APP_TIMER_DEF(double_click_timer_id);

static itq_timer_t debounce_timer;
static itq_timer_t double_click_timer;

bool is_button_pressed(void)
{
    return is_active;
}

static void start_timer(itq_timer_t* timer, void* context)
{
    timer->is_working = true;
    app_timer_start(timer->timer, timer->timeout_ticks, context);
}

static void stop_timer(itq_timer_t* timer)
{
    timer->is_working = false;
    app_timer_stop(timer->timer);
}

static void btn_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    NRF_LOG_INFO("Btn_IRQHandler triggered");

    if (debounce_timer.is_working)
        stop_timer(&debounce_timer);

    if (first_click_passed)
        start_timer(&debounce_timer, &is_active);
    else
    {
        start_timer(&debounce_timer, &first_click_passed);
        start_timer(&double_click_timer, NULL);
    }
}

static void init_timer(itq_timer_t* timer, app_timer_timeout_handler_t handler, app_timer_id_t timer_id, uint32_t timeout_ms)
{
    timer->timer = timer_id;
    app_timer_create(&timer->timer, APP_TIMER_MODE_SINGLE_SHOT, handler);
    timer->is_working = false;
    timer->timeout_ticks = APP_TIMER_TICKS(timeout_ms);
}

static void debounce_handler(void* context)
{
    bool press = nrf_gpio_pin_read(BUTTON_PIN);
    NRF_LOG_INFO("Debounce_handler enter, is_active: %d.", is_active);

    bool* context_bool = (bool*) context;

    if (nrf_gpio_pin_read(BUTTON_PIN))
        *context_bool = !*context_bool;
    if (context == &is_active && press)
    {
        first_click_passed = false;
        stop_timer(&double_click_timer);
    }

    NRF_LOG_INFO("Debounce_handler out, is_active: %d.", is_active);

    debounce_timer.is_working = false;
}

static void double_click_handler(void* context)
{
    NRF_LOG_INFO("Double_click_handler.");

    first_click_passed = false;

    double_click_timer.is_working = false;
}

void initialize_button_handler(void)
{
    nrfx_gpiote_init();

    nrfx_gpiote_in_config_t btn_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    btn_cfg.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BUTTON_PIN, &btn_cfg, btn_IRQHandler);

    nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
}

void initialize_timers(void)
{
    init_timer(&debounce_timer, debounce_handler, debounce_timer_id, 50);
    init_timer(&double_click_timer, double_click_handler, double_click_timer_id, 1000);
}

void process_button_events(void)
{

}
