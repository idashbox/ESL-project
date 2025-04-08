#include <string.h>
#include "app_error.h"
#include "fds.h"
#include "fds_internal_defs.h"
#include "flash_storage.h"

led_data_t data = {
    .color = (rgb_t) {0xdd, 0xaa, 0x00},
    .led_state = 0x00
};

void display_storage_state(void)
{
    fds_stat_t stat;
    fds_stat(&stat);
}

void flash_storage_init(led_data_t *led_data)
{
    fds_record_desc_t record_desc;
    fds_find_token_t record_token;
    fds_flash_record_t flash_record;

    memset(&record_token, 0, sizeof(fds_find_token_t));

    if (fds_record_find(CUSTOM_LED_SAVES_FILE_ID, CUSTOM_LED_SAVES_RECORD_KEY, &record_desc, &record_token) == NRF_SUCCESS)
    {
        if (fds_record_open(&record_desc, &flash_record) == NRF_SUCCESS)
        {
            memcpy(led_data, flash_record.p_data, sizeof(led_data_t));
            fds_record_close(&record_desc);
        }
    }

    led_update(led_data);
}

void check_and_trigger_gc(void)
{
    fds_stat_t stat;

    if (fds_stat(&stat) != NRF_SUCCESS)
    {
        return;
    }

    if (stat.freeable_words > CUSTOM_LED_SAVES_FDS_USAGE_LIMIT)
    {
        fds_gc();
    }
}

void save_led_data(led_data_t *led_data)
{
    fds_record_desc_t record_desc;
    fds_find_token_t record_token;
    fds_record_t record;

    ret_code_t ret_code;

    memset(&record_token, 0, sizeof(fds_find_token_t));

    record.file_id = CUSTOM_LED_SAVES_FILE_ID;
    record.key = CUSTOM_LED_SAVES_RECORD_KEY;
    record.data.p_data = (void *) &led_data;
    record.data.length_words = sizeof(led_data_t) / 4;

    if (fds_record_find(record.file_id, record.key, &record_desc, &record_token))
    {
        ret_code = fds_record_update(&record_desc, &record);
    }
    else
    {
        ret_code = fds_record_write(&record_desc, &record);
    }

    if (ret_code != NRF_SUCCESS)
    {
        fds_gc();
    }

    display_storage_state();
    check_and_trigger_gc();
}

void fds_events_handler(fds_evt_t const * p_evt)
{
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            flash_storage_init(&data);
            break;

        case FDS_EVT_GC:
            display_storage_state();
            break;

        case FDS_EVT_DEL_FILE:
            break;

        default:
            break;
    }
}

void fds_init_and_register(void)
{
    fds_register(fds_events_handler);
    fds_init();
}