#include "bsp_timer.h"
#include "bsp_port_hal.h"

#ifdef HAL_TIM_MODULE_ENABLED

/* 定时器中断上下文，用于把 HAL 回调转发给上层。 */
typedef struct
{
    bsp_timer_t timer;
    bsp_timer_callback_t callback;
    void *user_data;
} bsp_timer_entry_t;

static bsp_timer_entry_t g_timer_entries[] = {
    { { BSP_TIMER_ID_1 }, 0, 0 },
    { { BSP_TIMER_ID_2 }, 0, 0 },
    { { BSP_TIMER_ID_3 }, 0, 0 },
    { { BSP_TIMER_ID_4 }, 0, 0 },
    { { BSP_TIMER_ID_5 }, 0, 0 },
    { { BSP_TIMER_ID_6 }, 0, 0 },
};

/* 将 BSP 定时器编号映射到 CubeMX 生成的 TIM 句柄。 */
static TIM_HandleTypeDef *bsp_timer_resolve(bsp_timer_id_t id)
{
    switch (id)
    {
    case BSP_TIMER_ID_1:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim1) ? &htim1 : 0;
    case BSP_TIMER_ID_2:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim2) ? &htim2 : 0;
    case BSP_TIMER_ID_3:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim3) ? &htim3 : 0;
    case BSP_TIMER_ID_4:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim4) ? &htim4 : 0;
    case BSP_TIMER_ID_5:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim5) ? &htim5 : 0;
    case BSP_TIMER_ID_6:
        return BSP_PORT_WEAK_HANDLE_DEFINED(htim6) ? &htim6 : 0;
    default:
        return 0;
    }
}

/* 查找指定 BSP 定时器的回调上下文。 */
static bsp_timer_entry_t *bsp_timer_find_entry(bsp_timer_id_t id)
{
    unsigned int index;

    for (index = 0U; index < (sizeof(g_timer_entries) / sizeof(g_timer_entries[0])); ++index)
    {
        if (g_timer_entries[index].timer.id == id)
        {
            return &g_timer_entries[index];
        }
    }

    return 0;
}

/* 启动 TIM base update interrupt。 */
int bsp_timer_start_it(const bsp_timer_t *timer)
{
    TIM_HandleTypeDef *handle;

    if (timer == 0)
    {
        return -1;
    }

    handle = bsp_timer_resolve(timer->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_TIM_Base_Start_IT(handle) == HAL_OK) ? 0 : -1;
}

/* 停止 TIM base update interrupt。 */
int bsp_timer_stop_it(const bsp_timer_t *timer)
{
    TIM_HandleTypeDef *handle;

    if (timer == 0)
    {
        return -1;
    }

    handle = bsp_timer_resolve(timer->id);
    if (handle == 0)
    {
        return -1;
    }

    return (HAL_TIM_Base_Stop_IT(handle) == HAL_OK) ? 0 : -1;
}

/* 设置自动重装值，并清零计数器和更新标志。 */
int bsp_timer_set_autoreload(const bsp_timer_t *timer, uint32_t autoreload)
{
    TIM_HandleTypeDef *handle;

    if (timer == 0)
    {
        return -1;
    }

    handle = bsp_timer_resolve(timer->id);
    if (handle == 0)
    {
        return -1;
    }

    __HAL_TIM_SET_AUTORELOAD(handle, autoreload);
    __HAL_TIM_SET_COUNTER(handle, 0U);
    __HAL_TIM_CLEAR_FLAG(handle, TIM_FLAG_UPDATE);
    return 0;
}

/* 注册定时器更新中断回调。 */
int bsp_timer_register_period_elapsed_callback(const bsp_timer_t *timer,
                                               bsp_timer_callback_t callback,
                                               void *user_data)
{
    bsp_timer_entry_t *entry;

    if ((timer == 0) || (callback == 0))
    {
        return -1;
    }

    entry = bsp_timer_find_entry(timer->id);
    if (entry == 0)
    {
        return -1;
    }

    entry->callback = callback;
    entry->user_data = user_data;
    return 0;
}

/* HAL 更新中断回调入口，按 TIM 句柄反查 BSP 定时器。 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    bsp_timer_entry_t *entry;
    unsigned int index;

    entry = 0;
    for (index = 0U; index < (sizeof(g_timer_entries) / sizeof(g_timer_entries[0])); ++index)
    {
        if (bsp_timer_resolve(g_timer_entries[index].timer.id) == htim)
        {
            entry = &g_timer_entries[index];
            break;
        }
    }

    if ((entry == 0) || (entry->callback == 0))
    {
        return;
    }

    entry->callback(entry->user_data);
}

#endif
