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

typedef struct
{
    bsp_uart_t uart;
    bsp_gpio_t touch_gpio;
    uint8_t has_touch_gpio;
    uint8_t touch_active_high;
} board_fingerprint_cfg_t;

typedef enum
{
    BOARD_INTERFACE_MODE_NONE = 0,
    BOARD_INTERFACE_MODE_GPIO,
    BOARD_INTERFACE_MODE_UART,
    BOARD_INTERFACE_MODE_SPI,
    BOARD_INTERFACE_MODE_I2C,
    BOARD_INTERFACE_MODE_COUNT
} board_interface_mode_t;

typedef enum
{
    BOARD_INTERFACE_ID_NONE = 0,
    BOARD_INTERFACE_ID_DEBUG_UART,
    BOARD_INTERFACE_ID_COMM_UART,
    BOARD_INTERFACE_ID_SENSOR_I2C,
    BOARD_INTERFACE_ID_STORAGE_SPI,
    BOARD_INTERFACE_ID_COUNT
} board_interface_id_t;

typedef enum
{
    BOARD_PIN_FUNCTION_GPIO = 0,
    BOARD_PIN_FUNCTION_UART_TX,
    BOARD_PIN_FUNCTION_UART_RX,
    BOARD_PIN_FUNCTION_SPI_SCK,
    BOARD_PIN_FUNCTION_SPI_MISO,
    BOARD_PIN_FUNCTION_SPI_MOSI,
    BOARD_PIN_FUNCTION_SPI_CS,
    BOARD_PIN_FUNCTION_I2C_SCL,
    BOARD_PIN_FUNCTION_I2C_SDA
} board_pin_function_t;

typedef struct
{
    bsp_gpio_t gpio;
    board_pin_function_t function;
    bsp_gpio_mode_t gpio_mode;
    bsp_gpio_pull_t pull;
    bsp_gpio_speed_t speed;
    uint8_t alternate_function;
} board_pin_cfg_t;

typedef struct
{
    board_interface_mode_t mode;
    const board_pin_cfg_t *pins;
    uint8_t pin_count;
} board_interface_mode_cfg_t;

typedef struct
{
    board_interface_id_t id;
    const char *name;
    const board_interface_mode_cfg_t *mode_cfgs;
    uint8_t mode_count;
    const board_uart_cfg_t *uart;
    const board_i2c_cfg_t *i2c;
    const board_spi_cfg_t *spi;
    board_interface_mode_t default_mode;
} board_interface_cfg_t;

void board_init(void);

const board_led_cfg_t *board_get_status_led_cfg(void);
const board_uart_cfg_t *board_get_debug_uart_cfg(void);
const board_uart_cfg_t *board_get_comm_uart_cfg(void);
const board_i2c_cfg_t *board_get_sensor_i2c_cfg(void);
const board_spi_cfg_t *board_get_storage_spi_cfg(void);
const board_adc_cfg_t *board_get_adc_input_cfg(void);
const board_timer_cfg_t *board_get_tick_timer_cfg(void);
const board_key_cfg_t *board_get_user_key_cfg(void);
const board_fingerprint_cfg_t *board_get_fingerprint_cfg(void);

const board_interface_cfg_t *board_get_interface_cfg(board_interface_id_t id);
const board_interface_mode_cfg_t *board_get_interface_mode_cfg(
    board_interface_id_t id,
    board_interface_mode_t mode);
int board_interface_is_mode_supported(board_interface_id_t id,
                                      board_interface_mode_t mode);
int board_interface_acquire(board_interface_id_t id, board_interface_mode_t mode);
int board_interface_release(board_interface_id_t id);
int board_interface_set_mode(board_interface_id_t id, board_interface_mode_t mode);
board_interface_mode_t board_interface_get_mode(board_interface_id_t id);

#endif
