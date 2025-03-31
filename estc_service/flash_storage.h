#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>

#define FLASH_STORAGE_ADDR (BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE)
#define LED_STATE_FILE_ID 0x1234
#define LED_STATE_REC_KEY 0x5678

void save_led_state_and_color(uint8_t led_state, uint32_t hue, uint32_t saturation, uint32_t brightness);
void load_led_state_and_color(uint8_t *led_state, uint32_t *hue, uint32_t *saturation, uint32_t *brightness);

#endif  // FLASH_STORAGE_H
