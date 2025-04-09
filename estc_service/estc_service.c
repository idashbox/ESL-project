#include "estc_service.h"
#include "nrf_log.h"
#include "app_error.h"
#include "app_timer.h"
#include "led_controller.h"
#include "flash_storage.h"
#include <string.h>

extern ble_estc_service_t m_estc_service;
APP_TIMER_DEF(m_notify_color_timer);
APP_TIMER_DEF(m_notify_led_state_timer);

static const ble_uuid128_t base_uuid = { CUSTOM_SERVICE_UUID_BASE };
static led_data_t led_data = { .color = {0xdd, 0xaa, 0x00}, .led_state = 1 };

static ret_code_t add_char(ble_estc_service_t *service,
                           uint16_t uuid,
                           ble_gatts_char_handles_t *handles,
                           bool read,
                           bool write,
                           bool notify,
                           uint16_t len) {
    ble_uuid_t char_uuid = { .uuid = uuid, .type = service->uuid_type };
    ble_gatts_char_md_t char_md = {0};
    ble_gatts_attr_md_t attr_md = {0};
    ble_gatts_attr_t attr_value = {0};

    char_md.char_props.read   = read;
    char_md.char_props.write  = write;
    char_md.char_props.notify = notify;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    attr_value.p_uuid = &char_uuid;
    attr_value.p_attr_md = &attr_md;
    attr_value.init_len = len;
    attr_value.max_len = len;
    attr_value.p_value = NULL;

    return sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_value, handles);
}

static void notify_color_handler() {
    uint8_t buf[3];
    buf[0] = led_data.color.r;
    buf[1] = led_data.color.g;
    buf[2] = led_data.color.b;

    uint16_t len = sizeof(buf);
    ble_gatts_hvx_params_t params = {
        .handle = m_estc_service.color_handles.value_handle,
        .type = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_len = &len,
        .p_data = (uint8_t *) buf
    };
    sd_ble_gatts_hvx(m_estc_service.conn_handle, &params);
    NRF_LOG_INFO("Color Notify");
}

static void notify_led_state_handler() {
    char buf[LED_STATE_LEN];
    snprintf(buf, sizeof(buf), "%3d",led_data.led_state);

    uint16_t len = strlen(buf);
    ble_gatts_hvx_params_t params = {
        .handle = m_estc_service.state_handles.value_handle,
        .type = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_len = &len,
        .p_data = (uint8_t *) buf
    };
    sd_ble_gatts_hvx(m_estc_service.conn_handle, &params);
    NRF_LOG_INFO("LED state Notify");
}

ret_code_t ble_service_init(ble_estc_service_t *service) {
    ret_code_t err;

    err = sd_ble_uuid_vs_add(&base_uuid, &service->uuid_type);
    APP_ERROR_CHECK(err);

    ble_uuid_t uuid = { .uuid = CUSTOM_SERVICE_UUID, .type = service->uuid_type };
    err = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &service->service_handle);
    APP_ERROR_CHECK(err);

    err = add_char(service, LED_COLOR_CHAR_UUID, &service->color_handles, true, true, true, LED_COLOR_LEN);
    APP_ERROR_CHECK(err);
    err = add_char(service, LED_STATE_CHAR_UUID, &service->state_handles, true, true, true, LED_STATE_LEN);
    APP_ERROR_CHECK(err);

    app_timer_create(&m_notify_color_timer, APP_TIMER_MODE_REPEATED, notify_color_handler);
    app_timer_create(&m_notify_led_state_timer, APP_TIMER_MODE_REPEATED, notify_led_state_handler);

    return NRF_SUCCESS;
}

static void handle_color_write(const uint8_t *data, uint16_t len, bool connected) {
    if (len != LED_COLOR_LEN) return;
    memcpy(&led_data.color, data, LED_COLOR_LEN);
    led_update(&led_data);

    if (connected) {
        ble_gatts_value_t val = { .len = LED_COLOR_LEN, .offset = 0, .p_value = (uint8_t *)&led_data.color };
        sd_ble_gatts_value_set(m_estc_service.conn_handle, m_estc_service.color_handles.value_handle, &val);
    } else {
        save_led_data(&led_data);
        app_timer_start(m_notify_color_timer, APP_TIMER_TICKS(1000), &m_estc_service);
    }
}

static void handle_state_write(const uint8_t *data, uint16_t len, bool connected) {
    if (len != LED_STATE_LEN) return;
    led_data.led_state = *data;
    led_update(&led_data);

    if (connected) {
        ble_gatts_value_t val = { .len = LED_STATE_LEN, .offset = 0, .p_value = (uint8_t *)&led_data.led_state };
        sd_ble_gatts_value_set(m_estc_service.conn_handle, m_estc_service.state_handles.value_handle, &val);
    } else {
        save_led_data(&led_data);
        app_timer_start(m_notify_led_state_timer, APP_TIMER_TICKS(1000), &m_estc_service);
    }
}

void ble_service_event(const ble_evt_t *ble_evt) {
    switch (ble_evt->header.evt_id) {
        case BLE_GATTS_EVT_WRITE: {
            const ble_gatts_evt_write_t *w = &ble_evt->evt.gatts_evt.params.write;

            if (w->handle == m_estc_service.color_handles.value_handle)
                handle_color_write(w->data, w->len, false);

            if (w->handle == m_estc_service.state_handles.value_handle)
                handle_state_write(w->data, w->len, false);
            break;
        }

        case BLE_GAP_EVT_CONNECTED:
            handle_color_write((uint8_t *)&led_data.color, LED_COLOR_LEN, true);
            handle_state_write((uint8_t *)&led_data.led_state, LED_STATE_LEN, true);
            break;

        default:
            break;
    }
}
