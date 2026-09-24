#ifndef BASEOS_BSP_ADC_DMA_H
#define BASEOS_BSP_ADC_DMA_H

#include <stdint.h>

/* DMA 采集使用的逻辑 ADC 编号，与 CubeMX 生成的 ADC1/ADC2/ADC3 对应。 */
typedef enum
{
    BSP_ADC_DMA_ID_1 = 1,
    BSP_ADC_DMA_ID_2,
    BSP_ADC_DMA_ID_3
} bsp_adc_dma_id_t;

/* 脱离具体 target 的 DMA ADC 句柄。 */
typedef struct
{
    bsp_adc_dma_id_t id;
} bsp_adc_dma_t;

/* DMA 回调事件均发生在中断上下文，回调中只应置位标志或累加计数。 */
typedef enum
{
    BSP_ADC_DMA_EVENT_HALF = 0,
    BSP_ADC_DMA_EVENT_COMPLETE,
    BSP_ADC_DMA_EVENT_ERROR
} bsp_adc_dma_event_t;

/* ADC DMA 事件回调；由 HAL 中断回调转发，不能在其中执行耗时业务。 */
typedef void (*bsp_adc_dma_callback_t)(const bsp_adc_dma_t *adc,
                                       bsp_adc_dma_event_t event,
                                       void *user_data);

/* 执行 ADC 偏移和线性校准；不支持校准的 target 直接返回成功，失败返回 -1。 */
int bsp_adc_dma_calibrate(const bsp_adc_dma_t *adc);
/* 注册 DMA 半满、满和错误回调；callback 为 0 时注销回调，成功返回 0。 */
int bsp_adc_dma_register_callback(const bsp_adc_dma_t *adc,
                                  bsp_adc_dma_callback_t callback,
                                  void *user_data);
/* 启动循环 DMA 采集，sample_count 是 uint16_t 采样值的总数；成功返回 0。 */
int bsp_adc_dma_start(const bsp_adc_dma_t *adc, uint16_t *samples, uint32_t sample_count);
/* 停止 DMA 采集；成功返回 0。 */
int bsp_adc_dma_stop(const bsp_adc_dma_t *adc);
/* 读取 HAL 的 ADC/DMA 错误位，供应用日志和故障诊断使用；失败返回 -1。 */
int bsp_adc_dma_get_error(const bsp_adc_dma_t *adc, uint32_t *adc_error, uint32_t *dma_error);

#endif
