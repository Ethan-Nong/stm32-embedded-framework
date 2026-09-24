#include "bsp_adc.h"
#include "bsp_port_hal.h"

#ifdef HAL_ADC_MODULE_ENABLED

#define BSP_ADC_POLL_TIMEOUT 0xffffU
#ifdef ADC_REGULAR_RANK_1
#define BSP_ADC_REGULAR_RANK_1 ADC_REGULAR_RANK_1
#else
#define BSP_ADC_REGULAR_RANK_1 1U
#endif

/* 保存 ADC 当前配置，临时切换通道采样后再恢复原状态。 */
typedef struct
{
    ADC_InitTypeDef init;
#if defined(ADC_CFGR1_CHSELRMOD)
    uint32_t cfgr1;
    uint32_t cfgr2;
    uint32_t smpr;
    uint32_t chselr;
    uint32_t regular_seq_ranks;
#elif defined(ADC_SQR1_L)
    uint32_t cr1;
    uint32_t cr2;
    uint32_t smpr1;
    uint32_t smpr2;
    uint32_t sqr1;
    uint32_t sqr2;
    uint32_t sqr3;
#if defined(ADC_CCR_TSVREFE) && defined(ADC_COMMON_REGISTER)
    uint32_t common_ccr;
#endif
#endif
} bsp_adc_backup_t;

/* 将 BSP ADC 编号映射到 CubeMX 生成的 ADC 句柄。 */
static ADC_HandleTypeDef *bsp_adc_resolve(bsp_adc_id_t id)
{
    switch (id)
    {
    case BSP_ADC_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hadc1) ? &hadc1 : 0;
    case BSP_ADC_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(hadc2) ? &hadc2 : 0;
    default:
        return 0;
    }
}

/* 启动一次阻塞式转换并读取原始采样值。 */
static int bsp_adc_blocking_read(ADC_HandleTypeDef *handle, uint32_t *value)
{
    if (HAL_ADC_Start(handle) != HAL_OK)
    {
        return -1;
    }

    if (HAL_ADC_PollForConversion(handle, BSP_ADC_POLL_TIMEOUT) != HAL_OK)
    {
        (void)HAL_ADC_Stop(handle);
        return -1;
    }

    *value = HAL_ADC_GetValue(handle);
    (void)HAL_ADC_Stop(handle);
    return 0;
}

/* 备份当前 ADC 初始化参数和关键寄存器。 */
static void bsp_adc_backup_save(ADC_HandleTypeDef *handle, bsp_adc_backup_t *backup)
{
    backup->init = handle->Init;

#if defined(ADC_CFGR1_CHSELRMOD)
    backup->cfgr1 = handle->Instance->CFGR1;
    backup->cfgr2 = handle->Instance->CFGR2;
    backup->smpr = handle->Instance->SMPR;
    backup->chselr = handle->Instance->CHSELR;
    backup->regular_seq_ranks = handle->ADCGroupRegularSequencerRanks;
#elif defined(ADC_SQR1_L)
    backup->cr1 = handle->Instance->CR1;
    backup->cr2 = handle->Instance->CR2;
    backup->smpr1 = handle->Instance->SMPR1;
    backup->smpr2 = handle->Instance->SMPR2;
    backup->sqr1 = handle->Instance->SQR1;
    backup->sqr2 = handle->Instance->SQR2;
    backup->sqr3 = handle->Instance->SQR3;
#if defined(ADC_CCR_TSVREFE) && defined(ADC_COMMON_REGISTER)
    backup->common_ccr = ADC_COMMON_REGISTER(handle)->CCR;
#endif
#endif
}

/* 恢复临时采样前的 ADC 配置。 */
static int bsp_adc_backup_restore(ADC_HandleTypeDef *handle, const bsp_adc_backup_t *backup)
{
    handle->Init = backup->init;
    if (HAL_ADC_Init(handle) != HAL_OK)
    {
        return -1;
    }

#if defined(ADC_CFGR1_CHSELRMOD)
    handle->Instance->CFGR1 = backup->cfgr1;
    handle->Instance->CFGR2 = backup->cfgr2;
    handle->Instance->SMPR = backup->smpr;
    handle->Instance->CHSELR = backup->chselr;
    handle->ADCGroupRegularSequencerRanks = backup->regular_seq_ranks;
#elif defined(ADC_SQR1_L)
    handle->Instance->CR1 = backup->cr1;
    handle->Instance->CR2 = backup->cr2;
    handle->Instance->SMPR1 = backup->smpr1;
    handle->Instance->SMPR2 = backup->smpr2;
    handle->Instance->SQR1 = backup->sqr1;
    handle->Instance->SQR2 = backup->sqr2;
    handle->Instance->SQR3 = backup->sqr3;
#if defined(ADC_CCR_TSVREFE) && defined(ADC_COMMON_REGISTER)
    ADC_COMMON_REGISTER(handle)->CCR = backup->common_ccr;
#endif
#endif

    return 0;
}

/* 将 ADC 临时切换为单通道、软件触发、阻塞读取模式。 */
static void bsp_adc_prepare_single_mode(ADC_HandleTypeDef *handle)
{
#if defined(ADC_CFGR1_CHSELRMOD)
    handle->Init.ScanConvMode = ADC_SCAN_DISABLE;
    handle->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    handle->Init.ContinuousConvMode = DISABLE;
    handle->Init.NbrOfConversion = 1;
    handle->Init.DiscontinuousConvMode = DISABLE;
    handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    handle->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    handle->Init.DMAContinuousRequests = DISABLE;
#elif defined(ADC_CR2_EOCS)
    handle->Init.ScanConvMode = DISABLE;
    handle->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    handle->Init.ContinuousConvMode = DISABLE;
    handle->Init.NbrOfConversion = 1;
    handle->Init.DiscontinuousConvMode = DISABLE;
    handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
#else
    handle->Init.ScanConvMode = ADC_SCAN_DISABLE;
    handle->Init.ContinuousConvMode = DISABLE;
    handle->Init.DiscontinuousConvMode = DISABLE;
    handle->Init.NbrOfConversion = 1;
    handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
#endif
}

/* 配置单个规则通道和采样时间。 */
static int bsp_adc_configure_channel(ADC_HandleTypeDef *handle, const bsp_adc_channel_cfg_t *channel_cfg)
{
    ADC_ChannelConfTypeDef config = {0};

    config.Channel = channel_cfg->channel;
    config.Rank = BSP_ADC_REGULAR_RANK_1;
    config.SamplingTime = channel_cfg->sample_time;

    return (HAL_ADC_ConfigChannel(handle, &config) == HAL_OK) ? 0 : -1;
}

/* 依次采集多个通道，并在结束后恢复 ADC 原配置。 */
static int bsp_adc_read_sequence(const bsp_adc_t *adc, const bsp_adc_channel_cfg_t *channel_cfgs,
                                 uint32_t channel_count, uint32_t *values)
{
    ADC_HandleTypeDef *handle;
    bsp_adc_backup_t backup;
    uint32_t index;
    int status;

    if ((adc == 0) || (channel_cfgs == 0) || (values == 0) || (channel_count == 0U))
    {
        return -1;
    }

    handle = bsp_adc_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

    bsp_adc_backup_save(handle, &backup);
    (void)HAL_ADC_Stop(handle);

    bsp_adc_prepare_single_mode(handle);
    if (HAL_ADC_Init(handle) != HAL_OK)
    {
        (void)bsp_adc_backup_restore(handle, &backup);
        return -1;
    }

    status = 0;
    for (index = 0; index < channel_count; index++)
    {
        if ((bsp_adc_configure_channel(handle, &channel_cfgs[index]) != 0) ||
            (bsp_adc_blocking_read(handle, &values[index]) != 0))
        {
            status = -1;
            break;
        }
    }

    if (bsp_adc_backup_restore(handle, &backup) != 0)
    {
        status = -1;
    }

    return status;
}

/* 读取 ADC 当前已配置通道的一次转换结果。 */
int bsp_adc_read(const bsp_adc_t *adc, uint32_t *value)
{
    ADC_HandleTypeDef *handle;

    if ((adc == 0) || (value == 0))
    {
        return -1;
    }

    handle = bsp_adc_resolve(adc->id);
    if (handle == 0)
    {
        return -1;
    }

    return bsp_adc_blocking_read(handle, value);
}

/* 临时配置并读取单个指定通道。 */
int bsp_adc_read_channel(const bsp_adc_t *adc, const bsp_adc_channel_cfg_t *channel_cfg, uint32_t *value)
{
    return bsp_adc_read_sequence(adc, channel_cfg, 1U, value);
}

/* 临时配置并按顺序读取多个指定通道。 */
int bsp_adc_read_channels(const bsp_adc_t *adc, const bsp_adc_channel_cfg_t *channel_cfgs,
                          uint32_t channel_count, uint32_t *values)
{
    return bsp_adc_read_sequence(adc, channel_cfgs, channel_count, values);
}

#endif
