#include "board.h"
#include "board_config.h"

static const board_led_cfg_t g_status_led_cfg = {
    { BOARD_STATUS_LED_PORT, BOARD_STATUS_LED_PIN },
    BOARD_STATUS_LED_ACTIVE_LOW
};

static const board_uart_cfg_t g_debug_uart_cfg = {
    { BOARD_DEBUG_UART_ID }
};

static const board_uart_cfg_t g_comm_uart_cfg = {
    { BOARD_COMM_UART_ID }
};

static const board_i2c_cfg_t g_sensor_i2c_cfg = {
    { BOARD_SENSOR_I2C_ID }
};

static const board_spi_cfg_t g_storage_spi_cfg = {
    { BOARD_STORAGE_SPI_ID },
    { BOARD_STORAGE_SPI_CS_PORT, BOARD_STORAGE_SPI_CS_PIN },
    BOARD_STORAGE_SPI_CS_ACTIVE_LOW
};

static const board_adc_cfg_t g_adc_input_cfg = {
    { BOARD_ADC_INPUT_ID }
};

static const board_timer_cfg_t g_tick_timer_cfg = {
    { BOARD_TICK_TIMER_ID }
};

static const board_key_cfg_t g_user_key_cfg = {
    { BOARD_USER_KEY_PORT, BOARD_USER_KEY_PIN },
    BOARD_USER_KEY_ACTIVE_LOW
};

void board_init(void)
{
    bsp_gpio_write(&g_storage_spi_cfg.cs_gpio, BSP_GPIO_LEVEL_HIGH);
}

const board_led_cfg_t *board_get_status_led_cfg(void)
{
#if BOARD_HAS_STATUS_LED
    return &g_status_led_cfg;
#else
    return 0;
#endif
}

const board_uart_cfg_t *board_get_debug_uart_cfg(void)
{
    return &g_debug_uart_cfg;
}

const board_uart_cfg_t *board_get_comm_uart_cfg(void)
{
    return &g_comm_uart_cfg;
}

const board_i2c_cfg_t *board_get_sensor_i2c_cfg(void)
{
    return &g_sensor_i2c_cfg;
}

const board_spi_cfg_t *board_get_storage_spi_cfg(void)
{
    return &g_storage_spi_cfg;
}

const board_adc_cfg_t *board_get_adc_input_cfg(void)
{
    return &g_adc_input_cfg;
}

const board_timer_cfg_t *board_get_tick_timer_cfg(void)
{
    return &g_tick_timer_cfg;
}

const board_key_cfg_t *board_get_user_key_cfg(void)
{
    return &g_user_key_cfg;
}
