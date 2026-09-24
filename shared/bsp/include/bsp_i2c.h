#ifndef BASEOS_BSP_I2C_H
#define BASEOS_BSP_I2C_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑 I2C 总线编号。 */
typedef enum
{
    BSP_I2C_ID_1 = 1,
    BSP_I2C_ID_2,
    BSP_I2C_ID_3
} bsp_i2c_id_t;

/* 脱离具体 target 的抽象 I2C 总线句柄。 */
typedef struct
{
    bsp_i2c_id_t id;
} bsp_i2c_t;

/*
 * 探测指定从设备是否应答。
 * device_address 使用 HAL 约定的 8 位地址，即 7 位器件地址左移 1 位。
 * 成功返回 0；设备未连接、总线异常或 I2C 实例不存在时返回负值。
 */
int bsp_i2c_is_device_ready(const bsp_i2c_t *bus, uint16_t device_address);

/* 阻塞原始写，成功返回传输字节数，失败返回负值。 */
int bsp_i2c_write(const bsp_i2c_t *bus, uint16_t device_address, const void *data, uint16_t size);
/* 阻塞原始读，成功返回传输字节数，失败返回负值。 */
int bsp_i2c_read(const bsp_i2c_t *bus, uint16_t device_address, void *data, uint16_t size);
/* 阻塞寄存器/内存地址写辅助接口。 */
int bsp_i2c_mem_write(const bsp_i2c_t *bus, uint16_t device_address, uint16_t memory_address,
                      uint16_t memory_address_size, const void *data, uint16_t size);
/* 阻塞寄存器/内存地址读辅助接口。 */
int bsp_i2c_mem_read(const bsp_i2c_t *bus, uint16_t device_address, uint16_t memory_address,
                     uint16_t memory_address_size, void *data, uint16_t size);

#endif
