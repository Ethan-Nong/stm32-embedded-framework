#include "bsp_uart_demo.h"

#include <stdio.h>
#include <string.h>

#include "bsp_tick.h"

#define BSP_UART_DEMO_TX_PERIOD_MS     2000U
#define BSP_UART_DEMO_REPORT_PERIOD_MS 5000U

typedef struct
{
    bsp_uart_t test_uart;
    bsp_uart_t debug_uart;
    uint8_t has_test_uart;
    uint8_t has_debug_uart;
    uint8_t ready;
    uint32_t last_tx_ms;
    uint32_t last_report_ms;
    uint32_t rx_byte_count;
} bsp_uart_demo_ctx_t;

static bsp_uart_demo_ctx_t g_bsp_uart_demo_ctx;

static void bsp_uart_demo_log(const char *message)
{
    if ((message == 0) || (g_bsp_uart_demo_ctx.has_debug_uart == 0U))
    {
        return;
    }

    (void)bsp_uart_write(&g_bsp_uart_demo_ctx.debug_uart, message, (uint32_t)strlen(message));
}

static void bsp_uart_demo_on_rx(const bsp_uart_t *uart, uint8_t byte, void *user_data)
{
    char message[64];
    int length;
    bsp_uart_demo_ctx_t *ctx = (bsp_uart_demo_ctx_t *)user_data;

    if ((ctx == 0) || (ctx->has_debug_uart == 0U))
    {
        return;
    }

    ctx->rx_byte_count++;
    length = snprintf(message, sizeof(message),
                      "[bsp_uart] rx uart=%u byte=0x%02X\r\n",
                      (unsigned int)uart->id,
                      (unsigned int)byte);
    if (length > 0)
    {
        (void)bsp_uart_write(&ctx->debug_uart, message, (uint32_t)length);
    }
}

int bsp_uart_demo_init(const bsp_uart_t *test_uart, const bsp_uart_t *debug_uart)
{
    memset(&g_bsp_uart_demo_ctx, 0, sizeof(g_bsp_uart_demo_ctx));

    if (debug_uart != 0)
    {
        g_bsp_uart_demo_ctx.debug_uart = *debug_uart;
        g_bsp_uart_demo_ctx.has_debug_uart = 1U;
    }

    if (test_uart == 0)
    {
        bsp_uart_demo_log("[bsp_uart] test uart missing\r\n");
        return -1;
    }

    g_bsp_uart_demo_ctx.test_uart = *test_uart;
    g_bsp_uart_demo_ctx.has_test_uart = 1U;
    g_bsp_uart_demo_ctx.last_tx_ms = bsp_tick_get_ms();
    g_bsp_uart_demo_ctx.last_report_ms = g_bsp_uart_demo_ctx.last_tx_ms;

    if (bsp_uart_register_rx_callback(&g_bsp_uart_demo_ctx.test_uart, bsp_uart_demo_on_rx, &g_bsp_uart_demo_ctx) != 0)
    {
        bsp_uart_demo_log("[bsp_uart] rx callback register failed\r\n");
        return -1;
    }

    g_bsp_uart_demo_ctx.ready = 1U;
    bsp_uart_demo_log("[bsp_uart] demo ready\r\n");
    return 0;
}

void bsp_uart_demo_poll(void)
{
    static const char tx_message[] = "[bsp_uart] tx demo ping\r\n";
    char report[80];
    int length;
    uint32_t now_ms;

    if ((g_bsp_uart_demo_ctx.ready == 0U) || (g_bsp_uart_demo_ctx.has_test_uart == 0U))
    {
        return;
    }

    now_ms = bsp_tick_get_ms();
    if ((now_ms - g_bsp_uart_demo_ctx.last_tx_ms) >= BSP_UART_DEMO_TX_PERIOD_MS)
    {
        g_bsp_uart_demo_ctx.last_tx_ms = now_ms;
        (void)bsp_uart_write(&g_bsp_uart_demo_ctx.test_uart, tx_message, (uint32_t)(sizeof(tx_message) - 1U));
    }

    if ((g_bsp_uart_demo_ctx.has_debug_uart == 0U) ||
        ((now_ms - g_bsp_uart_demo_ctx.last_report_ms) < BSP_UART_DEMO_REPORT_PERIOD_MS))
    {
        return;
    }

    g_bsp_uart_demo_ctx.last_report_ms = now_ms;
    length = snprintf(report, sizeof(report),
                      "[bsp_uart] uart=%u rx_bytes=%lu\r\n",
                      (unsigned int)g_bsp_uart_demo_ctx.test_uart.id,
                      (unsigned long)g_bsp_uart_demo_ctx.rx_byte_count);
    if (length > 0)
    {
        (void)bsp_uart_write(&g_bsp_uart_demo_ctx.debug_uart, report, (uint32_t)length);
    }
}
