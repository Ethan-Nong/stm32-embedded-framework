#ifndef BASEOS_BSP_ETH_H
#define BASEOS_BSP_ETH_H

#include <stdint.h>

typedef struct
{
    uint8_t mac[6];
    uint8_t phy_address;
    uint8_t reset_i2c_address_7bit;
    uint8_t reset_io_pin;
    uint8_t reset_active_high;
} bsp_eth_cfg_t;

int bsp_eth_init(const bsp_eth_cfg_t *cfg);
void bsp_eth_deinit(void);
int bsp_eth_start(void);
int bsp_eth_stop(void);
int bsp_eth_transmit(const void *packet_config, uint32_t timeout_ms);
int bsp_eth_rx_available(void);
int bsp_eth_get_rx_buffer(void *buffers);
int bsp_eth_get_rx_length(uint32_t *length);
int bsp_eth_rebuild_rx_descriptors(void);
int bsp_eth_phy_read(uint8_t phy_address, uint8_t reg, uint32_t *value);
int bsp_eth_phy_write(uint8_t phy_address, uint8_t reg, uint32_t value);
int bsp_eth_set_link(uint32_t speed, uint32_t duplex);
void *bsp_eth_handle(void);

#endif
