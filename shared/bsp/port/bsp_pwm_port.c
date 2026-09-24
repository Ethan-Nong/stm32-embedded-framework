#include "bsp_pwm.h"
#include "bsp_port_hal.h"

#ifdef HAL_TIM_MODULE_ENABLED

/* PWM 适配后的 HAL 后端，包含定时器句柄和 HAL 通道编号。 */
typedef struct
{
    TIM_HandleTypeDef *handle;
    uint32_t channel;
} bsp_pwm_backend_t;

#define BSP_PWM_CHANNEL_INVALID 0xFFFFFFFFU

/* 将 BSP 定时器编号映射到 CubeMX 生成的 TIM 句柄。 */
static TIM_HandleTypeDef *bsp_pwm_resolve_timer(bsp_pwm_timer_id_t timer_id)
{
    switch (timer_id)
    {
    case BSP_PWM_TIMER_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim1) ? &htim1 : 0;
    case BSP_PWM_TIMER_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim2) ? &htim2 : 0;
    case BSP_PWM_TIMER_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim3) ? &htim3 : 0;
    case BSP_PWM_TIMER_ID_4:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim4) ? &htim4 : 0;
    case BSP_PWM_TIMER_ID_5:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim5) ? &htim5 : 0;
    case BSP_PWM_TIMER_ID_6:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim6) ? &htim6 : 0;
    case BSP_PWM_TIMER_ID_7:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim7) ? &htim7 : 0;
    case BSP_PWM_TIMER_ID_8:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim8) ? &htim8 : 0;
    case BSP_PWM_TIMER_ID_9:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim9) ? &htim9 : 0;
    case BSP_PWM_TIMER_ID_10:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim10) ? &htim10 : 0;
    case BSP_PWM_TIMER_ID_11:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim11) ? &htim11 : 0;
    case BSP_PWM_TIMER_ID_12:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim12) ? &htim12 : 0;
    case BSP_PWM_TIMER_ID_13:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim13) ? &htim13 : 0;
    case BSP_PWM_TIMER_ID_14:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim14) ? &htim14 : 0;
    case BSP_PWM_TIMER_ID_15:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim15) ? &htim15 : 0;
    case BSP_PWM_TIMER_ID_16:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim16) ? &htim16 : 0;
    case BSP_PWM_TIMER_ID_17:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim17) ? &htim17 : 0;
    default:
        return 0;
    }
}

/* 将 BSP 通道编号映射到 STM32 HAL 的 TIM_CHANNEL_x。 */
static uint32_t bsp_pwm_resolve_channel(bsp_pwm_channel_t channel)
{
    switch (channel)
    {
    case BSP_PWM_CHANNEL_1:
        return TIM_CHANNEL_1;
    case BSP_PWM_CHANNEL_2:
        return TIM_CHANNEL_2;
    case BSP_PWM_CHANNEL_3:
        return TIM_CHANNEL_3;
    case BSP_PWM_CHANNEL_4:
        return TIM_CHANNEL_4;
    default:
        return BSP_PWM_CHANNEL_INVALID;
    }
}

/* 同时解析 PWM 定时器和通道，供所有 PWM 操作复用。 */
static int bsp_pwm_resolve(const bsp_pwm_t *pwm, bsp_pwm_backend_t *backend)
{
    if (backend == 0)
    {
        return -1;
    }

    if (pwm == 0)
    {
        return -1;
    }

    backend->handle = bsp_pwm_resolve_timer(pwm->timer_id);
    backend->channel = bsp_pwm_resolve_channel(pwm->channel);
    if ((backend->handle == 0) || (backend->channel == BSP_PWM_CHANNEL_INVALID))
    {
        return -1;
    }

    return 0;
}

/* 判断定时器是否挂在 APB2，用于计算真实输入时钟。 */
static uint8_t bsp_pwm_is_apb2_timer(TIM_TypeDef *instance)
{
#if defined(TIM1)
    if (instance == TIM1)
    {
        return 1U;
    }
#endif
#if defined(TIM8)
    if (instance == TIM8)
    {
        return 1U;
    }
#endif
#if defined(TIM9)
    if (instance == TIM9)
    {
        return 1U;
    }
#endif
#if defined(TIM10)
    if (instance == TIM10)
    {
        return 1U;
    }
#endif
#if defined(TIM11)
    if (instance == TIM11)
    {
        return 1U;
    }
#endif
#if defined(TIM15)
    if (instance == TIM15)
    {
        return 1U;
    }
#endif
#if defined(TIM16)
    if (instance == TIM16)
    {
        return 1U;
    }
#endif
#if defined(TIM17)
    if (instance == TIM17)
    {
        return 1U;
    }
#endif
    return 0U;
}

/* 计算定时器输入时钟；APB 分频不为 1 时 TIM 时钟为 PCLK 的 2 倍。 */
static uint32_t bsp_pwm_get_timer_input_clock(TIM_HandleTypeDef *handle)
{
    uint32_t clock_hz;
    uint32_t prescaler_bits;

    if (bsp_pwm_is_apb2_timer(handle->Instance) != 0U)
    {
        clock_hz = HAL_RCC_GetPCLK2Freq();
        prescaler_bits = (RCC->CFGR & RCC_CFGR_PPRE2);
    }
    else
    {
        clock_hz = HAL_RCC_GetPCLK1Freq();
        prescaler_bits = (RCC->CFGR & RCC_CFGR_PPRE1);
    }

    if (prescaler_bits != 0U)
    {
        clock_hz *= 2U;
    }

    return clock_hz;
}

/* 启动指定 PWM 通道输出。 */
int bsp_pwm_start(const bsp_pwm_t *pwm)
{
    bsp_pwm_backend_t backend;

    if ((pwm == 0) || (bsp_pwm_resolve(pwm, &backend) != 0))
    {
        return -1;
    }

    return (HAL_TIM_PWM_Start(backend.handle, backend.channel) == HAL_OK) ? 0 : -1;
}

/* 停止指定 PWM 通道输出。 */
int bsp_pwm_stop(const bsp_pwm_t *pwm)
{
    bsp_pwm_backend_t backend;

    if ((pwm == 0) || (bsp_pwm_resolve(pwm, &backend) != 0))
    {
        return -1;
    }

    return (HAL_TIM_PWM_Stop(backend.handle, backend.channel) == HAL_OK) ? 0 : -1;
}

/* 按千分比更新比较值。 */
int bsp_pwm_set_duty(const bsp_pwm_t *pwm, uint16_t duty_permille)
{
    bsp_pwm_backend_t backend;
    uint32_t period;
    uint32_t compare;

    if ((pwm == 0) || (bsp_pwm_resolve(pwm, &backend) != 0))
    {
        return -1;
    }

    if (duty_permille > 1000U)
    {
        duty_permille = 1000U;
    }

    period = __HAL_TIM_GET_AUTORELOAD(backend.handle) + 1U;
    compare = (period * (uint32_t)duty_permille) / 1000U;
    if (compare > 0U)
    {
        compare -= 1U;
    }

    __HAL_TIM_SET_COMPARE(backend.handle, backend.channel, compare);
    return 0;
}

/* 在保持预分频不变的前提下更新 ARR 和 CCR。 */
int bsp_pwm_set_frequency(const bsp_pwm_t *pwm, uint32_t frequency_hz, uint16_t duty_permille)
{
    bsp_pwm_backend_t backend;
    uint32_t timer_clock_hz;
    uint32_t counter_clock_hz;
    uint32_t period_ticks;
    uint32_t compare;

    if ((pwm == 0) || (frequency_hz == 0U) || (bsp_pwm_resolve(pwm, &backend) != 0))
    {
        return -1;
    }

    if (duty_permille > 1000U)
    {
        duty_permille = 1000U;
    }

    timer_clock_hz = bsp_pwm_get_timer_input_clock(backend.handle);
    counter_clock_hz = timer_clock_hz / (backend.handle->Init.Prescaler + 1U);
    if (counter_clock_hz == 0U)
    {
        return -1;
    }

    period_ticks = (counter_clock_hz + (frequency_hz / 2U)) / frequency_hz;
    if (period_ticks < 2U)
    {
        period_ticks = 2U;
    }

    compare = (period_ticks * (uint32_t)duty_permille) / 1000U;
    if (compare == 0U)
    {
        compare = 1U;
    }
    if (compare >= period_ticks)
    {
        compare = period_ticks - 1U;
    }

    __HAL_TIM_SET_AUTORELOAD(backend.handle, period_ticks - 1U);
    __HAL_TIM_SET_COMPARE(backend.handle, backend.channel, compare);
    __HAL_TIM_SET_COUNTER(backend.handle, 0U);
    return 0;
}

/* 直接设置 CCR，适合定时器计数单位已经配置为 us 的场景。 */
int bsp_pwm_set_pulse_us(const bsp_pwm_t *pwm, uint16_t pulse_us)
{
    bsp_pwm_backend_t backend;

    if ((pwm == 0) || (bsp_pwm_resolve(pwm, &backend) != 0))
    {
        return -1;
    }

    __HAL_TIM_SET_COMPARE(backend.handle, backend.channel, pulse_us);
    return 0;
}

#endif
