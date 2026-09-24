#ifndef BASEOS_BSP_ADC_DEMO_H
#define BASEOS_BSP_ADC_DEMO_H

#include "bsp_adc.h"
#include "bsp_uart.h"

int bsp_adc_demo_init(const bsp_adc_t *test_adc, const bsp_uart_t *debug_uart);
void bsp_adc_demo_poll(void);

#endif
