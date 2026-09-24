#include "bsp_adc_demo.h"

#include <stdio.h>
#include <string.h>

#include "bsp_tick.h"

#define BSP_ADC_DEMO_SAMPLE_MS 1000U

typedef struct
{
    bsp_adc_t test_adc;
    bsp_uart_t debug_uart;
    uint8_t has_adc;
    uint8_t has_debug_uart;
    uint8_t ready;
    uint32_t last_sample_ms;
} bsp_adc_demo_ctx_t;

static bsp_adc_demo_ctx_t g_bsp_adc_demo_ctx;

static void bsp_adc_demo_log(const char *message)
{
    if ((message == 0) || (g_bsp_adc_demo_ctx.has_debug_uart == 0U))
    {
        return;
    }

    (void)bsp_uart_write(&g_bsp_adc_demo_ctx.debug_uart, message, (uint32_t)strlen(message));
}

int bsp_adc_demo_init(const bsp_adc_t *test_adc, const bsp_uart_t *debug_uart)
{
    memset(&g_bsp_adc_demo_ctx, 0, sizeof(g_bsp_adc_demo_ctx));

    if (debug_uart != 0)
    {
        g_bsp_adc_demo_ctx.debug_uart = *debug_uart;
        g_bsp_adc_demo_ctx.has_debug_uart = 1U;
    }

    if (test_adc == 0)
    {
        bsp_adc_demo_log("[bsp_adc] test adc missing\r\n");
        return -1;
    }

    g_bsp_adc_demo_ctx.test_adc = *test_adc;
    g_bsp_adc_demo_ctx.has_adc = 1U;
    g_bsp_adc_demo_ctx.ready = 1U;
    g_bsp_adc_demo_ctx.last_sample_ms = bsp_tick_get_ms();
    bsp_adc_demo_log("[bsp_adc] demo ready\r\n");
    return 0;
}

void bsp_adc_demo_poll(void)
{
    char message[80];
    int length;
    uint32_t now_ms;
    uint32_t value;

    if ((g_bsp_adc_demo_ctx.ready == 0U) ||
        (g_bsp_adc_demo_ctx.has_adc == 0U) ||
        (g_bsp_adc_demo_ctx.has_debug_uart == 0U))
    {
        return;
    }

    now_ms = bsp_tick_get_ms();
    if ((now_ms - g_bsp_adc_demo_ctx.last_sample_ms) < BSP_ADC_DEMO_SAMPLE_MS)
    {
        return;
    }

    g_bsp_adc_demo_ctx.last_sample_ms = now_ms;
    if (bsp_adc_read(&g_bsp_adc_demo_ctx.test_adc, &value) != 0)
    {
        bsp_adc_demo_log("[bsp_adc] sample failed\r\n");
        return;
    }

    length = snprintf(message, sizeof(message),
                      "[bsp_adc] adc=%u value=%lu\r\n",
                      (unsigned int)g_bsp_adc_demo_ctx.test_adc.id,
                      (unsigned long)value);
    if (length > 0)
    {
        (void)bsp_uart_write(&g_bsp_adc_demo_ctx.debug_uart, message, (uint32_t)length);
    }
}
