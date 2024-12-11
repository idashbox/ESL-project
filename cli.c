#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"
#include "nrf_error.h"
#include "led_controller.h"
#include "pwm_handler.h"

#define READ_SIZE 1
#define CMD_BUFFER_SIZE 64

#if ESTC_USB_CLI_ENABLED
static char m_rx_buffer[READ_SIZE];
static char command_buffer[CMD_BUFFER_SIZE];
static size_t command_length = 0;

void usb_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_cdc_acm_user_event_t event);
void cli_handle_command(const char *cmd);
void trim_whitespace(char *str);

#define CDC_ACM_COMM_INTERFACE 2
#define CDC_ACM_COMM_EPIN NRF_DRV_USBD_EPIN3
#define CDC_ACM_DATA_INTERFACE 3
#define CDC_ACM_DATA_EPIN NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT NRF_DRV_USBD_EPOUT4

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static void cli_send_response(const char *response)
{
    ret_code_t ret = app_usbd_cdc_acm_write(&usb_cdc_acm, response, strlen(response));
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Failed to send response: %d", ret);
    }
}

void trim_whitespace(char *str)
{
    char *end;

    while (*str == ' ')
        str++;

    end = str + strlen(str) - 1;
    while (end > str && *end == ' ')
        end--;

    *(end + 1) = '\0';
}

void usb_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        NRF_LOG_INFO("USB port opened.");
        app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        break;

    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
        NRF_LOG_INFO("USB port closed.");
        command_length = 0;
        break;

    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        for (size_t i = 0; i < app_usbd_cdc_acm_rx_size(&usb_cdc_acm); i++)
        {
            char c = m_rx_buffer[i];

            if (c == '\r' || c == '\n')
            {
                if (command_length > 0)
                {
                    command_buffer[command_length] = '\0';
                    NRF_LOG_INFO("Received command: %s", command_buffer);
                    cli_handle_command(command_buffer);
                    command_length = 0;
                }
                cli_send_response("\r\n");
            }
            else if (command_length < CMD_BUFFER_SIZE - 1)
            {
                command_buffer[command_length++] = c;
            }
            else
            {
                cli_send_response("Error: Command buffer overflow.\r\n");
                command_length = 0;
            }
        }
        app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        break;

    default:
        break;
    }
}


void cli_handle_command(const char *cmd)
{
    char response[128];
    trim_whitespace((char *)cmd);

    if (strncmp(cmd, "help", 4) == 0)
    {
        cli_send_response("Commands:\r\nRGB <r> <g> <b>\r\nHSV <h> <s> <v>\r\nhelp\r\n");
    }
    else if (strncmp(cmd, "RGB", 3) == 0)
    {
        int r, g, b;
        int parsed = sscanf(cmd + 4, "%d %d %d", &r, &g, &b);
        NRF_LOG_INFO("Parsed RGB: r=%d, g=%d, b=%d, parsed=%d", r, g, b, parsed);
        if (parsed == 3)
        {
            pwm_set_duty_cycle(1, r);
            pwm_set_duty_cycle(2, g);
            pwm_set_duty_cycle(3, b);
            snprintf(response, sizeof(response), "RGB set to %d, %d, %d\r\n", r, g, b);
            cli_send_response(response);
        }
        else
        {
            cli_send_response("Invalid RGB command.\r\n");
        }
    }
    else if (strncmp(cmd, "HSV", 3) == 0)
    {
        int h, s, v;
        int parsed = sscanf(cmd + 4, "%d %d %d", &h, &s, &v);
        NRF_LOG_INFO("Parsed HSV: h=%.1d, s=%.1d, v=%.1d, parsed=%d", h, s, v, parsed);
        if (parsed == 3)
        {
            uint8_t r, g, b;
            hsv_to_rgb(h, s, v, &r, &g, &b);
            pwm_set_duty_cycle(1, r);
            pwm_set_duty_cycle(2, g);
            pwm_set_duty_cycle(3, b);
            snprintf(response, sizeof(response), "HSV set to %.1d, %.1d, %.1d\r\n", h, s, v);
            cli_send_response(response);
        }
        else
        {
            cli_send_response("Invalid HSV command.\r\n");
        }
    }
    else
    {
        cli_send_response("Unknown command.\r\n");
    }
}

void cli_init(void)
{
    app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    NRF_LOG_INFO("CLI initialized.");
}
#endif