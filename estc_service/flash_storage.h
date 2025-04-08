#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include "led_controller.h"

#define CUSTOM_LED_SAVES_FILE_ID 0x1234
#define CUSTOM_LED_SAVES_RECORD_KEY 0x7777

#define CUSTOM_LED_SAVES_FDS_USAGE_LIMIT ((uint32_t) FDS_PHY_PAGE_SIZE * 75 / 100)

void flash_storage_init(led_data_t *data);
void save_led_data(led_data_t *data);
void fds_init_and_register(void);


#endif  // FLASH_STORAGE_H
