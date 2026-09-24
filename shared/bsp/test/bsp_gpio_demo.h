#ifndef BASEOS_BSP_GPIO_DEMO_H
#define BASEOS_BSP_GPIO_DEMO_H

#include "bsp_gpio.h"
#include "bsp_uart.h"

int bsp_gpio_demo_init(const bsp_gpio_t *test_gpio, const bsp_uart_t *debug_uart);
void bsp_gpio_demo_poll(void);

#endif
