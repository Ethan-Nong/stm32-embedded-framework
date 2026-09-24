#include "drv_spi_device_template.h"

static void drv_spi_device_template_cs_write(const drv_spi_device_template_cfg_t *cfg,
                                             uint8_t asserted)
{
    bsp_gpio_level_t level;

    if (cfg->cs_active_low != 0U)
    {
        level = asserted ? BSP_GPIO_LEVEL_LOW : BSP_GPIO_LEVEL_HIGH;
    }
    else
    {
        level = asserted ? BSP_GPIO_LEVEL_HIGH : BSP_GPIO_LEVEL_LOW;
    }

    bsp_gpio_write(&cfg->cs, level);
}

int drv_spi_device_template_transfer(const drv_spi_device_template_cfg_t *cfg,
                                     const void *tx_data,
                                     void *rx_data,
                                     uint16_t size)
{
    int result;

    if ((cfg == 0) || (size == 0U))
    {
        return -1;
    }

    if (bsp_spi_lock(&cfg->bus, 1000U) != 0)
    {
        return -1;
    }

    result = bsp_spi_set_prescaler(&cfg->bus, cfg->prescaler);
    if (result == 0)
    {
        drv_spi_device_template_cs_write(cfg, 1U);
        result = bsp_spi_transfer(&cfg->bus, tx_data, rx_data, size);
        drv_spi_device_template_cs_write(cfg, 0U);
    }

    bsp_spi_unlock(&cfg->bus);
    return result;
}
