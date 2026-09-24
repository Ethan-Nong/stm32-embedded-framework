#ifndef DRV_SPI_DEVICE_TEMPLATE_H
#define DRV_SPI_DEVICE_TEMPLATE_H

#include <stdint.h>

#include "bsp_gpio.h"
#include "bsp_spi.h"

typedef struct
{
    bsp_spi_t bus;
    bsp_gpio_t cs;
    bsp_spi_prescaler_t prescaler;
    uint8_t cs_active_low;
} drv_spi_device_template_cfg_t;

int drv_spi_device_template_transfer(const drv_spi_device_template_cfg_t *cfg,
                                     const void *tx_data,
                                     void *rx_data,
                                     uint16_t size);

#endif
