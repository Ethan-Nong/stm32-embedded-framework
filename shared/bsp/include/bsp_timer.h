#ifndef BASEOS_BSP_TIMER_H
#define BASEOS_BSP_TIMER_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑定时器编号。 */
typedef enum
{
    BSP_TIMER_ID_1 = 1,
    BSP_TIMER_ID_2,
    BSP_TIMER_ID_3,
    BSP_TIMER_ID_4,
    BSP_TIMER_ID_5,
    BSP_TIMER_ID_6
} bsp_timer_id_t;

/* 定时器到期回调。 */
typedef void (*bsp_timer_callback_t)(void *user_data);

/* 脱离具体 target 的抽象定时器句柄。 */
typedef struct
{
    bsp_timer_id_t id;
} bsp_timer_t;

/* 启动定时器更新中断模式。 */
int bsp_timer_start_it(const bsp_timer_t *timer);
/* 停止定时器更新中断模式。 */
int bsp_timer_stop_it(const bsp_timer_t *timer);
/* 设置自动重装值并清零计数器。单位由具体定时器预分频决定。 */
int bsp_timer_set_autoreload(const bsp_timer_t *timer, uint32_t autoreload);
/* 注册定时器更新到期回调。 */
int bsp_timer_register_period_elapsed_callback(const bsp_timer_t *timer,
                                               bsp_timer_callback_t callback,
                                               void *user_data);

#endif
