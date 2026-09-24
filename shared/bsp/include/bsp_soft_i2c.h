#ifndef BASEOS_BSP_SOFT_I2C_H
#define BASEOS_BSP_SOFT_I2C_H

#include <stdint.h>

#include "bsp_gpio.h"

#define BSP_SOFT_I2C_OK      0
#define BSP_SOFT_I2C_EINVAL -1
#define BSP_SOFT_I2C_EIO    -2
#define BSP_SOFT_I2C_EACK   -3

/*
 * 软件 I2C 总线描述符。
 *
 * delay_us 为时钟半周期，传入 0 时使用适合约 100kHz 通信的默认值。
 * timeout_cycles 为等待从机释放 SCL 的轮询次数，传入 0 时使用默认超时值。
 */
typedef struct
{
    bsp_gpio_t scl;
    bsp_gpio_t sda;
    uint16_t delay_us;
    uint16_t timeout_cycles;
} bsp_soft_i2c_t;

/* 将 SCL/SDA 配置为开漏输出并释放总线。 */
int bsp_soft_i2c_init(const bsp_soft_i2c_t *bus);

/* 释放 SCL/SDA，不改变引脚当前配置。 */
int bsp_soft_i2c_deinit(const bsp_soft_i2c_t *bus);

/* 仅发送写地址并返回 ACK/NACK，不传输数据。 */
int bsp_soft_i2c_probe(const bsp_soft_i2c_t *bus, uint8_t device_address);

/* 主机原始写。device_address 为 7 位 I2C 地址。 */
int bsp_soft_i2c_write(const bsp_soft_i2c_t *bus, uint8_t device_address, const void *data, uint16_t size);

/* 主机原始读。device_address 为 7 位 I2C 地址。 */
int bsp_soft_i2c_read(const bsp_soft_i2c_t *bus, uint8_t device_address, void *data, uint16_t size);

/* 寄存器/存储器地址写辅助接口。memory_address_size 支持 1 或 2 字节。 */
int bsp_soft_i2c_mem_write(const bsp_soft_i2c_t *bus, uint8_t device_address, uint16_t memory_address,
                           uint8_t memory_address_size, const void *data, uint16_t size);

/* 使用重复起始条件的寄存器/存储器地址读辅助接口。memory_address_size 支持 1 或 2 字节。 */
int bsp_soft_i2c_mem_read(const bsp_soft_i2c_t *bus, uint8_t device_address, uint16_t memory_address,
                          uint8_t memory_address_size, void *data, uint16_t size);

#endif
