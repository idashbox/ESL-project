#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>
#include <stdlib.h>  
#include "ble.h"
#include "sdk_errors.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "app_timer.h"

// Custom Service UUIDs
#define CUSTOM_SERVICE_UUID_BASE { 0xE8, 0x66, 0x78, 0x47, 0x5A, 0xDB, 0x4A, 0xAC, 0xB5, 0x6F, 0xB8, 0x81, 0x37, 0x51, 0x29, 0x3B }
#define CUSTOM_SERVICE_UUID 0x1400
#define CUSTOM_CHAR_LED_STATE_UUID 0x1403
#define CUSTOM_CHAR_LED_COLOR_UUID 0x1404

#define CHAR_PROP_READ  1
#define CHAR_PROP_WRITE 1

typedef struct
{
    uint16_t service_handle;
    ble_gatts_char_handles_t led_state_char_handles;
    ble_gatts_char_handles_t led_color_char_handles;
    uint8_t led_state;
    uint32_t led_color;
    uint8_t uuid_type;
    uint16_t conn_handle;
} ble_estc_service_t;

ret_code_t estc_ble_service_init(ble_estc_service_t *service);

#endif /* ESTC_SERVICE_H__ */
