#ifndef BASEOS_BSP_TICK_H
#define BASEOS_BSP_TICK_H

#include <stdint.h>

/* 毫秒级阻塞延时 */
void bsp_delay_ms(uint32_t ms);

/* 微秒级阻塞延时 */
void bsp_delay_us(uint32_t us);

/* 获取当前系统毫秒 tick */
uint32_t bsp_tick_get_ms(void);

/* 获取统一的单调微秒时钟，供不同 DMA 采集链路标记同一时间基准。 */
uint64_t bsp_tick_get_us(void);

#endif
