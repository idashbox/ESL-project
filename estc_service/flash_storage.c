#include "flash_storage.h"
#include "nrf_nvmc.h"
#include "nrf_log.h"
#include <string.h>

// Функция для сохранения состояния светодиода и его цвета
void save_led_state_and_color(uint8_t led_state, uint32_t hue, uint32_t saturation, uint32_t brightness)
{
    uint32_t color_data[4] = {led_state, hue, saturation, brightness};  // Состояние + 3 параметра цвета

    nrf_nvmc_page_erase(FLASH_STORAGE_ADDR);

    // Запись всех данных в память
    nrf_nvmc_write_words(FLASH_STORAGE_ADDR, color_data, sizeof(color_data) / sizeof(uint32_t));

    NRF_LOG_INFO("LED state and color saved: State: %d, H:%d S:%d B:%d", led_state, hue, saturation, brightness);
}

// Функция для загрузки состояния светодиода и его цвета
void load_led_state_and_color(uint8_t *led_state, uint32_t *hue, uint32_t *saturation, uint32_t *brightness)
{
    uint32_t color_data[4];

    // Чтение данных из памяти
    memcpy(color_data, (void*)FLASH_STORAGE_ADDR, sizeof(color_data));

    // Проверка валидности данных
    if (color_data[1] <= 360 && color_data[2] <= 100 && color_data[3] <= 100)
    {
        *led_state = color_data[0];  // Состояние светодиода
        *hue = color_data[1];        // Hue
        *saturation = color_data[2]; // Saturation
        *brightness = color_data[3]; // Brightness

        NRF_LOG_INFO("Loaded LED state: %d, Color: H:%d S:%d B:%d", *led_state, *hue, *saturation, *brightness);
    }
    else
    {
        // Если данные некорректны, устанавливаем значения по умолчанию
        *led_state = 0;              // Состояние по умолчанию (выключен)
        *hue = 360 * 0.81;          // Значения по умолчанию для цвета
        *saturation = 100;
        *brightness = 100;

        NRF_LOG_INFO("No valid color found, using defaults");
    }
}
