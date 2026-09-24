#include "bsp_soft_i2c_demo.h"

#include <stdio.h>
#include <string.h>

#include "bsp_tick.h"

#define BSP_SOFT_I2C_DEMO_PROBE_INTERVAL_MS 10U

typedef struct
{
    bsp_soft_i2c_t test_bus;
    bsp_uart_t debug_uart;
    uint32_t last_probe_ms;
    uint8_t address_start;
    uint8_t address_end;
    uint8_t current_address;
    uint8_t ack_count;
    uint8_t has_debug_uart;
    uint8_t ready;
} bsp_soft_i2c_demo_ctx_t;

static bsp_soft_i2c_demo_ctx_t g_bsp_soft_i2c_demo_ctx;

static void bsp_soft_i2c_demo_log(const char *message)
{
    if ((message == 0) || (g_bsp_soft_i2c_demo_ctx.has_debug_uart == 0U))
    {
        return;
    }

    (void)bsp_uart_write(&g_bsp_soft_i2c_demo_ctx.debug_uart, message, (uint32_t)strlen(message));
}

static void bsp_soft_i2c_demo_log_probe(uint8_t address)
{
    char message[48];
    int length;

    if (g_bsp_soft_i2c_demo_ctx.has_debug_uart == 0U)
    {
        return;
    }

    length = snprintf(message, sizeof(message), "[bsp_soft_i2c] addr=0x%02X ACK\r\n", (unsigned int)address);
    if (length > 0)
    {
        (void)bsp_uart_write(&g_bsp_soft_i2c_demo_ctx.debug_uart, message, (uint32_t)length);
    }
}

static void bsp_soft_i2c_demo_log_summary(void)
{
    char message[56];
    int length;

    if (g_bsp_soft_i2c_demo_ctx.has_debug_uart == 0U)
    {
        return;
    }

    length = snprintf(message, sizeof(message),
                      "[bsp_soft_i2c] scan complete ack_count=%u\r\n",
                      (unsigned int)g_bsp_soft_i2c_demo_ctx.ack_count);
    if (length > 0)
    {
        (void)bsp_uart_write(&g_bsp_soft_i2c_demo_ctx.debug_uart, message, (uint32_t)length);
    }
}

int bsp_soft_i2c_demo_init(const bsp_soft_i2c_t *test_bus,
                           const bsp_uart_t *debug_uart,
                           uint8_t address_start,
                           uint8_t address_end)
{
    int ret;

    memset(&g_bsp_soft_i2c_demo_ctx, 0, sizeof(g_bsp_soft_i2c_demo_ctx));

    if (debug_uart != 0)
    {
        g_bsp_soft_i2c_demo_ctx.debug_uart = *debug_uart;
        g_bsp_soft_i2c_demo_ctx.has_debug_uart = 1U;
    }

    if ((test_bus == 0) || (address_start > address_end) || (address_end > 0x7FU))
    {
        bsp_soft_i2c_demo_log("[bsp_soft_i2c] invalid demo config\r\n");
        return BSP_SOFT_I2C_EINVAL;
    }

    g_bsp_soft_i2c_demo_ctx.test_bus = *test_bus;
    ret = bsp_soft_i2c_init(&g_bsp_soft_i2c_demo_ctx.test_bus);
    if (ret != BSP_SOFT_I2C_OK)
    {
        bsp_soft_i2c_demo_log("[bsp_soft_i2c] bus init failed\r\n");
        return ret;
    }

    g_bsp_soft_i2c_demo_ctx.address_start = address_start;
    g_bsp_soft_i2c_demo_ctx.address_end = address_end;
    g_bsp_soft_i2c_demo_ctx.current_address = address_start;
    g_bsp_soft_i2c_demo_ctx.last_probe_ms = bsp_tick_get_ms();
    g_bsp_soft_i2c_demo_ctx.ready = 1U;
    bsp_soft_i2c_demo_log("[bsp_soft_i2c] scan demo ready\r\n");
    return BSP_SOFT_I2C_OK;
}

void bsp_soft_i2c_demo_poll(void)
{
    uint32_t now_ms;
    int ret;

    if (g_bsp_soft_i2c_demo_ctx.ready == 0U)
    {
        return;
    }

    now_ms = bsp_tick_get_ms();
    if ((uint32_t)(now_ms - g_bsp_soft_i2c_demo_ctx.last_probe_ms) < BSP_SOFT_I2C_DEMO_PROBE_INTERVAL_MS)
    {
        return;
    }

    g_bsp_soft_i2c_demo_ctx.last_probe_ms = now_ms;
    ret = bsp_soft_i2c_probe(&g_bsp_soft_i2c_demo_ctx.test_bus,
                             g_bsp_soft_i2c_demo_ctx.current_address);
    if (ret == BSP_SOFT_I2C_OK)
    {
        g_bsp_soft_i2c_demo_ctx.ack_count++;
        bsp_soft_i2c_demo_log_probe(g_bsp_soft_i2c_demo_ctx.current_address);
    }

    if (g_bsp_soft_i2c_demo_ctx.current_address >= g_bsp_soft_i2c_demo_ctx.address_end)
    {
        bsp_soft_i2c_demo_log_summary();
        g_bsp_soft_i2c_demo_ctx.current_address = g_bsp_soft_i2c_demo_ctx.address_start;
        g_bsp_soft_i2c_demo_ctx.ack_count = 0U;
        return;
    }

    g_bsp_soft_i2c_demo_ctx.current_address++;
}
