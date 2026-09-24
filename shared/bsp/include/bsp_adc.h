#ifndef BASEOS_BSP_ADC_H
#define BASEOS_BSP_ADC_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑 ADC 编号。 */
typedef enum
{
    BSP_ADC_ID_1 = 1,
    BSP_ADC_ID_2
} bsp_adc_id_t;

/* 脱离具体 target 的抽象 ADC 句柄。 */
typedef struct
{
    bsp_adc_id_t id;
} bsp_adc_t;

/* 单个 ADC 通道的阻塞采样配置。 */
typedef struct
{
    uint32_t channel;
    uint32_t sample_time;
} bsp_adc_channel_cfg_t;

/* 执行一次阻塞采样并返回原始值。 */
int bsp_adc_read(const bsp_adc_t *adc, uint32_t *value);
/* 阻塞采集指定单通道并返回原始值。 */
int bsp_adc_read_channel(const bsp_adc_t *adc, const bsp_adc_channel_cfg_t *channel_cfg, uint32_t *value);
/* 依次阻塞采集多个通道并返回各自原始值。 */
int bsp_adc_read_channels(const bsp_adc_t *adc, const bsp_adc_channel_cfg_t *channel_cfgs,
                          uint32_t channel_count, uint32_t *values);

#endif
