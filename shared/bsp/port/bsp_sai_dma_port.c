#include "bsp_sai_dma.h"
#include "bsp_port_hal.h"

#ifdef HAL_SAI_MODULE_ENABLED

/* 每个逻辑 SAI 端点保存一个回调，由 HAL 回调按句柄反查并转发。 */
typedef struct
{
    bsp_sai_dma_t sai;
    bsp_sai_dma_callback_t callback;
    void *user_data;
} bsp_sai_dma_entry_t;

static bsp_sai_dma_entry_t g_sai_dma_entries[] = {
    { { BSP_SAI_DMA_ID_1_A }, 0, 0 }
};

/* 将 BSP 逻辑编号映射到 CubeMX 生成的弱 HAL 句柄。 */
static SAI_HandleTypeDef *bsp_sai_dma_resolve(bsp_sai_dma_id_t id)
{
    switch (id)
    {
    case BSP_SAI_DMA_ID_1_A:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hsai_BlockA1) ? &hsai_BlockA1 : 0;
    default:
        return 0;
    }
}

/* 根据逻辑编号查找保存回调和用户参数的端点表项。 */
static bsp_sai_dma_entry_t *bsp_sai_dma_find_entry(bsp_sai_dma_id_t id)
{
    uint32_t index;

    for (index = 0U; index < (sizeof(g_sai_dma_entries) / sizeof(g_sai_dma_entries[0])); ++index)
    {
        if (g_sai_dma_entries[index].sai.id == id)
        {
            return &g_sai_dma_entries[index];
        }
    }
    return 0;
}

/* HAL 回调只提供底层句柄，此处反查对应的 BSP 端点。 */
static bsp_sai_dma_entry_t *bsp_sai_dma_find_entry_by_handle(SAI_HandleTypeDef *handle)
{
    uint32_t index;

    for (index = 0U; index < (sizeof(g_sai_dma_entries) / sizeof(g_sai_dma_entries[0])); ++index)
    {
        if (bsp_sai_dma_resolve(g_sai_dma_entries[index].sai.id) == handle)
        {
            return &g_sai_dma_entries[index];
        }
    }
    return 0;
}

/* 在中断上下文把 HAL 事件转发给已注册的设备驱动回调。 */
static void bsp_sai_dma_dispatch(SAI_HandleTypeDef *handle, bsp_sai_dma_event_t event)
{
    bsp_sai_dma_entry_t *entry = bsp_sai_dma_find_entry_by_handle(handle);

    if ((entry != 0) && (entry->callback != 0))
    {
        entry->callback(&entry->sai, event, entry->user_data);
    }
}

/* 每个逻辑端点只保存一个上层回调，重复注册会替换原回调。 */
int bsp_sai_dma_register_callback(const bsp_sai_dma_t *sai,
                                  bsp_sai_dma_callback_t callback,
                                  void *user_data)
{
    bsp_sai_dma_entry_t *entry;

    if (sai == 0)
    {
        return -1;
    }
    entry = bsp_sai_dma_find_entry(sai->id);
    if (entry == 0)
    {
        return -1;
    }
    entry->callback = callback;
    entry->user_data = user_data;
    return 0;
}

/* 启动前确认 target 提供 SAI 句柄及 RX DMA，避免解引用弱空句柄。 */
int bsp_sai_dma_start(const bsp_sai_dma_t *sai, uint32_t *words, uint16_t word_count)
{
    SAI_HandleTypeDef *handle;

    if ((sai == 0) || (words == 0) || (word_count == 0U))
    {
        return -1;
    }
    handle = bsp_sai_dma_resolve(sai->id);
    if ((handle == 0) || (handle->hdmarx == 0))
    {
        return -1;
    }
    return (HAL_SAI_Receive_DMA(handle, (uint8_t *)words, word_count) == HAL_OK) ? 0 : -1;
}

/* 将 HAL 停止结果转换为 BSP 统一的 0/-1 返回值。 */
int bsp_sai_dma_stop(const bsp_sai_dma_t *sai)
{
    SAI_HandleTypeDef *handle;

    if (sai == 0)
    {
        return -1;
    }
    handle = bsp_sai_dma_resolve(sai->id);
    if (handle == 0)
    {
        return -1;
    }
    return (HAL_SAI_DMAStop(handle) == HAL_OK) ? 0 : -1;
}

/* 同时返回 SAI 外设错误和其 RX DMA 错误，便于上层定位链路故障。 */
int bsp_sai_dma_get_error(const bsp_sai_dma_t *sai,
                          uint32_t *sai_error,
                          uint32_t *dma_error)
{
    SAI_HandleTypeDef *handle;

    if ((sai == 0) || (sai_error == 0) || (dma_error == 0))
    {
        return -1;
    }
    handle = bsp_sai_dma_resolve(sai->id);
    if (handle == 0)
    {
        return -1;
    }
    *sai_error = HAL_SAI_GetError(handle);
    *dma_error = (handle->hdmarx != 0) ? handle->hdmarx->ErrorCode : 0U;
    return 0;
}

/* 以下三个 HAL 弱回调只负责事件转发，具体业务由上层回调决定。 */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    bsp_sai_dma_dispatch(hsai, BSP_SAI_DMA_EVENT_HALF);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    bsp_sai_dma_dispatch(hsai, BSP_SAI_DMA_EVENT_COMPLETE);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    bsp_sai_dma_dispatch(hsai, BSP_SAI_DMA_EVENT_ERROR);
}

#endif
