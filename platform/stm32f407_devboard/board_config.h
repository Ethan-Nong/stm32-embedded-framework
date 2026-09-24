#ifndef BOARD_STM32F407_DEVBOARD_BOARD_CONFIG_H
#define BOARD_STM32F407_DEVBOARD_BOARD_CONFIG_H

#include <stdint.h>
#include "main.h"
#include "bsp_gpio.h"
#include "bsp_exti.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

#define BOARD_NAME "stm32f407_devboard"

/* No status LED is configured in the current CubeMX target. */
#define BOARD_HAS_STATUS_LED        1U
#define BOARD_STATUS_LED_PORT       BSP_GPIO_PORT_C
#define BOARD_STATUS_LED_PIN        ((uint16_t)(1U << 0))
#define BOARD_STATUS_LED_ACTIVE_LOW 1U

#define BOARD_DEBUG_UART_ID         BSP_UART_ID_1
#define BOARD_COMM_UART_ID          BSP_UART_ID_2

#define BOARD_SENSOR_I2C_ID         BSP_I2C_ID_1

#define BOARD_STORAGE_SPI_ID        BSP_SPI_ID_1
#define BOARD_STORAGE_SPI_CS_PORT   BSP_GPIO_PORT_B
#define BOARD_STORAGE_SPI_CS_PIN    SPI1_CS_Pin
#define BOARD_STORAGE_SPI_CS_ACTIVE_LOW 1U
#define BOARD_STORAGE_SPI_PRESCALER BSP_SPI_PRESCALER_32

#define BOARD_ADC_INPUT_ID          BSP_ADC_ID_1

#define BOARD_TICK_TIMER_ID         BSP_TIMER_ID_3

#define BOARD_USER_KEY_PORT         BSP_GPIO_PORT_B
#define BOARD_USER_KEY_PIN          INT_0_Pin
#define BOARD_USER_KEY_ACTIVE_LOW   0U

#endif
