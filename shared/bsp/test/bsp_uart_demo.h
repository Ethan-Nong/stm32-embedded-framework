#ifndef BASEOS_BSP_UART_DEMO_H
#define BASEOS_BSP_UART_DEMO_H

#include "bsp_uart.h"

int bsp_uart_demo_init(const bsp_uart_t *test_uart, const bsp_uart_t *debug_uart);
void bsp_uart_demo_poll(void);

#endif
