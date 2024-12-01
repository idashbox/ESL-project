#include "flash_storage.h"
#include "nrf_nvmc.h"
#include "nrf_log.h"
#include <string.h>

void save_color(uint32_t hue, uint32_t saturation, uint32_t brightness)
{
    uint32_t color_data[3] = {hue, saturation, brightness};

    nrf_nvmc_page_erase(FLASH_STORAGE_ADDR);

    nrf_nvmc_write_words(FLASH_STORAGE_ADDR, color_data, sizeof(color_data) / sizeof(uint32_t));

    NRF_LOG_INFO("Color saved: H:%d S:%d B:%d", hue, saturation, brightness);
}

void load_saved_color(uint32_t *hue, uint32_t *saturation, uint32_t *brightness)
{
    uint32_t color_data[3];
    memcpy(color_data, (void*)FLASH_STORAGE_ADDR, sizeof(color_data));

    if (color_data[0] <= 360 && color_data[1] <= 100 && color_data[2] <= 100)
    {
        *hue = color_data[0];
        *saturation = color_data[1];
        *brightness = color_data[2];
        NRF_LOG_INFO("Loaded color: H:%d S:%d B:%d", *hue, *saturation, *brightness);
    }
    else
    {
        *hue = 360 * 0.81;
        *saturation = 100;
        *brightness = 100;
        NRF_LOG_INFO("No valid color found, using defaults");
    }
}
