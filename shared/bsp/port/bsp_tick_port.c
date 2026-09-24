#include "bsp_tick.h"
#include "bsp_port_hal.h"

/* 毫秒级阻塞延时，直接复用 STM32 HAL tick。 */
void bsp_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* 微秒级阻塞延时，优先使用 DWT 周期计数器。 */
void bsp_delay_us(uint32_t us)
{
    uint32_t cycles;
    uint32_t start;

    if (us == 0U)
    {
        return;
    }

#if defined(DWT) && defined(CoreDebug_DEMCR_TRCENA_Msk) && defined(DWT_CTRL_CYCCNTENA_Msk)
    /* DWT 周期计数器可在 Cortex-M3/M4/M7 上提供稳定的微秒延时。 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    cycles = (SystemCoreClock / 1000000U) * us;
    start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles)
    {
    }
#else
    /* 无 DWT 时使用空循环兜底，精度会受编译优化设置影响。 */
    while (us > 0U)
    {
        volatile uint32_t loop = SystemCoreClock / 8000000U;
        while (loop > 0U)
        {
            loop--;
        }
        us--;
    }
#endif
}

/* 获取当前 HAL 系统 tick，单位为毫秒。 */
uint32_t bsp_tick_get_ms(void)
{
    return HAL_GetTick();
}

/*
 * DWT 的 32 位周期计数器在 H743/400 MHz 下约 10.7 秒回绕一次。
 * 本函数在短临界区内累计相邻读数差值，将其扩展为不会因 DWT 回绕而倒退的微秒时钟。
 */
uint64_t bsp_tick_get_us(void)
{
#if defined(DWT) && defined(CoreDebug_DEMCR_TRCENA_Msk) && defined(DWT_CTRL_CYCCNTENA_Msk)
    static uint64_t elapsed_us;
    static uint32_t last_cycles;
    static uint32_t remainder_cycles;
    static uint8_t initialized;
    uint32_t cycles_per_us;
    uint32_t current_cycles;
    uint32_t delta_cycles;
    uint32_t primask;

    cycles_per_us = SystemCoreClock / 1000000U;
    if (cycles_per_us == 0U)
    {
        return (uint64_t)HAL_GetTick() * 1000U;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    current_cycles = DWT->CYCCNT;
    if (initialized == 0U)
    {
        initialized = 1U;
        last_cycles = current_cycles;
        elapsed_us = (uint64_t)HAL_GetTick() * 1000U;
    }
    else
    {
        delta_cycles = current_cycles - last_cycles;
        last_cycles = current_cycles;
        delta_cycles += remainder_cycles;
        elapsed_us += delta_cycles / cycles_per_us;
        remainder_cycles = delta_cycles % cycles_per_us;
    }
    __set_PRIMASK(primask);
    return elapsed_us;
#else
    return (uint64_t)HAL_GetTick() * 1000U;
#endif
}
