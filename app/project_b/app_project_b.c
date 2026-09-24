#include "app_project_b.h"

#include "board.h"
#include "bsp_spi.h"
#include "bsp_tick.h"
#include "bsp_uart.h"
#include "project_config.h"
#include "platform_config.h"

static const board_spi_cfg_t *g_shared_spi;
static const board_uart_cfg_t *g_debug_uart;
static const board_led_cfg_t *g_status_led;

void app_project_b_init(void)
{
    static const char boot_message[] = "[project_b] init\r\n";

    board_init();
    g_shared_spi = board_get_storage_spi_cfg();
    g_debug_uart = board_get_debug_uart_cfg();
    g_status_led = board_get_status_led_cfg();

    if (g_shared_spi != 0)
    {
        (void)bsp_spi_lock(&g_shared_spi->bus, 100U);
        (void)bsp_spi_set_prescaler(&g_shared_spi->bus,
                                    PROJECT_SHARED_SPI_PRESCALER);
        bsp_spi_unlock(&g_shared_spi->bus);
    }

    if (g_debug_uart != 0)
    {
        (void)bsp_uart_write(&g_debug_uart->uart,
                             boot_message,
                             (uint32_t)(sizeof(boot_message) - 1U));
    }
}

void app_project_b_loop(void)
{
    if (g_status_led != 0)
    {
        bsp_gpio_toggle(&g_status_led->gpio);
    }

    bsp_delay_ms(500U);
}
