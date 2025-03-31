#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "estc_service.h"
#include "flash_storage.h"
#include "led_controller.h"
#include "pwm_handler.h"

#define DEVICE_NAME                     "ESTC-SVC"                              /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                18000                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define LEDS_NUMBER 4

const uint32_t leds[] = {LED_1_PIN, LED_2_R_PIN, LED_2_G_PIN, LED_2_B_PIN};

NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);
BLE_ADVERTISING_DEF(m_advertising);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

static ble_uuid_t m_adv_uuids[] =
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},

};

ble_estc_service_t m_estc_service;

APP_TIMER_DEF(m_led_state_notify_timer);
APP_TIMER_DEF(m_led_color_notify_timer);

// void notify_timer_handler(void *p_context)
// {
//     ble_estc_service_t *p_service = (ble_estc_service_t *) p_context;
//     static uint32_t notify_value = 0;
//     notify_value++;

//     ble_gatts_hvx_params_t hvx_params = {0};
//     hvx_params.handle = p_service->notify_char_handles.value_handle;
//     hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
//     hvx_params.offset = 0;
//     uint16_t len = sizeof(notify_value);
//     hvx_params.p_len = &len;
//     hvx_params.p_data = (uint8_t *)&notify_value;

//     if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
//     {
//         sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);
//     }
// }

// void indicate_timer_handler(void *p_context)
// {
//     ble_estc_service_t *p_service = (ble_estc_service_t *) p_context;
//     static uint32_t indicate_value = 100;
//     indicate_value++;

//     ble_gatts_hvx_params_t hvx_params = {0};
//     hvx_params.handle = p_service->indicate_char_handles.value_handle;
//     hvx_params.type = BLE_GATT_HVX_INDICATION;
//     hvx_params.offset = 0;
//     uint16_t len = sizeof(indicate_value);
//     hvx_params.p_len = &len;
//     hvx_params.p_data = (uint8_t *)&indicate_value;

//     if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
//     {
//         sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);
//     }
// }

void led_state_notify_handler(void *p_context)
{
    ble_estc_service_t *p_service = (ble_estc_service_t *) p_context;
    ble_gatts_hvx_params_t hvx_params = {0};
    hvx_params.handle = m_estc_service.led_state_char_handles.value_handle;
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

    // Загружаем состояние LED из памяти
    uint8_t led_state;
    load_led_state_and_color(&led_state, &hue, &saturation, &brightness);  // Загружаем сохранённое состояние и цвет из памяти
    p_service->led_state = led_state;  // Обновляем состояние в p_service для последующей передачи

    uint16_t len = sizeof(led_state);
    hvx_params.p_len = &len;
    hvx_params.p_data = (uint8_t *) &led_state;

    if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Отправляем уведомление по BLE
        sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);

        // Сохраняем состояние и цвет в памяти
        save_led_state_and_color(led_state, hue, saturation, brightness);  // Сохраняем состояние и цвет в память
    }
}

void led_color_notify_handler(void *p_context)
{
    ble_estc_service_t *p_service = (ble_estc_service_t *) p_context;
    ble_gatts_hvx_params_t hvx_params = {0};
    hvx_params.handle = m_estc_service.led_color_char_handles.value_handle;
    hvx_params.type = BLE_GATT_HVX_INDICATION;

    // Загружаем сохранённый цвет из памяти
    uint32_t saved_hue, saved_saturation, saved_brightness;
    load_led_state_and_color(&p_service->led_state, &saved_hue, &saved_saturation, &saved_brightness);  // Загружаем сохранённые параметры цвета

    // Обновляем текущие значения цвета из памяти
    hue = saved_hue;
    saturation = saved_saturation;
    brightness = saved_brightness;

    // Отправляем уведомление с текущим цветом
    uint32_t led_color = (hue << 16) | (saturation << 8) | brightness;  // Преобразуем цвет в 32-битное значение
    uint16_t len = sizeof(led_color);
    hvx_params.p_len = &len;
    hvx_params.p_data = (uint8_t *) &led_color;

    if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Отправляем уведомление по BLE
        sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);

        // Сохраняем новый цвет в память
        save_led_state_and_color(1, hue, saturation, brightness);  // Сохраняем новый цвет и состояние (LED включен)
    }
}


static void advertising_start(void);

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    app_timer_create(&m_led_state_notify_timer, APP_TIMER_MODE_REPEATED, led_state_notify_handler);
    app_timer_create(&m_led_color_notify_timer, APP_TIMER_MODE_REPEATED, led_color_notify_handler);

    app_timer_start(m_led_state_notify_timer, APP_TIMER_TICKS(10000), &m_estc_service);
    app_timer_start(m_led_color_notify_timer, APP_TIMER_TICKS(2000), &m_estc_service);

}

static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
	APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    err_code = estc_ble_service_init(&m_estc_service);
    APP_ERROR_CHECK(err_code);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void application_timers_start(void)
{
}

static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("ADV Event: Start fast advertising");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("ADV Event: idle, no connectable advertising is ongoing");
            sleep_mode_enter();
            break;

        default:
            break;
    }
}

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    ble_estc_service_t *p_service = (ble_estc_service_t *) p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            p_service->conn_handle = BLE_CONN_HANDLE_INVALID;

            NRF_LOG_INFO("Disconnected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            break;

        case BLE_GAP_EVT_CONNECTED:
            p_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle; 

            NRF_LOG_INFO("Connected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);

            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout (conn_handle: %d)", p_ble_evt->evt.gattc_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout (conn_handle: %d)", p_ble_evt->evt.gatts_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;
        default:
            break;
    }
}

static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

static void buttons_leds_init(void)
{
    ret_code_t err_code;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

void init_led_controller(void)
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
    }
    NRF_LOG_INFO("LEDS init");
}

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}

static void advertising_start(void)
{
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    log_init();
    timers_init();
    buttons_leds_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    init_led_controller();
    pwm_init();
    process_led_events();

    NRF_LOG_INFO("ESTC GATT service example started");
    application_timers_start();

    advertising_start();

    for (;;)
    {
        idle_state_handle();
    }
}
