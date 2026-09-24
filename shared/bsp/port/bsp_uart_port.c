#include "bsp_uart.h"
#include "bsp_port_hal.h"

#include <string.h>

#ifdef HAL_UART_MODULE_ENABLED

#define BSP_UART_WRITE_TIMEOUT_MS 1000U
#define BSP_UART_DMA_TX_BUFFER_SIZE 256U
#define BSP_UART_IDLE_RX_BUFFER_SIZE 512U
#define BSP_UART_DMA_BUFFER __attribute__((section("UART_DMA_BUFFER"), aligned(32)))

/* UART收发上下文：同时保留普通单字节RX和TO_IDLE批量RX所需的缓存。 */
typedef struct
{
    bsp_uart_t uart;
    bsp_uart_rx_callback_t callback;
    void *user_data;
    uint8_t rx_byte;
    volatile uint8_t tx_busy;
    uint8_t idle_rx_buffer[BSP_UART_IDLE_RX_BUFFER_SIZE];
    uint8_t idle_dispatch_buffer[BSP_UART_IDLE_RX_BUFFER_SIZE];
    bsp_uart_rx_mode_t rx_mode;
} bsp_uart_rx_entry_t;

/* 接收状态仅由 CPU 访问，保留在普通 RAM。 */
static bsp_uart_rx_entry_t g_uart_rx_entries[] = {
    { { BSP_UART_ID_1 } },
    { { BSP_UART_ID_2 } },
    { { BSP_UART_ID_3 } },
    { { BSP_UART_ID_4 } },
    { { BSP_UART_ID_5 } },
    { { BSP_UART_ID_6 } },
    { { BSP_UART_ID_7 } },
    { { BSP_UART_ID_8 } },
};

/* 只有 DMA 读取的发送字节需要放入 D2 SRAM，不能影响接收注册表。 */
static uint8_t g_uart_tx_buffers[BSP_UART_ID_8][BSP_UART_DMA_TX_BUFFER_SIZE]
    BSP_UART_DMA_BUFFER;

static bsp_uart_rx_entry_t *bsp_uart_find_entry_by_handle(UART_HandleTypeDef *handle);
static void bsp_uart_port_tx_complete(UART_HandleTypeDef *handle);
static void bsp_uart_port_error(UART_HandleTypeDef *handle);

/* 将 BSP UART 编号映射到 CubeMX 生成的 UART 句柄。 */
static UART_HandleTypeDef *bsp_uart_resolve(bsp_uart_id_t id)
{
    switch (id)
    {
    case BSP_UART_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart1) ? &huart1 : 0;
    case BSP_UART_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart2) ? &huart2 : 0;
    case BSP_UART_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart3) ? &huart3 : 0;
    case BSP_UART_ID_4:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart4) ? &huart4 : 0;
    case BSP_UART_ID_5:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart5) ? &huart5 : 0;
    case BSP_UART_ID_6:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart6) ? &huart6 : 0;
    case BSP_UART_ID_7:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart7) ? &huart7 : 0;
    case BSP_UART_ID_8:
        return BSP_PORT_WEAK_HANDLE_DEFINED(huart8) ? &huart8 : 0;
    default:
        return 0;
    }
}

/* 按 BSP UART 编号查找收发上下文。 */
static bsp_uart_rx_entry_t *bsp_uart_find_entry(bsp_uart_id_t id)
{
    unsigned int index;

    for (index = 0U; index < (sizeof(g_uart_rx_entries) / sizeof(g_uart_rx_entries[0])); ++index)
    {
        if (g_uart_rx_entries[index].uart.id == id)
        {
            return &g_uart_rx_entries[index];
        }
    }

    return 0;
}

static uint8_t *bsp_uart_get_tx_buffer(bsp_uart_id_t id)
{
    if ((id < BSP_UART_ID_1) || (id > BSP_UART_ID_8))
    {
        return 0;
    }

    return g_uart_tx_buffers[(uint32_t)id - (uint32_t)BSP_UART_ID_1];
}

/* 按注册时选择的模式启动逐字节接收或空闲事件批量接收。 */
static int bsp_uart_start_rx_it(bsp_uart_rx_entry_t *entry)
{
    UART_HandleTypeDef *handle;
    HAL_StatusTypeDef status;

    if (entry == 0)
    {
        return -1;
    }

    handle = bsp_uart_resolve(entry->uart.id);
    if (handle == 0)
    {
        return -1;
    }
    if (entry->rx_mode == BSP_UART_RX_MODE_TO_IDLE_IT)
    {
        status = HAL_UARTEx_ReceiveToIdle_IT(handle, entry->idle_rx_buffer,
                                             BSP_UART_IDLE_RX_BUFFER_SIZE);
    }
    else
    {
        status = HAL_UART_Receive_IT(handle, &entry->rx_byte, 1U);
    }
    return ((status == HAL_OK) || (status == HAL_BUSY)) ? 0 : -1;
}

/* 阻塞式发送。 */
int bsp_uart_write(const bsp_uart_t *uart, const void *data, uint32_t size)
{
    UART_HandleTypeDef *handle;

    if ((uart == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    if (size > 0xFFFFU)
    {
        return -1;
    }

    handle = bsp_uart_resolve(uart->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_UART_Transmit(handle,
                              (uint8_t *)data,
                              (uint16_t)size,
                              BSP_UART_WRITE_TIMEOUT_MS) == HAL_OK) ? (int)size : -1;
}

/* 异步发送：有 DMA 时使用 DMA，否则退回 TX 中断。 */
int bsp_uart_write_dma(const bsp_uart_t *uart, const void *data, uint32_t size)
{
    UART_HandleTypeDef *handle;
    bsp_uart_rx_entry_t *entry;
    uint8_t *tx_buffer;
    HAL_StatusTypeDef status;

    if ((uart == 0) || (data == 0) || (size == 0U))
    {
        return 0;
    }

    if ((size > BSP_UART_DMA_TX_BUFFER_SIZE) || (size > 0xFFFFU))
    {
        return -1;
    }

    handle = bsp_uart_resolve(uart->id);
    entry = bsp_uart_find_entry(uart->id);
    if ((handle == 0) || (entry == 0))
    {
        return -1;
    }
    tx_buffer = bsp_uart_get_tx_buffer(entry->uart.id);
    if (tx_buffer == 0)
    {
        return -1;
    }

    if (entry->tx_busy != 0U)
    {
        return 0;
    }

    memcpy(tx_buffer, data, (size_t)size);
    entry->tx_busy = 1U;
    if (handle->hdmatx != 0)
    {
        status = HAL_UART_Transmit_DMA(handle, tx_buffer, (uint16_t)size);
    }
    else
    {
        status = HAL_UART_Transmit_IT(handle, tx_buffer, (uint16_t)size);
    }
    if (status != HAL_OK)
    {
        entry->tx_busy = 0U;
        return (status == HAL_BUSY) ? 0 : -1;
    }

    return (int)size;
}

/* 查询异步发送是否仍在进行。 */
int bsp_uart_tx_dma_busy(const bsp_uart_t *uart)
{
    bsp_uart_rx_entry_t *entry;

    if (uart == 0)
    {
        return 0;
    }

    entry = bsp_uart_find_entry(uart->id);
    return ((entry != 0) && (entry->tx_busy != 0U)) ? 1 : 0;
}

/* 兼容旧驱动：未指定模式时保持原来的逐字节中断接收。 */
int bsp_uart_register_rx_callback(const bsp_uart_t *uart, bsp_uart_rx_callback_t callback, void *user_data)
{
    return bsp_uart_register_rx_callback_mode(uart, callback, user_data,
                                              BSP_UART_RX_MODE_BYTE_IT);
}

/* 注册接收回调和接收模式；空闲模式仅改变底层收包方式，不改变逐字节回调契约。 */
int bsp_uart_register_rx_callback_mode(const bsp_uart_t *uart,
                                       bsp_uart_rx_callback_t callback,
                                       void *user_data,
                                       bsp_uart_rx_mode_t mode)
{
    bsp_uart_rx_entry_t *entry;
    int ret;

    if ((uart == 0) || (callback == 0) ||
        ((mode != BSP_UART_RX_MODE_BYTE_IT) &&
         (mode != BSP_UART_RX_MODE_TO_IDLE_IT)))
    {
        return -1;
    }

    entry = bsp_uart_find_entry(uart->id);
    if (entry == 0)
    {
        return -1;
    }

    entry->callback = callback;
    entry->user_data = user_data;
    entry->rx_mode = mode;
    ret = bsp_uart_start_rx_it(entry);
    if (ret != 0)
    {
        entry->callback = 0;
        entry->user_data = 0;
        entry->rx_mode = BSP_UART_RX_MODE_BYTE_IT;
    }
    return ret;
}

/*
 * 半双工发送期间接收器可能产生ORE，HAL也可能保留旧的BUSY接收状态。
 * DE切回接收后中止旧请求、清除ORE，再按该UART原有模式重新挂起接收。
 */
int bsp_uart_restart_rx(const bsp_uart_t *uart)
{
    UART_HandleTypeDef *handle;
    bsp_uart_rx_entry_t *entry;

    if (uart == 0)
    {
        return -1;
    }
    handle = bsp_uart_resolve(uart->id);
    entry = bsp_uart_find_entry(uart->id);
    if ((handle == 0) || (entry == 0) || (entry->callback == 0))
    {
        return -1;
    }

    (void)HAL_UART_AbortReceive(handle);
    __HAL_UART_CLEAR_OREFLAG(handle);
    return bsp_uart_start_rx_it(entry);
}

/* 注销接收回调；如之前已注册，则中止当前接收请求。 */
int bsp_uart_unregister_rx_callback(const bsp_uart_t *uart)
{
    UART_HandleTypeDef *handle;
    bsp_uart_rx_entry_t *entry;
    uint8_t was_registered;

    if (uart == 0)
    {
        return -1;
    }

    entry = bsp_uart_find_entry(uart->id);
    if (entry == 0)
    {
        return -1;
    }

    was_registered = (entry->callback != 0) ? 1U : 0U;
    entry->callback = 0;
    entry->user_data = 0;
    entry->rx_mode = BSP_UART_RX_MODE_BYTE_IT;

    handle = bsp_uart_resolve(uart->id);
    if ((was_registered != 0U) && (handle != 0))
    {
        (void)HAL_UART_AbortReceive(handle);
    }

    return 0;
}

/* HAL RX 完成回调：转发字节后继续投递下一次单字节接收。 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    bsp_uart_rx_entry_t *entry;
    unsigned int index;

    entry = 0;
    for (index = 0U; index < (sizeof(g_uart_rx_entries) / sizeof(g_uart_rx_entries[0])); ++index)
    {
        if (bsp_uart_resolve(g_uart_rx_entries[index].uart.id) == huart)
        {
            entry = &g_uart_rx_entries[index];
            break;
        }
    }

    if (entry == 0)
    {
        return;
    }

    if (entry->rx_mode != BSP_UART_RX_MODE_BYTE_IT)
    {
        return;
    }

    if (entry->callback != 0)
    {
        entry->callback(&entry->uart, entry->rx_byte, entry->user_data);
    }

    if (entry->callback != 0)
    {
        (void)bsp_uart_start_rx_it(entry);
    }
}

/* TO_IDLE模式在空闲或缓冲区满时批量收包，再按原接口逐字节转交上层。 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    bsp_uart_rx_entry_t *entry;
    uint16_t index;
    uint8_t *received;

    entry = bsp_uart_find_entry_by_handle(huart);
    if ((entry == 0) || (entry->rx_mode != BSP_UART_RX_MODE_TO_IDLE_IT))
    {
        return;
    }
    if (size > BSP_UART_IDLE_RX_BUFFER_SIZE)
    {
        size = BSP_UART_IDLE_RX_BUFFER_SIZE;
    }
    received = entry->idle_dispatch_buffer;
    if (entry->callback != 0)
    {
        /* Preserve the completed batch, then re-arm reception before invoking
           upper-layer callbacks.  The 512-byte persistent shadow avoids a
           large ISR stack frame and normally holds a complete EC800K
           +QMTRECV JSON URC, so long downlinks do not create a receive gap at
           every old 64-byte boundary.  Re-arming afterwards left a loss
           window between adjacent modem bursts. */
        memcpy(received, entry->idle_rx_buffer, size);
        (void)bsp_uart_start_rx_it(entry);
        for (index = 0U; index < size; ++index)
        {
            entry->callback(&entry->uart, received[index], entry->user_data);
        }
    }
}

/* HAL TX 完成回调：清除异步发送 busy 状态。 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    bsp_uart_port_tx_complete(huart);
}

/* HAL 错误回调：释放 TX 状态，并在需要时恢复 RX 接收。 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    bsp_uart_port_error(huart);
}

/* 从 HAL 句柄反查 UART 上下文。 */
static bsp_uart_rx_entry_t *bsp_uart_find_entry_by_handle(UART_HandleTypeDef *handle)
{
    unsigned int index;

    for (index = 0U; index < (sizeof(g_uart_rx_entries) / sizeof(g_uart_rx_entries[0])); ++index)
    {
        if (bsp_uart_resolve(g_uart_rx_entries[index].uart.id) == handle)
        {
            return &g_uart_rx_entries[index];
        }
    }

    return 0;
}

/* 发送完成时清除 busy 标志。 */
static void bsp_uart_port_tx_complete(UART_HandleTypeDef *handle)
{
    bsp_uart_rx_entry_t *entry;

    entry = bsp_uart_find_entry_by_handle(handle);
    if (entry != 0)
    {
        entry->tx_busy = 0U;
    }
}

/* 错误发生时清除 busy，并尝试恢复已注册的 RX。 */
static void bsp_uart_port_error(UART_HandleTypeDef *handle)
{
    bsp_uart_rx_entry_t *entry;

    entry = bsp_uart_find_entry_by_handle(handle);
    if (entry != 0)
    {
        entry->tx_busy = 0U;
        if (entry->callback != 0)
        {
            (void)HAL_UART_AbortReceive(handle);
            __HAL_UART_CLEAR_OREFLAG(handle);
            __HAL_UART_CLEAR_NEFLAG(handle);
            __HAL_UART_CLEAR_FEFLAG(handle);
            (void)bsp_uart_start_rx_it(entry);
        }
    }
}

#endif
