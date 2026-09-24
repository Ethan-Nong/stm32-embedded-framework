#include "bsp_adc_dma.h"
#include "bsp_port_hal.h"

#ifdef HAL_ADC_MODULE_ENABLED

/* 每个 ADC 仅保存一个 DMA 回调；HAL 回调负责按句柄转发。 */
typedef struct
{
    bsp_adc_dma_t adc;
    bsp_adc_dma_callback_t callback;
    void *user_data;
} bsp_adc_dma_entry_t;

static bsp_adc_dma_entry_t g_adc_dma_entries[] = {
    { { BSP_ADC_DMA_ID_1 }, 0, 0 },
    { { BSP_ADC_DMA_ID_2 }, 0, 0 },
    { { BSP_ADC_DMA_ID_3 }, 0, 0 }
};

/* 将 BSP ADC 编号映射到 CubeMX 生成的 ADC 句柄。 */
static ADC_HandleTypeDef *bsp_adc_dma_resolve(bsp_adc_dma_id_t id)
{
    switch (id)
    {
    case BSP_ADC_DMA_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hadc1) ? &hadc1 : 0;
    case BSP_ADC_DMA_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hadc2) ? &hadc2 : 0;
    case BSP_ADC_DMA_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hadc3) ? &hadc3 : 0;
    default:
        return 0;
    }
}

/* 查找保存指定 ADC 回调的上下文。 */
static bsp_adc_dma_entry_t *bsp_adc_dma_find_entry(bsp_adc_dma_id_t id)
{
    uint32_t index;

    for (index = 0U; index < (sizeof(g_adc_dma_entries) / sizeof(g_adc_dma_entries[0])); ++index)
    {
        if (g_adc_dma_entries[index].adc.id == id)
        {
            return &g_adc_dma_entries[index];
        }
    }

    return 0;
}

/* 从 HAL 句柄反查 BSP ADC 回调上下文。 */
static bsp_adc_dma_entry_t *bsp_adc_dma_find_entry_by_handle(ADC_HandleTypeDef *handle)
{
    uint32_t index;

    for (index = 0U; index < (sizeof(g_adc_dma_entries) / sizeof(g_adc_dma_entries[0])); ++index)
    {
        if (bsp_adc_dma_resolve(g_adc_dma_entries[index].adc.id) == handle)
        {
            return &g_adc_dma_entries[index];
        }
    }

    return 0;
}

/* 在 HAL DMA 回调中转发事件，应用回调必须保持极短。 */
static void bsp_adc_dma_dispatch(ADC_HandleTypeDef *handle, bsp_adc_dma_event_t event)
{
    bsp_adc_dma_entry_t *entry = bsp_adc_dma_find_entry_by_handle(handle);

    if ((entry != 0) && (entry->callback != 0))
    {
        entry->callback(&entry->adc, event, entry->user_data);
    }
}

/* 校准策略由当前 target 的 HAL 能力决定，BSP 对上统一返回 0/-1。 */
int bsp_adc_dma_calibrate(const bsp_adc_dma_t *adc)
{
    ADC_HandleTypeDef *handle;

    if (adc == 0)
    {
        return -1;
    }

    handle = bsp_adc_dma_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

#if defined(ADC_CALIB_OFFSET_LINEARITY)
    return (HAL_ADCEx_Calibration_Start(handle, ADC_CALIB_OFFSET_LINEARITY,
                                        ADC_SINGLE_ENDED) == HAL_OK) ? 0 : -1;
#else
    /* 较早的 STM32 HAL 没有统一的线性校准接口，交给 target 覆盖实现。 */
    return 0;
#endif
}

/* 每个逻辑 ADC 只保存一个上层回调，重复注册会替换原回调。 */
int bsp_adc_dma_register_callback(const bsp_adc_dma_t *adc,
                                  bsp_adc_dma_callback_t callback,
                                  void *user_data)
{
    bsp_adc_dma_entry_t *entry;

    if (adc == 0)
    {
        return -1;
    }

    entry = bsp_adc_dma_find_entry(adc->id);
    if (entry == 0)
    {
        return -1;
    }

    entry->callback = callback;
    entry->user_data = user_data;
    return 0;
}

/* CubeMX 必须已配置循环 DMA 和外部触发，本函数只负责启动接收。 */
int bsp_adc_dma_start(const bsp_adc_dma_t *adc, uint16_t *samples, uint32_t sample_count)
{
    ADC_HandleTypeDef *handle;

    if ((adc == 0) || (samples == 0) || (sample_count == 0U))
    {
        return -1;
    }

    handle = bsp_adc_dma_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_ADC_Start_DMA(handle, (uint32_t *)samples, sample_count) == HAL_OK) ? 0 : -1;
}

/* 停止指定 ADC 的 HAL DMA 采集。 */
int bsp_adc_dma_stop(const bsp_adc_dma_t *adc)
{
    ADC_HandleTypeDef *handle;

    if (adc == 0)
    {
        return -1;
    }

    handle = bsp_adc_dma_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_ADC_Stop_DMA(handle) == HAL_OK) ? 0 : -1;
}

/* 同时读取 ADC 和关联 DMA 的错误位。 */
int bsp_adc_dma_get_error(const bsp_adc_dma_t *adc, uint32_t *adc_error, uint32_t *dma_error)
{
    ADC_HandleTypeDef *handle;

    if ((adc == 0) || (adc_error == 0) || (dma_error == 0))
    {
        return -1;
    }

    handle = bsp_adc_dma_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

    *adc_error = handle->ErrorCode;
    *dma_error = (handle->DMA_Handle != 0) ? handle->DMA_Handle->ErrorCode : 0U;
    return 0;
}

/* 以下三个 HAL 回调统一按 ADC 句柄转发到已注册的 BSP 回调。 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    bsp_adc_dma_dispatch(hadc, BSP_ADC_DMA_EVENT_HALF);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    bsp_adc_dma_dispatch(hadc, BSP_ADC_DMA_EVENT_COMPLETE);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    bsp_adc_dma_dispatch(hadc, BSP_ADC_DMA_EVENT_ERROR);
}

#endif
