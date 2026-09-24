#ifndef BASEOS_BSP_SAI_DMA_H
#define BASEOS_BSP_SAI_DMA_H

#include <stdint.h>

/* 提供给设备驱动使用的逻辑 SAI DMA 接收端点。 */
typedef enum
{
    BSP_SAI_DMA_ID_1_A = 0
} bsp_sai_dma_id_t;

/* 脱离具体 target 和 HAL 句柄的 SAI 接收端点。 */
typedef struct
{
    bsp_sai_dma_id_t id;
} bsp_sai_dma_t;

/* 回调在 DMA/SAI 中断上下文执行，只允许置位标志或累加计数。 */
typedef enum
{
    BSP_SAI_DMA_EVENT_HALF = 0,
    BSP_SAI_DMA_EVENT_COMPLETE,
    BSP_SAI_DMA_EVENT_ERROR
} bsp_sai_dma_event_t;

/* SAI DMA 事件回调；sai 指向 BSP 内部保存的逻辑端点。 */
typedef void (*bsp_sai_dma_callback_t)(const bsp_sai_dma_t *sai,
                                       bsp_sai_dma_event_t event,
                                       void *user_data);

/* 注册或注销事件回调；callback 为 0 时注销，成功返回 0。 */
int bsp_sai_dma_register_callback(const bsp_sai_dma_t *sai,
                                  bsp_sai_dma_callback_t callback,
                                  void *user_data);
/* 启动循环 DMA 接收，word_count 为 32 位字数量；成功返回 0。 */
int bsp_sai_dma_start(const bsp_sai_dma_t *sai, uint32_t *words, uint16_t word_count);
/* 停止指定 SAI 的 DMA 接收；成功返回 0。 */
int bsp_sai_dma_stop(const bsp_sai_dma_t *sai);
/* 读取 HAL SAI 和 RX DMA 错误位；参数无效或端点不存在时返回 -1。 */
int bsp_sai_dma_get_error(const bsp_sai_dma_t *sai,
                          uint32_t *sai_error,
                          uint32_t *dma_error);

#endif
