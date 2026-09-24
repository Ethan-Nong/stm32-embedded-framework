#include "bsp_soft_i2c.h"

#include "bsp_tick.h"

#define BSP_SOFT_I2C_DEFAULT_DELAY_US      5U
#define BSP_SOFT_I2C_DEFAULT_TIMEOUT_LOOPS 200U

#define BSP_SOFT_I2C_WRITE_BIT             0U
#define BSP_SOFT_I2C_READ_BIT              1U

static const bsp_gpio_config_t g_soft_i2c_gpio_cfg = {
    BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
    BSP_GPIO_PULL_UP,
    BSP_GPIO_SPEED_HIGH
};

static uint16_t bsp_soft_i2c_delay_us(const bsp_soft_i2c_t *bus)
{
    return (bus->delay_us != 0U) ? bus->delay_us : BSP_SOFT_I2C_DEFAULT_DELAY_US;
}

static uint16_t bsp_soft_i2c_timeout_loops(const bsp_soft_i2c_t *bus)
{
    return (bus->timeout_cycles != 0U) ? bus->timeout_cycles : BSP_SOFT_I2C_DEFAULT_TIMEOUT_LOOPS;
}

static int bsp_soft_i2c_validate(const bsp_soft_i2c_t *bus)
{
    if ((bus == 0) || (bus->scl.pin == 0U) || (bus->sda.pin == 0U))
    {
        return BSP_SOFT_I2C_EINVAL;
    }

    return BSP_SOFT_I2C_OK;
}

static void bsp_soft_i2c_delay(const bsp_soft_i2c_t *bus)
{
    bsp_delay_us((uint32_t)bsp_soft_i2c_delay_us(bus));
}

static void bsp_soft_i2c_scl_release(const bsp_soft_i2c_t *bus)
{
    bsp_gpio_write(&bus->scl, BSP_GPIO_LEVEL_HIGH);
}

static void bsp_soft_i2c_scl_low(const bsp_soft_i2c_t *bus)
{
    bsp_gpio_write(&bus->scl, BSP_GPIO_LEVEL_LOW);
}

static void bsp_soft_i2c_sda_release(const bsp_soft_i2c_t *bus)
{
    bsp_gpio_write(&bus->sda, BSP_GPIO_LEVEL_HIGH);
}

static void bsp_soft_i2c_sda_low(const bsp_soft_i2c_t *bus)
{
    bsp_gpio_write(&bus->sda, BSP_GPIO_LEVEL_LOW);
}

static uint8_t bsp_soft_i2c_sda_read(const bsp_soft_i2c_t *bus)
{
    return (bsp_gpio_read(&bus->sda) == BSP_GPIO_LEVEL_HIGH) ? 1U : 0U;
}

static uint8_t bsp_soft_i2c_scl_read(const bsp_soft_i2c_t *bus)
{
    return (bsp_gpio_read(&bus->scl) == BSP_GPIO_LEVEL_HIGH) ? 1U : 0U;
}

static int bsp_soft_i2c_wait_scl_high(const bsp_soft_i2c_t *bus)
{
    uint16_t timeout;

    /* 从机时钟拉伸期间会持续拉低 SCL。 */
    timeout = bsp_soft_i2c_timeout_loops(bus);
    while (bsp_soft_i2c_scl_read(bus) == 0U)
    {
        if (timeout == 0U)
        {
            return BSP_SOFT_I2C_EIO;
        }
        timeout--;
        bsp_soft_i2c_delay(bus);
    }

    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_clock_high(const bsp_soft_i2c_t *bus)
{
    bsp_soft_i2c_scl_release(bus);
    if (bsp_soft_i2c_wait_scl_high(bus) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    bsp_soft_i2c_delay(bus);
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_start(const bsp_soft_i2c_t *bus)
{
    /* SCL 为高电平时，SDA 由高变低形成起始条件。 */
    bsp_soft_i2c_sda_release(bus);
    if (bsp_soft_i2c_clock_high(bus) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    bsp_soft_i2c_sda_low(bus);
    bsp_soft_i2c_delay(bus);
    bsp_soft_i2c_scl_low(bus);
    bsp_soft_i2c_delay(bus);
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_stop(const bsp_soft_i2c_t *bus)
{
    /* SCL 为高电平时，SDA 由低变高形成停止条件。 */
    bsp_soft_i2c_sda_low(bus);
    bsp_soft_i2c_delay(bus);
    if (bsp_soft_i2c_clock_high(bus) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    bsp_soft_i2c_sda_release(bus);
    bsp_soft_i2c_delay(bus);
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_write_bit(const bsp_soft_i2c_t *bus, uint8_t bit)
{
    if (bit != 0U)
    {
        bsp_soft_i2c_sda_release(bus);
    }
    else
    {
        bsp_soft_i2c_sda_low(bus);
    }

    bsp_soft_i2c_delay(bus);
    if (bsp_soft_i2c_clock_high(bus) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    bsp_soft_i2c_scl_low(bus);
    bsp_soft_i2c_delay(bus);
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_read_bit(const bsp_soft_i2c_t *bus, uint8_t *bit)
{
    if (bit == 0)
    {
        return BSP_SOFT_I2C_EINVAL;
    }

    bsp_soft_i2c_sda_release(bus);
    bsp_soft_i2c_delay(bus);
    if (bsp_soft_i2c_clock_high(bus) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    *bit = bsp_soft_i2c_sda_read(bus);
    bsp_soft_i2c_scl_low(bus);
    bsp_soft_i2c_delay(bus);
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_write_byte(const bsp_soft_i2c_t *bus, uint8_t data)
{
    uint8_t mask;
    uint8_t ack;

    for (mask = 0x80U; mask != 0U; mask >>= 1U)
    {
        if (bsp_soft_i2c_write_bit(bus, (data & mask) != 0U) != BSP_SOFT_I2C_OK)
        {
            return BSP_SOFT_I2C_EIO;
        }
    }

    if (bsp_soft_i2c_read_bit(bus, &ack) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    return (ack == 0U) ? BSP_SOFT_I2C_OK : BSP_SOFT_I2C_EACK;
}

static int bsp_soft_i2c_read_byte(const bsp_soft_i2c_t *bus, uint8_t *data, uint8_t ack)
{
    uint8_t value;
    uint8_t bit;
    uint8_t index;

    if (data == 0)
    {
        return BSP_SOFT_I2C_EINVAL;
    }

    value = 0U;
    for (index = 0U; index < 8U; index++)
    {
        if (bsp_soft_i2c_read_bit(bus, &bit) != BSP_SOFT_I2C_OK)
        {
            return BSP_SOFT_I2C_EIO;
        }
        value = (uint8_t)((value << 1U) | bit);
    }

    if (bsp_soft_i2c_write_bit(bus, (ack == 0U) ? 1U : 0U) != BSP_SOFT_I2C_OK)
    {
        return BSP_SOFT_I2C_EIO;
    }

    *data = value;
    return BSP_SOFT_I2C_OK;
}

static int bsp_soft_i2c_write_address(const bsp_soft_i2c_t *bus, uint8_t device_address, uint8_t read)
{
    return bsp_soft_i2c_write_byte(bus, (uint8_t)((device_address << 1U) | (read & 0x01U)));
}

static int bsp_soft_i2c_write_memory_address(const bsp_soft_i2c_t *bus, uint16_t memory_address,
                                             uint8_t memory_address_size)
{
    if (memory_address_size == 1U)
    {
        return bsp_soft_i2c_write_byte(bus, (uint8_t)memory_address);
    }

    if (memory_address_size == 2U)
    {
        int ret;

        ret = bsp_soft_i2c_write_byte(bus, (uint8_t)(memory_address >> 8U));
        if (ret != BSP_SOFT_I2C_OK)
        {
            return ret;
        }
        return bsp_soft_i2c_write_byte(bus, (uint8_t)memory_address);
    }

    return BSP_SOFT_I2C_EINVAL;
}

int bsp_soft_i2c_init(const bsp_soft_i2c_t *bus)
{
    int ret;

    ret = bsp_soft_i2c_validate(bus);
    if (ret != BSP_SOFT_I2C_OK)
    {
        return ret;
    }

    bsp_gpio_config(&bus->scl, &g_soft_i2c_gpio_cfg);
    bsp_gpio_config(&bus->sda, &g_soft_i2c_gpio_cfg);
    bsp_soft_i2c_scl_release(bus);
    bsp_soft_i2c_sda_release(bus);
    bsp_soft_i2c_delay(bus);

    return ((bsp_soft_i2c_scl_read(bus) != 0U) && (bsp_soft_i2c_sda_read(bus) != 0U)) ?
        BSP_SOFT_I2C_OK :
        BSP_SOFT_I2C_EIO;
}

int bsp_soft_i2c_deinit(const bsp_soft_i2c_t *bus)
{
    int ret;

    ret = bsp_soft_i2c_validate(bus);
    if (ret != BSP_SOFT_I2C_OK)
    {
        return ret;
    }

    bsp_soft_i2c_scl_release(bus);
    bsp_soft_i2c_sda_release(bus);
    return BSP_SOFT_I2C_OK;
}

int bsp_soft_i2c_probe(const bsp_soft_i2c_t *bus, uint8_t device_address)
{
    int ret;

    if ((bsp_soft_i2c_validate(bus) != BSP_SOFT_I2C_OK) || (device_address > 0x7FU))
    {
        return BSP_SOFT_I2C_EINVAL;
    }

    ret = bsp_soft_i2c_start(bus);
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_WRITE_BIT);
    }

    (void)bsp_soft_i2c_stop(bus);
    return ret;
}

int bsp_soft_i2c_write(const bsp_soft_i2c_t *bus, uint8_t device_address, const void *data, uint16_t size)
{
    const uint8_t *bytes;
    uint16_t index;
    int ret;

    if ((bsp_soft_i2c_validate(bus) != BSP_SOFT_I2C_OK) || (device_address > 0x7FU) ||
        ((data == 0) && (size != 0U)))
    {
        return BSP_SOFT_I2C_EINVAL;
    }
    if (size == 0U)
    {
        return 0;
    }

    ret = bsp_soft_i2c_start(bus);
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_WRITE_BIT);
    }

    bytes = (const uint8_t *)data;
    for (index = 0U; (ret == BSP_SOFT_I2C_OK) && (index < size); index++)
    {
        ret = bsp_soft_i2c_write_byte(bus, bytes[index]);
    }

    (void)bsp_soft_i2c_stop(bus);
    return (ret == BSP_SOFT_I2C_OK) ? (int)size : ret;
}

int bsp_soft_i2c_read(const bsp_soft_i2c_t *bus, uint8_t device_address, void *data, uint16_t size)
{
    uint8_t *bytes;
    uint16_t index;
    int ret;

    if ((bsp_soft_i2c_validate(bus) != BSP_SOFT_I2C_OK) || (device_address > 0x7FU) ||
        ((data == 0) && (size != 0U)))
    {
        return BSP_SOFT_I2C_EINVAL;
    }
    if (size == 0U)
    {
        return 0;
    }

    ret = bsp_soft_i2c_start(bus);
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_READ_BIT);
    }

    bytes = (uint8_t *)data;
    for (index = 0U; (ret == BSP_SOFT_I2C_OK) && (index < size); index++)
    {
        ret = bsp_soft_i2c_read_byte(bus, &bytes[index], (index + 1U) < size);
    }

    (void)bsp_soft_i2c_stop(bus);
    return (ret == BSP_SOFT_I2C_OK) ? (int)size : ret;
}

int bsp_soft_i2c_mem_write(const bsp_soft_i2c_t *bus, uint8_t device_address, uint16_t memory_address,
                           uint8_t memory_address_size, const void *data, uint16_t size)
{
    const uint8_t *bytes;
    uint16_t index;
    int ret;

    if ((bsp_soft_i2c_validate(bus) != BSP_SOFT_I2C_OK) || (device_address > 0x7FU) ||
        ((data == 0) && (size != 0U)))
    {
        return BSP_SOFT_I2C_EINVAL;
    }
    if (size == 0U)
    {
        return 0;
    }

    ret = bsp_soft_i2c_start(bus);
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_WRITE_BIT);
    }
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_memory_address(bus, memory_address, memory_address_size);
    }

    bytes = (const uint8_t *)data;
    for (index = 0U; (ret == BSP_SOFT_I2C_OK) && (index < size); index++)
    {
        ret = bsp_soft_i2c_write_byte(bus, bytes[index]);
    }

    (void)bsp_soft_i2c_stop(bus);
    return (ret == BSP_SOFT_I2C_OK) ? (int)size : ret;
}

int bsp_soft_i2c_mem_read(const bsp_soft_i2c_t *bus, uint8_t device_address, uint16_t memory_address,
                          uint8_t memory_address_size, void *data, uint16_t size)
{
    uint8_t *bytes;
    uint16_t index;
    int ret;

    if ((bsp_soft_i2c_validate(bus) != BSP_SOFT_I2C_OK) || (device_address > 0x7FU) ||
        ((data == 0) && (size != 0U)))
    {
        return BSP_SOFT_I2C_EINVAL;
    }
    if (size == 0U)
    {
        return 0;
    }

    ret = bsp_soft_i2c_start(bus);
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_WRITE_BIT);
    }
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_memory_address(bus, memory_address, memory_address_size);
    }
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_start(bus);
    }
    if (ret == BSP_SOFT_I2C_OK)
    {
        ret = bsp_soft_i2c_write_address(bus, device_address, BSP_SOFT_I2C_READ_BIT);
    }

    bytes = (uint8_t *)data;
    for (index = 0U; (ret == BSP_SOFT_I2C_OK) && (index < size); index++)
    {
        ret = bsp_soft_i2c_read_byte(bus, &bytes[index], (index + 1U) < size);
    }

    (void)bsp_soft_i2c_stop(bus);
    return (ret == BSP_SOFT_I2C_OK) ? (int)size : ret;
}
