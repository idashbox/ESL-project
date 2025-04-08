#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "sdk_errors.h"
#include "ble_srv_common.h"

#define CUSTOM_SERVICE_UUID_BASE { 0xE8, 0x66, 0x78, 0x47, 0x5A, 0xDB, 0x4A, 0xAC, 0xB5, 0x6F, 0xB8, 0x81, 0x37, 0x51, 0x29, 0x3B }
#define CUSTOM_SERVICE_UUID        0x1400
#define LED_COLOR_CHAR_UUID        0x1401
#define LED_STATE_CHAR_UUID        0x1402
#define LED_NOTIFY_CHAR_UUID       0x1403

#define LED_COLOR_LEN   3
#define LED_STATE_LEN   1
#define LED_NOTIFY_LEN  16

typedef struct {
    uint16_t service_handle;
    uint16_t conn_handle;
    uint8_t uuid_type;
    ble_gatts_char_handles_t color_handles;
    ble_gatts_char_handles_t state_handles;
    ble_gatts_char_handles_t notify_handles;
} ble_estc_service_t;

ret_code_t ble_service_init(ble_estc_service_t *service, void *ctx);
void ble_service_event(const ble_evt_t *ble_evt, void *ctx);

#endif /* ESTC_SERVICE_H__ */
