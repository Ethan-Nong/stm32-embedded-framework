#ifndef BOARD_STM32F407_DEVBOARD_BOARD_H
#define BOARD_STM32F407_DEVBOARD_BOARD_H

#include <stdint.h>
#include "bsp_gpio.h"
#include "bsp_exti.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

typedef struct
{
    bsp_gpio_t gpio;
    uint8_t active_low;
} board_led_cfg_t;

typedef struct
{
    bsp_uart_t uart;
} board_uart_cfg_t;

typedef struct
{
    bsp_i2c_t bus;
} board_i2c_cfg_t;

typedef struct
{
    bsp_spi_t bus;
    bsp_gpio_t cs_gpio;
    uint8_t cs_active_low;
} board_spi_cfg_t;

typedef struct
{
    bsp_adc_t adc;
} board_adc_cfg_t;

typedef struct
{
    bsp_timer_t timer;
} board_timer_cfg_t;

typedef struct
{
    bsp_gpio_t gpio;
    uint8_t active_low;
} board_key_cfg_t;

void board_init(void);

const board_led_cfg_t *board_get_status_led_cfg(void);
const board_uart_cfg_t *board_get_debug_uart_cfg(void);
const board_uart_cfg_t *board_get_comm_uart_cfg(void);
const board_i2c_cfg_t *board_get_sensor_i2c_cfg(void);
const board_spi_cfg_t *board_get_storage_spi_cfg(void);
const board_adc_cfg_t *board_get_adc_input_cfg(void);
const board_timer_cfg_t *board_get_tick_timer_cfg(void);
const board_key_cfg_t *board_get_user_key_cfg(void);

#endif
