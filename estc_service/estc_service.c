#include "estc_service.h"
#include <string.h>
#include "nrf_log.h"
#include "ble_srv_common.h"
#include "nrf_ble_gatt.h"
#include "app_error.h"
#include "app_timer.h"

static uint32_t char_value = 0;

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

    //Notify
    ble_gatts_char_md_t notify_char_md = {0};
    ble_gatts_attr_md_t notify_attr_md = {0};
    ble_gatts_attr_t notify_attr_char_value = {0};
    ble_uuid_t notify_char_uuid;

    notify_char_uuid.type = p_service->uuid_type;
    notify_char_uuid.uuid = CUSTOM_CHAR_NOTIFY_UUID;

    notify_char_md.char_props.notify = 1;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&notify_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&notify_attr_md.write_perm);
    notify_attr_md.vloc = BLE_GATTS_VLOC_USER;

    notify_attr_char_value.p_uuid = &notify_char_uuid;
    notify_attr_char_value.p_attr_md = &notify_attr_md;
    notify_attr_char_value.init_len = sizeof(uint32_t);
    notify_attr_char_value.init_offs = 0;
    notify_attr_char_value.max_len = sizeof(uint32_t);
    notify_attr_char_value.p_value = (uint8_t *) &char_value;

    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle, &notify_char_md, &notify_attr_char_value, &p_service->notify_char_handles);
    APP_ERROR_CHECK(err_code);

    //Indicate
    ble_gatts_char_md_t indicate_char_md = {0};
    ble_gatts_attr_md_t indicate_attr_md = {0};
    ble_gatts_attr_t indicate_attr_char_value = {0};
    ble_uuid_t indicate_char_uuid;

    indicate_char_uuid.type = p_service->uuid_type;
    indicate_char_uuid.uuid = CUSTOM_CHAR_INDICATE_UUID;

    indicate_char_md.char_props.indicate = 1;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&indicate_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&indicate_attr_md.write_perm);
    indicate_attr_md.vloc = BLE_GATTS_VLOC_USER;

    indicate_attr_char_value.p_uuid = &indicate_char_uuid;
    indicate_attr_char_value.p_attr_md = &indicate_attr_md;
    indicate_attr_char_value.init_len = sizeof(uint32_t);
    indicate_attr_char_value.init_offs = 0;
    indicate_attr_char_value.max_len = sizeof(uint32_t);
    indicate_attr_char_value.p_value = (uint8_t *) &char_value;

    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle, &indicate_char_md, &indicate_attr_char_value, &p_service->indicate_char_handles);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Custom BLE service initialized with Notify and Indicate characteristics.");
    return NRF_SUCCESS;
}
