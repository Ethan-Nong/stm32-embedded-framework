#ifndef BOARD_CORE_F407_SEED_DEV_CONFIG_H
#define BOARD_CORE_F407_SEED_DEV_CONFIG_H

#include <stdint.h>

#include "main.h"
#include "bsp_adc.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

#define BOARD_NAME "core_f407_seed_dev"

#define BOARD_HAS_STATUS_LED         0U

/* USART1 is the PC serial monitor; USART2 is the fingerprint UART. */
#define BOARD_DEBUG_UART_ID          BSP_UART_ID_1
#define BOARD_COMM_UART_ID           BSP_UART_ID_2

#define BOARD_DEBUG_UART_TX_PORT       BSP_GPIO_PORT_A
#define BOARD_DEBUG_UART_TX_PIN        ((uint16_t)(1U << 9))
#define BOARD_DEBUG_UART_TX_AF         7U
#define BOARD_DEBUG_UART_RX_PORT       BSP_GPIO_PORT_A
#define BOARD_DEBUG_UART_RX_PIN        ((uint16_t)(1U << 10))
#define BOARD_DEBUG_UART_RX_AF         7U

#define BOARD_COMM_UART_TX_PORT        BSP_GPIO_PORT_A
#define BOARD_COMM_UART_TX_PIN         ((uint16_t)(1U << 2))
#define BOARD_COMM_UART_TX_AF          7U
#define BOARD_COMM_UART_RX_PORT        BSP_GPIO_PORT_A
#define BOARD_COMM_UART_RX_PIN         ((uint16_t)(1U << 3))
#define BOARD_COMM_UART_RX_AF          7U

#define BOARD_SENSOR_I2C_ID          BSP_I2C_ID_1
#define BOARD_SENSOR_I2C_SCL_PORT    BSP_GPIO_PORT_B
#define BOARD_SENSOR_I2C_SCL_PIN     ((uint16_t)(1U << 6))
#define BOARD_SENSOR_I2C_SCL_AF      4U
#define BOARD_SENSOR_I2C_SDA_PORT    BSP_GPIO_PORT_B
#define BOARD_SENSOR_I2C_SDA_PIN     ((uint16_t)(1U << 7))
#define BOARD_SENSOR_I2C_SDA_AF      4U

#define BOARD_STORAGE_SPI_ID         BSP_SPI_ID_1
#define BOARD_STORAGE_SPI_SCK_PORT   BSP_GPIO_PORT_A
#define BOARD_STORAGE_SPI_SCK_PIN    ((uint16_t)(1U << 5))
#define BOARD_STORAGE_SPI_SCK_AF     5U
#define BOARD_STORAGE_SPI_MISO_PORT  BSP_GPIO_PORT_A
#define BOARD_STORAGE_SPI_MISO_PIN   ((uint16_t)(1U << 6))
#define BOARD_STORAGE_SPI_MISO_AF    5U
#define BOARD_STORAGE_SPI_MOSI_PORT  BSP_GPIO_PORT_A
#define BOARD_STORAGE_SPI_MOSI_PIN   ((uint16_t)(1U << 7))
#define BOARD_STORAGE_SPI_MOSI_AF    5U
#define BOARD_STORAGE_SPI_CS_PORT    BSP_GPIO_PORT_B
#define BOARD_STORAGE_SPI_CS_PIN     ((uint16_t)(1U << 1))
#define BOARD_STORAGE_SPI_CS_ACTIVE_LOW 1U
#define BOARD_STORAGE_SPI_PRESCALER  BSP_SPI_PRESCALER_32

#define BOARD_ADC_INPUT_ID           BSP_ADC_ID_1
#define BOARD_TICK_TIMER_ID          BSP_TIMER_ID_1

#define BOARD_USER_KEY_PORT          BSP_GPIO_PORT_B
#define BOARD_USER_KEY_PIN           ((uint16_t)(1U << 2))
#define BOARD_USER_KEY_ACTIVE_LOW    1U

/*
 * DS211 wiring on this board:
 * module Touch (yellow) -> PB0, active high.
 */
#define BOARD_HAS_FINGERPRINT              1U
#define BOARD_FINGERPRINT_UART_ID          BSP_UART_ID_2
#define BOARD_FINGERPRINT_HAS_TOUCH        1U
#define BOARD_FINGERPRINT_TOUCH_PORT       BSP_GPIO_PORT_B
#define BOARD_FINGERPRINT_TOUCH_PIN        ((uint16_t)(1U << 0))
#define BOARD_FINGERPRINT_TOUCH_ACTIVE_HIGH 1U

#endif
