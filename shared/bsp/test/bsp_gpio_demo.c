#include "bsp_gpio_demo.h"

#include <stdio.h>
#include <string.h>

#include "bsp_tick.h"

#define BSP_GPIO_DEMO_TOGGLE_MS 500U

typedef struct
{
    bsp_gpio_t test_gpio;
    bsp_uart_t debug_uart;
    uint8_t has_gpio;
    uint8_t has_debug_uart;
    uint8_t ready;
    uint8_t level_high;
    uint32_t last_toggle_ms;
} bsp_gpio_demo_ctx_t;

static bsp_gpio_demo_ctx_t g_bsp_gpio_demo_ctx;

static void bsp_gpio_demo_log_state(void)
{
    char message[80];
    int length;

    if (g_bsp_gpio_demo_ctx.has_debug_uart == 0U)
    {
        return;
    }

    length = snprintf(message, sizeof(message),
                      "[bsp_gpio] toggle port=%u pin=0x%04X level=%s\r\n",
                      (unsigned int)g_bsp_gpio_demo_ctx.test_gpio.port,
                      (unsigned int)g_bsp_gpio_demo_ctx.test_gpio.pin,
                      (g_bsp_gpio_demo_ctx.level_high != 0U) ? "HIGH" : "LOW");
    if (length > 0)
    {
        (void)bsp_uart_write(&g_bsp_gpio_demo_ctx.debug_uart, message, (uint32_t)length);
    }
}

int bsp_gpio_demo_init(const bsp_gpio_t *test_gpio, const bsp_uart_t *debug_uart)
{
    memset(&g_bsp_gpio_demo_ctx, 0, sizeof(g_bsp_gpio_demo_ctx));

    if (debug_uart != 0)
    {
        g_bsp_gpio_demo_ctx.debug_uart = *debug_uart;
        g_bsp_gpio_demo_ctx.has_debug_uart = 1U;
    }

    if (test_gpio == 0)
    {
        if (g_bsp_gpio_demo_ctx.has_debug_uart != 0U)
        {
            static const char message[] = "[bsp_gpio] test gpio missing\r\n";
            (void)bsp_uart_write(&g_bsp_gpio_demo_ctx.debug_uart, message, (uint32_t)(sizeof(message) - 1U));
        }
        return -1;
    }

    g_bsp_gpio_demo_ctx.test_gpio = *test_gpio;
    g_bsp_gpio_demo_ctx.has_gpio = 1U;
    g_bsp_gpio_demo_ctx.ready = 1U;
    g_bsp_gpio_demo_ctx.last_toggle_ms = bsp_tick_get_ms();
    g_bsp_gpio_demo_ctx.level_high = 0U;
    bsp_gpio_write(&g_bsp_gpio_demo_ctx.test_gpio, BSP_GPIO_LEVEL_LOW);
    bsp_gpio_demo_log_state();
    return 0;
}

void bsp_gpio_demo_poll(void)
{
    uint32_t now_ms;

    if ((g_bsp_gpio_demo_ctx.ready == 0U) || (g_bsp_gpio_demo_ctx.has_gpio == 0U))
    {
        return;
    }

    now_ms = bsp_tick_get_ms();
    if ((now_ms - g_bsp_gpio_demo_ctx.last_toggle_ms) < BSP_GPIO_DEMO_TOGGLE_MS)
    {
        return;
    }

    g_bsp_gpio_demo_ctx.last_toggle_ms = now_ms;
    g_bsp_gpio_demo_ctx.level_high = (uint8_t)(g_bsp_gpio_demo_ctx.level_high == 0U);
    bsp_gpio_write(&g_bsp_gpio_demo_ctx.test_gpio,
                   (g_bsp_gpio_demo_ctx.level_high != 0U) ? BSP_GPIO_LEVEL_HIGH : BSP_GPIO_LEVEL_LOW);
    bsp_gpio_demo_log_state();
}
