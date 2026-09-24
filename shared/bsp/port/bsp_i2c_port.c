#include "bsp_i2c.h"
#include "bsp_port_hal.h"

#ifdef HAL_I2C_MODULE_ENABLED

#define BSP_I2C_TIMEOUT_MS 50U

/* 将 BSP I2C 编号映射到 CubeMX 生成的 I2C 句柄。 */
static I2C_HandleTypeDef *bsp_i2c_resolve(bsp_i2c_id_t id)
{
    switch (id)
    {
    case BSP_I2C_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hi2c1) ? &hi2c1 : 0;
    case BSP_I2C_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hi2c2) ? &hi2c2 : 0;
    case BSP_I2C_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hi2c3) ? &hi2c3 : 0;
    default:
        return 0;
    }
}

/* 统一处理 I2C HAL 返回值；失败时重置 HAL 状态，避免后续事务一直 BUSY。 */
static int bsp_i2c_finish(I2C_HandleTypeDef *handle, HAL_StatusTypeDef status, uint16_t size)
{
    if (status == HAL_OK)
    {
        return (int)size;
    }

    /*
     * STM32 HAL I2C 在超时/错误后可能保持 BUSY/错误状态；重新初始化可避免后续事务
     * 一直失败。这里不做长时间总线恢复，只清 HAL 外设状态。
     */
    (void)HAL_I2C_DeInit(handle);
    (void)HAL_I2C_Init(handle);
    return -1;
}

/*
 * 使用 HAL 的地址应答探测封装设备在线检查。
 * 探测失败不重置 I2C 外设：未插设备属于正常情况，不能影响随后其他设备的事务。
 */
int bsp_i2c_is_device_ready(const bsp_i2c_t *bus, uint16_t device_address)
{
    I2C_HandleTypeDef *handle;

    if (bus == 0)
    {
        return -1;
    }

    handle = bsp_i2c_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_I2C_IsDeviceReady(handle, device_address, 2U,
                                  BSP_I2C_TIMEOUT_MS) == HAL_OK) ? 0 : -1;
}

/* 阻塞式主机写。 */
int bsp_i2c_write(const bsp_i2c_t *bus, uint16_t device_address, const void *data, uint16_t size)
{
    I2C_HandleTypeDef *handle;

    if ((bus == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_i2c_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return bsp_i2c_finish(handle,
                          HAL_I2C_Master_Transmit(handle,
                                                  device_address,
                                                  (uint8_t *)data,
                                                  size,
                                                  BSP_I2C_TIMEOUT_MS),
                          size);
}

/* 阻塞式主机读。 */
int bsp_i2c_read(const bsp_i2c_t *bus, uint16_t device_address, void *data, uint16_t size)
{
    I2C_HandleTypeDef *handle;

    if ((bus == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_i2c_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return bsp_i2c_finish(handle,
                          HAL_I2C_Master_Receive(handle,
                                                 device_address,
                                                 (uint8_t *)data,
                                                 size,
                                                 BSP_I2C_TIMEOUT_MS),
                          size);
}

/* 阻塞式寄存器/内存地址写。 */
int bsp_i2c_mem_write(const bsp_i2c_t *bus, uint16_t device_address, uint16_t memory_address,
                      uint16_t memory_address_size, const void *data, uint16_t size)
{
    I2C_HandleTypeDef *handle;

    if ((bus == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_i2c_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return bsp_i2c_finish(handle,
                          HAL_I2C_Mem_Write(handle,
                                            device_address,
                                            memory_address,
                                            memory_address_size,
                                            (uint8_t *)data,
                                            size,
                                            BSP_I2C_TIMEOUT_MS),
                          size);
}

/* 阻塞式寄存器/内存地址读。 */
int bsp_i2c_mem_read(const bsp_i2c_t *bus, uint16_t device_address, uint16_t memory_address,
                     uint16_t memory_address_size, void *data, uint16_t size)
{
    I2C_HandleTypeDef *handle;

    if ((bus == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_i2c_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return bsp_i2c_finish(handle,
                          HAL_I2C_Mem_Read(handle,
                                           device_address,
                                           memory_address,
                                           memory_address_size,
                                           (uint8_t *)data,
                                           size,
                                           BSP_I2C_TIMEOUT_MS),
                          size);
}

#endif
