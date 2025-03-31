#include "estc_service.h"
#include <string.h>
#include "nrf_log.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "flash_storage.h"

// Global variable to hold LED state and color data
static uint32_t led_color = 0;  // RGB color as a 32-bit value (e.g., 0xFF0000 for red)
static uint8_t led_state = 0;   // 0 = off, 1 = on

// Initialize the custom BLE service
uint32_t estc_ble_service_init(ble_estc_service_t *p_service)
{
    if (p_service == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t err_code;
    ble_uuid_t service_uuid;
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};

    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_service->uuid_type);
    APP_ERROR_CHECK(err_code);

    service_uuid.type = p_service->uuid_type;
    service_uuid.uuid = CUSTOM_SERVICE_UUID;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &p_service->service_handle);
    APP_ERROR_CHECK(err_code);

    // Add LED state characteristic
    ble_gatts_char_md_t led_state_char_md = {0};
    ble_gatts_attr_md_t led_state_attr_md = {0};
    ble_gatts_attr_t led_state_attr_char_value = {0};
    ble_uuid_t led_state_char_uuid;

    led_state_char_uuid.type = p_service->uuid_type;
    led_state_char_uuid.uuid = CUSTOM_CHAR_LED_STATE_UUID;

    led_state_char_md.char_props.read = CHAR_PROP_READ;
    led_state_char_md.char_props.write = CHAR_PROP_WRITE;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&led_state_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&led_state_attr_md.write_perm);
    led_state_attr_md.vloc = BLE_GATTS_VLOC_USER;

    led_state_attr_char_value.p_uuid = &led_state_char_uuid;
    led_state_attr_char_value.p_attr_md = &led_state_attr_md;
    led_state_attr_char_value.init_len = sizeof(uint8_t);
    led_state_attr_char_value.p_value = (uint8_t *)&led_state;

    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle, &led_state_char_md, &led_state_attr_char_value, &p_service->led_state_char_handles);
    APP_ERROR_CHECK(err_code);

    // Add LED color characteristic
    ble_gatts_char_md_t led_color_char_md = {0};
    ble_gatts_attr_md_t led_color_attr_md = {0};
    ble_gatts_attr_t led_color_attr_char_value = {0};
    ble_uuid_t led_color_char_uuid;

    led_color_char_uuid.type = p_service->uuid_type;
    led_color_char_uuid.uuid = CUSTOM_CHAR_LED_COLOR_UUID;

    led_color_char_md.char_props.read = CHAR_PROP_READ;
    led_color_char_md.char_props.write = CHAR_PROP_WRITE;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&led_color_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&led_color_attr_md.write_perm);
    led_color_attr_md.vloc = BLE_GATTS_VLOC_USER;

    led_color_attr_char_value.p_uuid = &led_color_char_uuid;
    led_color_attr_char_value.p_attr_md = &led_color_attr_md;
    led_color_attr_char_value.init_len = sizeof(uint32_t); // RGB value
    led_color_attr_char_value.p_value = (uint8_t *)&led_color;

    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle, &led_color_char_md, &led_color_attr_char_value, &p_service->led_color_char_handles);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Custom BLE service initialized with LED state and color characteristics.");

    return NRF_SUCCESS;
}
