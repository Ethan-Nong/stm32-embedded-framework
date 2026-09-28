#include "board.h"
#include "board_config.h"
#include "common_def.h"

#if defined(BOARD_HAS_STATUS_LED) && BOARD_HAS_STATUS_LED
static const board_led_cfg_t g_status_led_cfg = {
    { BOARD_STATUS_LED_PORT, BOARD_STATUS_LED_PIN },
    BOARD_STATUS_LED_ACTIVE_LOW
};
#endif

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

#if defined(BOARD_HAS_FINGERPRINT) && BOARD_HAS_FINGERPRINT
static const board_fingerprint_cfg_t g_fingerprint_cfg = {
    { BOARD_FINGERPRINT_UART_ID },
    { BOARD_FINGERPRINT_TOUCH_PORT, BOARD_FINGERPRINT_TOUCH_PIN },
    BOARD_FINGERPRINT_HAS_TOUCH,
    BOARD_FINGERPRINT_TOUCH_ACTIVE_HIGH
};
#endif

static const board_pin_cfg_t g_debug_uart_pins[] = {
    {
        { BOARD_DEBUG_UART_TX_PORT, BOARD_DEBUG_UART_TX_PIN },
        BOARD_PIN_FUNCTION_UART_TX,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        BOARD_DEBUG_UART_TX_AF
    },
    {
        { BOARD_DEBUG_UART_RX_PORT, BOARD_DEBUG_UART_RX_PIN },
        BOARD_PIN_FUNCTION_UART_RX,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_HIGH,
        BOARD_DEBUG_UART_RX_AF
    }
};

static const board_pin_cfg_t g_debug_uart_gpio_pins[] = {
    {
        { BOARD_DEBUG_UART_TX_PORT, BOARD_DEBUG_UART_TX_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_DEBUG_UART_RX_PORT, BOARD_DEBUG_UART_RX_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_LOW,
        0U
    }
};

static const board_pin_cfg_t g_comm_uart_pins[] = {
    {
        { BOARD_COMM_UART_TX_PORT, BOARD_COMM_UART_TX_PIN },
        BOARD_PIN_FUNCTION_UART_TX,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        BOARD_COMM_UART_TX_AF
    },
    {
        { BOARD_COMM_UART_RX_PORT, BOARD_COMM_UART_RX_PIN },
        BOARD_PIN_FUNCTION_UART_RX,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_HIGH,
        BOARD_COMM_UART_RX_AF
    }
};

static const board_pin_cfg_t g_comm_uart_gpio_pins[] = {
    {
        { BOARD_COMM_UART_TX_PORT, BOARD_COMM_UART_TX_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_COMM_UART_RX_PORT, BOARD_COMM_UART_RX_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_LOW,
        0U
    }
};

static const board_pin_cfg_t g_sensor_i2c_pins[] = {
    {
        { BOARD_SENSOR_I2C_SCL_PORT, BOARD_SENSOR_I2C_SCL_PIN },
        BOARD_PIN_FUNCTION_I2C_SCL,
        BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_HIGH,
        BOARD_SENSOR_I2C_SCL_AF
    },
    {
        { BOARD_SENSOR_I2C_SDA_PORT, BOARD_SENSOR_I2C_SDA_PIN },
        BOARD_PIN_FUNCTION_I2C_SDA,
        BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_HIGH,
        BOARD_SENSOR_I2C_SDA_AF
    }
};

static const board_pin_cfg_t g_sensor_i2c_gpio_pins[] = {
    {
        { BOARD_SENSOR_I2C_SCL_PORT, BOARD_SENSOR_I2C_SCL_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_SENSOR_I2C_SDA_PORT, BOARD_SENSOR_I2C_SDA_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
        BSP_GPIO_PULL_UP,
        BSP_GPIO_SPEED_LOW,
        0U
    }
};

static const board_pin_cfg_t g_storage_spi_pins[] = {
    {
        { BOARD_STORAGE_SPI_SCK_PORT, BOARD_STORAGE_SPI_SCK_PIN },
        BOARD_PIN_FUNCTION_SPI_SCK,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        BOARD_STORAGE_SPI_SCK_AF
    },
    {
        { BOARD_STORAGE_SPI_MISO_PORT, BOARD_STORAGE_SPI_MISO_PIN },
        BOARD_PIN_FUNCTION_SPI_MISO,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        BOARD_STORAGE_SPI_MISO_AF
    },
    {
        { BOARD_STORAGE_SPI_MOSI_PORT, BOARD_STORAGE_SPI_MOSI_PIN },
        BOARD_PIN_FUNCTION_SPI_MOSI,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        BOARD_STORAGE_SPI_MOSI_AF
    },
    {
        { BOARD_STORAGE_SPI_CS_PORT, BOARD_STORAGE_SPI_CS_PIN },
        BOARD_PIN_FUNCTION_SPI_CS,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_HIGH,
        0U
    }
};

static const board_pin_cfg_t g_storage_spi_gpio_pins[] = {
    {
        { BOARD_STORAGE_SPI_SCK_PORT, BOARD_STORAGE_SPI_SCK_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_STORAGE_SPI_MISO_PORT, BOARD_STORAGE_SPI_MISO_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_STORAGE_SPI_MOSI_PORT, BOARD_STORAGE_SPI_MOSI_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    },
    {
        { BOARD_STORAGE_SPI_CS_PORT, BOARD_STORAGE_SPI_CS_PIN },
        BOARD_PIN_FUNCTION_GPIO,
        BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW,
        0U
    }
};

static const board_interface_mode_cfg_t g_debug_uart_modes[] = {
    {
        BOARD_INTERFACE_MODE_UART,
        g_debug_uart_pins,
        (uint8_t)ARRAY_SIZE(g_debug_uart_pins)
    },
    {
        BOARD_INTERFACE_MODE_GPIO,
        g_debug_uart_gpio_pins,
        (uint8_t)ARRAY_SIZE(g_debug_uart_gpio_pins)
    }
};

static const board_interface_mode_cfg_t g_comm_uart_modes[] = {
    {
        BOARD_INTERFACE_MODE_UART,
        g_comm_uart_pins,
        (uint8_t)ARRAY_SIZE(g_comm_uart_pins)
    },
    {
        BOARD_INTERFACE_MODE_GPIO,
        g_comm_uart_gpio_pins,
        (uint8_t)ARRAY_SIZE(g_comm_uart_gpio_pins)
    }
};

static const board_interface_mode_cfg_t g_sensor_i2c_modes[] = {
    {
        BOARD_INTERFACE_MODE_I2C,
        g_sensor_i2c_pins,
        (uint8_t)ARRAY_SIZE(g_sensor_i2c_pins)
    },
    {
        BOARD_INTERFACE_MODE_GPIO,
        g_sensor_i2c_gpio_pins,
        (uint8_t)ARRAY_SIZE(g_sensor_i2c_gpio_pins)
    }
};

static const board_interface_mode_cfg_t g_storage_spi_modes[] = {
    {
        BOARD_INTERFACE_MODE_SPI,
        g_storage_spi_pins,
        (uint8_t)ARRAY_SIZE(g_storage_spi_pins)
    },
    {
        BOARD_INTERFACE_MODE_GPIO,
        g_storage_spi_gpio_pins,
        (uint8_t)ARRAY_SIZE(g_storage_spi_gpio_pins)
    }
};

static const board_interface_cfg_t g_board_interfaces[] = {
    {
        BOARD_INTERFACE_ID_DEBUG_UART,
        "debug_uart",
        g_debug_uart_modes,
        (uint8_t)ARRAY_SIZE(g_debug_uart_modes),
        &g_debug_uart_cfg,
        0,
        0,
        BOARD_INTERFACE_MODE_UART
    },
    {
        BOARD_INTERFACE_ID_COMM_UART,
        "comm_uart",
        g_comm_uart_modes,
        (uint8_t)ARRAY_SIZE(g_comm_uart_modes),
        &g_comm_uart_cfg,
        0,
        0,
        BOARD_INTERFACE_MODE_UART
    },
    {
        BOARD_INTERFACE_ID_SENSOR_I2C,
        "sensor_i2c",
        g_sensor_i2c_modes,
        (uint8_t)ARRAY_SIZE(g_sensor_i2c_modes),
        0,
        &g_sensor_i2c_cfg,
        0,
        BOARD_INTERFACE_MODE_I2C
    },
    {
        BOARD_INTERFACE_ID_STORAGE_SPI,
        "storage_spi",
        g_storage_spi_modes,
        (uint8_t)ARRAY_SIZE(g_storage_spi_modes),
        0,
        0,
        &g_storage_spi_cfg,
        BOARD_INTERFACE_MODE_SPI
    }
};

static board_interface_mode_t g_board_interface_modes[BOARD_INTERFACE_ID_COUNT];
static uint8_t g_board_interface_refcounts[BOARD_INTERFACE_ID_COUNT];

static int board_interface_index(board_interface_id_t id)
{
    if ((id <= BOARD_INTERFACE_ID_NONE) || (id >= BOARD_INTERFACE_ID_COUNT))
    {
        return -1;
    }

    return (int)id;
}

static int board_interface_mode_index(const board_interface_cfg_t *interface_cfg,
                                      board_interface_mode_t mode)
{
    uint8_t index;

    if ((interface_cfg == 0) || (mode == BOARD_INTERFACE_MODE_NONE))
    {
        return -1;
    }

    for (index = 0U; index < interface_cfg->mode_count; ++index)
    {
        if (interface_cfg->mode_cfgs[index].mode == mode)
        {
            return (int)index;
        }
    }

    return -1;
}

static void board_interface_apply_pin(const board_pin_cfg_t *pin,
                                      board_interface_mode_t mode)
{
    bsp_gpio_config_t gpio_config;
    bsp_gpio_mode_t alternate_mode;

    if (pin == 0)
    {
        return;
    }

    if ((mode == BOARD_INTERFACE_MODE_GPIO) ||
        (pin->alternate_function == 0U))
    {
        gpio_config.mode = pin->gpio_mode;
        gpio_config.pull = pin->pull;
        gpio_config.speed = pin->speed;
        bsp_gpio_config(&pin->gpio, &gpio_config);
        return;
    }

    alternate_mode = ((pin->function == BOARD_PIN_FUNCTION_I2C_SCL) ||
                      (pin->function == BOARD_PIN_FUNCTION_I2C_SDA)) ?
                     BSP_GPIO_MODE_AF_OPEN_DRAIN :
                     BSP_GPIO_MODE_AF_PUSH_PULL;
    bsp_gpio_config_alternate(&pin->gpio,
                              alternate_mode,
                              pin->pull,
                              pin->speed,
                              pin->alternate_function);
}

static void board_interface_apply_mode(const board_interface_mode_cfg_t *mode_cfg)
{
    uint8_t index;

    if (mode_cfg == 0)
    {
        return;
    }

    for (index = 0U; index < mode_cfg->pin_count; ++index)
    {
        board_interface_apply_pin(&mode_cfg->pins[index], mode_cfg->mode);
    }
}

static int board_interfaces_overlap(const board_interface_cfg_t *new_interface,
                                    const board_interface_mode_cfg_t *new_mode_cfg)
{
    uint8_t interface_index;
    uint8_t new_pin_index;
    uint8_t other_pin_index;

    for (interface_index = 0U;
         interface_index < (uint8_t)ARRAY_SIZE(g_board_interfaces);
         ++interface_index)
    {
        const board_interface_cfg_t *other_interface = &g_board_interfaces[interface_index];
        int other_index;
        int other_mode_index;
        const board_interface_mode_cfg_t *other_mode_cfg;

        if (other_interface == new_interface)
        {
            continue;
        }

        other_index = board_interface_index(other_interface->id);
        if ((other_index < 0) ||
            (g_board_interface_refcounts[other_index] == 0U))
        {
            continue;
        }

        other_mode_index = board_interface_mode_index(
            other_interface,
            g_board_interface_modes[other_index]);
        if (other_mode_index < 0)
        {
            continue;
        }
        other_mode_cfg = &other_interface->mode_cfgs[other_mode_index];

        for (new_pin_index = 0U;
             new_pin_index < new_mode_cfg->pin_count;
             ++new_pin_index)
        {
            for (other_pin_index = 0U;
                 other_pin_index < other_mode_cfg->pin_count;
                 ++other_pin_index)
            {
                const board_pin_cfg_t *new_pin = &new_mode_cfg->pins[new_pin_index];
                const board_pin_cfg_t *other_pin = &other_mode_cfg->pins[other_pin_index];

                if ((new_pin->gpio.port == other_pin->gpio.port) &&
                    (new_pin->gpio.pin == other_pin->gpio.pin))
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

void board_init(void)
{
    uint8_t index;

    for (index = 0U; index < (uint8_t)ARRAY_SIZE(g_board_interfaces); ++index)
    {
        const board_interface_cfg_t *interface_cfg = &g_board_interfaces[index];
        int interface_index = board_interface_index(interface_cfg->id);
        int mode_index = board_interface_mode_index(interface_cfg,
                                                    interface_cfg->default_mode);

        if ((interface_index >= 0) && (mode_index >= 0))
        {
            board_interface_apply_mode(&interface_cfg->mode_cfgs[mode_index]);
            g_board_interface_modes[interface_index] = interface_cfg->default_mode;
            g_board_interface_refcounts[interface_index] = 0U;
        }
    }

    bsp_gpio_write(&g_storage_spi_cfg.cs_gpio, BSP_GPIO_LEVEL_HIGH);

#if defined(BOARD_HAS_FINGERPRINT) && BOARD_HAS_FINGERPRINT && BOARD_FINGERPRINT_HAS_TOUCH
    bsp_gpio_config_input(&g_fingerprint_cfg.touch_gpio);
#endif
}

const board_led_cfg_t *board_get_status_led_cfg(void)
{
#if defined(BOARD_HAS_STATUS_LED) && BOARD_HAS_STATUS_LED
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

const board_fingerprint_cfg_t *board_get_fingerprint_cfg(void)
{
#if defined(BOARD_HAS_FINGERPRINT) && BOARD_HAS_FINGERPRINT
    return &g_fingerprint_cfg;
#else
    return 0;
#endif
}

const board_interface_cfg_t *board_get_interface_cfg(board_interface_id_t id)
{
    uint8_t index;

    if (id <= BOARD_INTERFACE_ID_NONE)
    {
        return 0;
    }

    for (index = 0U; index < (uint8_t)ARRAY_SIZE(g_board_interfaces); ++index)
    {
        if (g_board_interfaces[index].id == id)
        {
            return &g_board_interfaces[index];
        }
    }

    return 0;
}

const board_interface_mode_cfg_t *board_get_interface_mode_cfg(
    board_interface_id_t id,
    board_interface_mode_t mode)
{
    const board_interface_cfg_t *interface_cfg = board_get_interface_cfg(id);
    int mode_index;

    if (interface_cfg == 0)
    {
        return 0;
    }

    mode_index = board_interface_mode_index(interface_cfg, mode);
    if (mode_index < 0)
    {
        return 0;
    }

    return &interface_cfg->mode_cfgs[mode_index];
}

int board_interface_is_mode_supported(board_interface_id_t id,
                                      board_interface_mode_t mode)
{
    return (board_get_interface_mode_cfg(id, mode) != 0) ? 1 : 0;
}

int board_interface_acquire(board_interface_id_t id, board_interface_mode_t mode)
{
    int index = board_interface_index(id);
    const board_interface_cfg_t *interface_cfg;
    const board_interface_mode_cfg_t *mode_cfg;

    if (index < 0)
    {
        return -1;
    }

    interface_cfg = board_get_interface_cfg(id);
    if (interface_cfg == 0)
    {
        return -1;
    }
    mode_cfg = board_get_interface_mode_cfg(id, mode);
    if (mode_cfg == 0)
    {
        return -1;
    }

    if ((g_board_interface_refcounts[index] != 0U) &&
        (g_board_interface_modes[index] != mode))
    {
        return -1;
    }

    if (g_board_interface_refcounts[index] == 0U)
    {
        if (board_interfaces_overlap(interface_cfg, mode_cfg) != 0)
        {
            return -1;
        }

        board_interface_apply_mode(mode_cfg);
        g_board_interface_modes[index] = mode;
    }

    g_board_interface_refcounts[index]++;
    return 0;
}

int board_interface_release(board_interface_id_t id)
{
    int index = board_interface_index(id);

    if ((index < 0) || (g_board_interface_refcounts[index] == 0U))
    {
        return -1;
    }

    g_board_interface_refcounts[index]--;
    return 0;
}

int board_interface_set_mode(board_interface_id_t id, board_interface_mode_t mode)
{
    int index = board_interface_index(id);
    const board_interface_cfg_t *interface_cfg;
    const board_interface_mode_cfg_t *mode_cfg;

    if (index < 0)
    {
        return -1;
    }

    interface_cfg = board_get_interface_cfg(id);
    if (interface_cfg == 0)
    {
        return -1;
    }
    mode_cfg = board_get_interface_mode_cfg(id, mode);
    if (mode_cfg == 0)
    {
        return -1;
    }

    if (g_board_interface_refcounts[index] > 1U)
    {
        return -1;
    }

    if (board_interfaces_overlap(interface_cfg, mode_cfg) != 0)
    {
        return -1;
    }

    board_interface_apply_mode(mode_cfg);
    g_board_interface_modes[index] = mode;
    return 0;
}

board_interface_mode_t board_interface_get_mode(board_interface_id_t id)
{
    int index = board_interface_index(id);

    if (index < 0)
    {
        return BOARD_INTERFACE_MODE_NONE;
    }

    return g_board_interface_modes[index];
}
