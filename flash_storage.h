#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>

#define FLASH_STORAGE_ADDR (BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE)


void flash_init(void);
void save_color(uint32_t hue, uint32_t saturation, uint32_t brightness);
void load_saved_color(uint32_t *hue, uint32_t *saturation, uint32_t *brightness);

#endif
