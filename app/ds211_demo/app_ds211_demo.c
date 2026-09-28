#include "app_ds211_demo.h"

#include <string.h>

#include "board.h"
#include "bsp_tick.h"
#include "bsp_uart.h"
#include "drv_ds211.h"
#include "project_config.h"

static const board_uart_cfg_t *g_debug_uart;
static ds211_t g_fingerprint;
static volatile uint8_t g_pending_command;
static uint8_t g_last_progress_stage;

static const app_interface_t g_ds211_demo_app = {
    "ds211_demo",
    app_ds211_demo_init,
    app_ds211_demo_loop
};

static void app_uart_write(const char *text)
{
    if ((g_debug_uart == 0) || (text == 0))
    {
        return;
    }

    (void)bsp_uart_write(&g_debug_uart->uart,
                         text,
                         (uint32_t)strlen(text));
}

static void app_uart_write_u32(uint32_t value)
{
    char buffer[11];
    uint8_t index = (uint8_t)sizeof(buffer);

    buffer[--index] = '\0';
    do
    {
        buffer[--index] = (char)('0' + (value % 10U));
        value /= 10U;
    } while ((value != 0U) && (index != 0U));

    app_uart_write(&buffer[index]);
}

static char app_hex_digit(uint8_t value)
{
    value &= 0x0FU;
    return (char)((value < 10U) ? ('0' + value) : ('A' + value - 10U));
}

static void app_uart_write_hex8(uint8_t value)
{
    char text[3];

    text[0] = app_hex_digit((uint8_t)(value >> 4));
    text[1] = app_hex_digit(value);
    text[2] = '\0';
    app_uart_write(text);
}

static void app_write_result(const char *operation, int result)
{
    app_uart_write(operation);
    app_uart_write(": ");
    if (result == DS211_STATUS_OK)
    {
        app_uart_write("OK");
    }
    else if (result < 0)
    {
        app_uart_write("driver error ");
        app_uart_write_u32((uint32_t)(-result));
    }
    else
    {
        app_uart_write("ACK 0x");
        app_uart_write_hex8((uint8_t)result);
        app_uart_write(" (");
        app_uart_write(ds211_ack_string(result));
        app_uart_write(")");
    }
    app_uart_write("\r\n");
}

static void app_print_help(void)
{
    app_uart_write("\r\nDS211 commands:\r\n");
    app_uart_write("  1  handshake\r\n");
    app_uart_write("  2  read system parameters\r\n");
    app_uart_write("  3  auto enroll page 0\r\n");
    app_uart_write("  4  identify fingerprint (1:N)\r\n");
    app_uart_write("  5  clear fingerprint library\r\n");
    app_uart_write("  6  read valid template count\r\n");
    app_uart_write("  h  show this help\r\n");
}

static void app_debug_rx(const bsp_uart_t *uart, uint8_t byte, void *user_data)
{
    (void)uart;
    (void)user_data;

    if ((byte == '\r') || (byte == '\n') || (byte == ' ') || (byte == '\t'))
    {
        return;
    }

    g_pending_command = byte;
}

static void app_enroll_progress(uint8_t stage, uint8_t detail, void *user_data)
{
    (void)user_data;
    if (stage == g_last_progress_stage)
    {
        return;
    }
    g_last_progress_stage = stage;

    switch (stage)
    {
    case DS211_PROGRESS_CAPTURE_IMAGE:
        app_uart_write("Place finger on sensor...\r\n");
        break;
    case DS211_PROGRESS_GENERATE_FEATURE:
        app_uart_write("Generating feature...\r\n");
        break;
    case DS211_PROGRESS_FINGER_LEAVE:
        app_uart_write("Remove finger...\r\n");
        break;
    case DS211_PROGRESS_MERGE_TEMPLATE:
        app_uart_write("Merging template...\r\n");
        break;
    case DS211_PROGRESS_CHECK_DUPLICATE:
        app_uart_write("Checking duplicate...\r\n");
        break;
    case DS211_PROGRESS_STORE_TEMPLATE:
        app_uart_write("Storing template...\r\n");
        break;
    default:
        app_uart_write("Enroll step ");
        app_uart_write_u32(stage);
        app_uart_write(", detail ");
        app_uart_write_u32(detail);
        app_uart_write("\r\n");
        break;
    }
}

static void app_handle_command(uint8_t command)
{
    ds211_sys_params_t params;
    ds211_search_result_t result;
    uint16_t count;
    int status;

    app_uart_write("\r\n");
    switch (command)
    {
    case '1':
        status = ds211_handshake(&g_fingerprint, 1500U);
        app_write_result("Handshake", status);
        break;
    case '2':
        status = ds211_read_sys_params(&g_fingerprint, &params, 1500U);
        app_write_result("Read parameters", status);
        if (status == DS211_ACK_OK)
        {
            app_uart_write("  status: ");
            app_uart_write_u32(params.status);
            app_uart_write("\r\n  sensor type: ");
            app_uart_write_u32(params.sensor_type);
            app_uart_write("\r\n  library size: ");
            app_uart_write_u32(params.library_size);
            app_uart_write("\r\n  score level: ");
            app_uart_write_u32(params.score_level);
            app_uart_write("\r\n  baud rate: ");
            app_uart_write_u32(params.baud_rate_factor * 9600U);
            app_uart_write("\r\n");
        }
        break;
    case '3':
        app_uart_write("Auto enroll page 0, press count ");
        app_uart_write_u32(3U);
        app_uart_write("\r\n");
        g_last_progress_stage = 0U;
        /* bit3: allow overwrite, bit4: reject duplicate fingerprints. */
        status = ds211_auto_enroll(&g_fingerprint,
                                       0U,
                                       3U,
                                       0x0018U,
                                       app_enroll_progress,
                                       0,
                                       120000U);
        app_write_result("Enroll", status);
        if (status == DS211_ACK_OK)
        {
            app_uart_write("Fingerprint stored at page 0.\r\n");
        }
        break;
    case '4':
        /* bit2: do not return intermediate verification steps. */
        status = ds211_auto_identify(&g_fingerprint,
                                         2U,
                                         0xFFFFU,
                                         0x0004U,
                                         &result,
                                         30000U);
        app_write_result("Identify", status);
        if (status == DS211_ACK_OK)
        {
            app_uart_write("  page id: ");
            app_uart_write_u32(result.page_id);
            app_uart_write("\r\n  score: ");
            app_uart_write_u32(result.score);
            app_uart_write("\r\n");
        }
        break;
    case '5':
        status = ds211_clear_library(&g_fingerprint, 3000U);
        app_write_result("Clear library", status);
        break;
    case '6':
        count = 0U;
        status = ds211_get_valid_template_count(&g_fingerprint,
                                                    &count,
                                                    1500U);
        app_write_result("Read template count", status);
        if (status == DS211_ACK_OK)
        {
            app_uart_write("  valid templates: ");
            app_uart_write_u32(count);
            app_uart_write("\r\n");
        }
        break;
    case 'h':
    case 'H':
        app_print_help();
        break;
    default:
        app_uart_write("Unknown command. Send h for help.\r\n");
        break;
    }
}

int app_ds211_demo_init(void)
{
    const board_fingerprint_cfg_t *fingerprint_cfg;
    ds211_cfg_t config;
    int status;

    if ((board_interface_acquire(BOARD_INTERFACE_ID_DEBUG_UART,
                                 BOARD_INTERFACE_MODE_UART) != 0) ||
        (board_interface_acquire(BOARD_INTERFACE_ID_COMM_UART,
                                 BOARD_INTERFACE_MODE_UART) != 0))
    {
        return -1;
    }

    g_debug_uart = board_get_debug_uart_cfg();
    fingerprint_cfg = board_get_fingerprint_cfg();
    if ((g_debug_uart == 0) || (fingerprint_cfg == 0))
    {
        return -1;
    }

    config.uart = fingerprint_cfg->uart;
    config.touch_gpio = fingerprint_cfg->touch_gpio;
    config.has_touch_gpio = fingerprint_cfg->has_touch_gpio;
    config.touch_active_high = fingerprint_cfg->touch_active_high;
    config.device_address = DS211_DEVICE_ADDRESS_DEFAULT;

    (void)bsp_uart_register_rx_callback(&g_debug_uart->uart,
                                        app_debug_rx,
                                        0);

    status = ds211_init(&g_fingerprint, &config);
    app_uart_write("\r\ncore_f407_seed_dev DS211 demo\r\n");
    app_uart_write("DS211 UART: 115200 8N2\r\n");
    app_write_result("Driver init", status);
    app_print_help();
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    bsp_delay_ms(100U);
    status = ds211_handshake(&g_fingerprint, 1500U);
    app_write_result("Startup handshake", status);
    return 0;
}

void app_ds211_demo_loop(void)
{
    uint8_t command = g_pending_command;

    if (command != 0U)
    {
        g_pending_command = 0U;
        app_handle_command(command);
    }
}

const app_interface_t *app_ds211_demo_get_interface(void)
{
    return &g_ds211_demo_app;
}
