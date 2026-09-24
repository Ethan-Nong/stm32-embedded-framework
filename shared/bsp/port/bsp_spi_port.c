#include "bsp_spi.h"
#include "bsp_port_hal.h"

#ifdef HAL_SPI_MODULE_ENABLED

/* SPI DMA 发送上下文，用于跟踪 busy 状态和完成回调。 */
typedef struct
{
    bsp_spi_t bus;
    bsp_spi_tx_callback_t callback;
    void *user_data;
    volatile uint8_t busy;
} bsp_spi_dma_entry_t;

static bsp_spi_dma_entry_t g_spi_dma_entries[] = {
    { { BSP_SPI_ID_1 }, 0, 0, 0U },
    { { BSP_SPI_ID_2 }, 0, 0, 0U },
    { { BSP_SPI_ID_3 }, 0, 0, 0U },
};

static volatile uint8_t g_spi_locks[3];

/* 将 BSP SPI 编号映射到 CubeMX 生成的 SPI 句柄。 */
static SPI_HandleTypeDef *bsp_spi_resolve(bsp_spi_id_t id)
{
    switch (id)
    {
    case BSP_SPI_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hspi1) ? &hspi1 : 0;
    case BSP_SPI_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hspi2) ? &hspi2 : 0;
    case BSP_SPI_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hspi3) ? &hspi3 : 0;
    default:
        return 0;
    }
}

/* 将 BSP 分频枚举转换为 STM32 HAL 的 BR 位。 */
static int bsp_spi_resolve_prescaler(bsp_spi_prescaler_t prescaler, uint32_t *hal_prescaler)
{
    if (hal_prescaler == 0)
    {
        return -1;
    }

    switch (prescaler)
    {
    case BSP_SPI_PRESCALER_2:   *hal_prescaler = SPI_BAUDRATEPRESCALER_2; break;
    case BSP_SPI_PRESCALER_4:   *hal_prescaler = SPI_BAUDRATEPRESCALER_4; break;
    case BSP_SPI_PRESCALER_8:   *hal_prescaler = SPI_BAUDRATEPRESCALER_8; break;
    case BSP_SPI_PRESCALER_16:  *hal_prescaler = SPI_BAUDRATEPRESCALER_16; break;
    case BSP_SPI_PRESCALER_32:  *hal_prescaler = SPI_BAUDRATEPRESCALER_32; break;
    case BSP_SPI_PRESCALER_64:  *hal_prescaler = SPI_BAUDRATEPRESCALER_64; break;
    case BSP_SPI_PRESCALER_128: *hal_prescaler = SPI_BAUDRATEPRESCALER_128; break;
    case BSP_SPI_PRESCALER_256: *hal_prescaler = SPI_BAUDRATEPRESCALER_256; break;
    default: return -1;
    }
    return 0;
}

/* 按 BSP SPI 编号查找 DMA 上下文。 */
static bsp_spi_dma_entry_t *bsp_spi_find_dma_entry(bsp_spi_id_t id)
{
    unsigned int index;

    for (index = 0U; index < (sizeof(g_spi_dma_entries) / sizeof(g_spi_dma_entries[0])); ++index)
    {
        if (g_spi_dma_entries[index].bus.id == id)
        {
            return &g_spi_dma_entries[index];
        }
    }

    return 0;
}

/* 从 HAL 回调传入的句柄反查 BSP DMA 上下文。 */
static bsp_spi_dma_entry_t *bsp_spi_find_dma_entry_by_handle(SPI_HandleTypeDef *handle)
{
    unsigned int index;

    for (index = 0U; index < (sizeof(g_spi_dma_entries) / sizeof(g_spi_dma_entries[0])); ++index)
    {
        if (bsp_spi_resolve(g_spi_dma_entries[index].bus.id) == handle)
        {
            return &g_spi_dma_entries[index];
        }
    }

    return 0;
}

static int bsp_spi_lock_index(bsp_spi_id_t id, uint32_t *index)
{
    if ((index == 0) || (id < BSP_SPI_ID_1) || (id > BSP_SPI_ID_3))
    {
        return -1;
    }

    *index = (uint32_t)id - 1U;
    return 0;
}

/*
 * 默认实现保证同一固件内的 SPI 事务不会交叉执行。
 * 使用 FreeRTOS 等 RTOS 时，建议在 bsp/overrides 中覆盖为互斥量实现。
 */
int bsp_spi_lock(const bsp_spi_t *bus, uint32_t timeout_ms)
{
    uint32_t index;
    uint32_t start;
    uint32_t primask;

    if ((bus == 0) || (bsp_spi_lock_index(bus->id, &index) != 0))
    {
        return -1;
    }

    start = HAL_GetTick();
    for (;;)
    {
        primask = __get_PRIMASK();
        __disable_irq();
        if (g_spi_locks[index] == 0U)
        {
            g_spi_locks[index] = 1U;
            if (primask == 0U)
            {
                __enable_irq();
            }
            return 0;
        }
        if (primask == 0U)
        {
            __enable_irq();
        }

        if ((timeout_ms == 0U) || ((HAL_GetTick() - start) >= timeout_ms))
        {
            return -1;
        }
    }
}

void bsp_spi_unlock(const bsp_spi_t *bus)
{
    uint32_t index;
    uint32_t primask;

    if ((bus == 0) || (bsp_spi_lock_index(bus->id, &index) != 0))
    {
        return;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    g_spi_locks[index] = 0U;
    if (primask == 0U)
    {
        __enable_irq();
    }
}

/* 阻塞式全双工传输。 */
int bsp_spi_transfer(const bsp_spi_t *bus, const void *tx_data, void *rx_data, uint16_t size)
{
    SPI_HandleTypeDef *handle;

    if ((bus == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_spi_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_SPI_TransmitReceive(handle, (uint8_t *)tx_data, (uint8_t *)rx_data, size, 0xffffU) == HAL_OK) ? (int)size : -1;
}

/* 阻塞式只发送。 */
int bsp_spi_write(const bsp_spi_t *bus, const void *tx_data, uint16_t size)
{
    SPI_HandleTypeDef *handle;

    if ((bus == 0) || (tx_data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_spi_resolve(bus->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_SPI_Transmit(handle, (uint8_t *)tx_data, size, 0xffffU) == HAL_OK) ? (int)size : -1;
}

/* DMA 只发送，完成或错误时通过 callback 回到上层。 */
int bsp_spi_write_dma(const bsp_spi_t *bus,
                      const void *tx_data,
                      uint16_t size,
                      bsp_spi_tx_callback_t callback,
                      void *user_data)
{
    SPI_HandleTypeDef *handle;
    bsp_spi_dma_entry_t *entry;
    HAL_StatusTypeDef status;

    if ((bus == 0) || (tx_data == 0) || (size == 0U))
    {
        return 0;
    }

    handle = bsp_spi_resolve(bus->id);
    entry = bsp_spi_find_dma_entry(bus->id);
    if ((handle == 0) || (entry == 0) || (handle->hdmatx == 0))
    {
        return -1;
    }

    if (entry->busy != 0U)
    {
        return -1;
    }

    entry->callback = callback;
    entry->user_data = user_data;
    entry->busy = 1U;

    status = HAL_SPI_Transmit_DMA(handle, (uint8_t *)tx_data, size);
    if (status != HAL_OK)
    {
        entry->busy = 0U;
        entry->callback = 0;
        entry->user_data = 0;
        return -1;
    }

    return (int)size;
}

/* 查询指定 SPI TX DMA 是否仍在传输。 */
int bsp_spi_tx_dma_busy(const bsp_spi_t *bus)
{
    bsp_spi_dma_entry_t *entry;

    if (bus == 0)
    {
        return 0;
    }

    entry = bsp_spi_find_dma_entry(bus->id);
    return ((entry != 0) && (entry->busy != 0U)) ? 1 : 0;
}

int bsp_spi_set_prescaler(const bsp_spi_t *bus, bsp_spi_prescaler_t prescaler)
{
    SPI_HandleTypeDef *handle;
    uint32_t hal_prescaler;
    uint32_t timeout = 1000000U;

    if ((bus == 0) || (bsp_spi_resolve_prescaler(prescaler, &hal_prescaler) != 0))
    {
        return -1;
    }
    handle = bsp_spi_resolve(bus->id);
    if ((handle == 0) || (bsp_spi_tx_dma_busy(bus) != 0))
    {
        return -1;
    }
#if defined(SPI_CFG1_MBR)
    while ((HAL_SPI_GetState(handle) != HAL_SPI_STATE_READY) && (timeout > 0U))
#else
    while (((handle->Instance->SR & SPI_SR_BSY) != 0U) && (timeout > 0U))
#endif
    {
        timeout--;
    }
    if (timeout == 0U)
    {
        return -1;
    }
    __HAL_SPI_DISABLE(handle);
#if defined(SPI_CFG1_MBR)
    handle->Instance->CFG1 = (handle->Instance->CFG1 & ~SPI_CFG1_MBR) | hal_prescaler;
#else
    handle->Instance->CR1 = (handle->Instance->CR1 & ~SPI_CR1_BR) | hal_prescaler;
#endif
    handle->Init.BaudRatePrescaler = hal_prescaler;
    __HAL_SPI_ENABLE(handle);
    return 0;
}

/* 统一结束 DMA 事务并转发完成状态。 */
static void bsp_spi_port_dma_finish(SPI_HandleTypeDef *handle, int status)
{
    bsp_spi_dma_entry_t *entry;
    bsp_spi_tx_callback_t callback;
    void *user_data;

    entry = bsp_spi_find_dma_entry_by_handle(handle);
    if (entry == 0)
    {
        return;
    }

    callback = entry->callback;
    user_data = entry->user_data;
    entry->busy = 0U;
    entry->callback = 0;
    entry->user_data = 0;

    if (callback != 0)
    {
        callback(&entry->bus, status, user_data);
    }
}

/* HAL SPI 发送完成回调。 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    bsp_spi_port_dma_finish(hspi, 0);
}

/* HAL SPI 错误回调。 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    bsp_spi_port_dma_finish(hspi, -1);
}

#endif
