#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    uint8_t blinks[] = {6, 5, 8, 1};

    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            for (int j = 0; j < blinks[i]; j++)
            {
                bsp_board_led_invert(i);
                nrf_delay_ms(500);
            }
            bsp_board_led_off(i);
            nrf_delay_ms(1000);
        }
    }
}
