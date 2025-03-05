#include "estc_service.h"
#include <string.h>
#include "nrf_log.h"
#include "ble_srv_common.h"
#include "nrf_ble_gatt.h"
#include "app_error.h"

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

    ble_gatts_char_md_t char_md = {0};
    ble_gatts_attr_md_t attr_md = {0};
    ble_gatts_attr_t attr_char_value = {0};
    ble_uuid_t char_uuid;

    char_uuid.type = p_service->uuid_type;
    char_uuid.uuid = CUSTOM_CHAR_UUID;

    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    attr_md.read_perm = (ble_gap_conn_sec_mode_t){1, 1};
    attr_md.write_perm = (ble_gap_conn_sec_mode_t){1, 1};
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    attr_char_value.p_uuid = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = sizeof(uint32_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = sizeof(uint32_t);
    attr_char_value.p_value = (uint8_t *) &char_value;

    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle, &char_md, &attr_char_value, &p_service->char_handles);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Custom BLE service initialized.");
    return NRF_SUCCESS;
}
