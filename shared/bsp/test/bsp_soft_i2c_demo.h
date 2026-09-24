#ifndef BASEOS_BSP_SOFT_I2C_DEMO_H
#define BASEOS_BSP_SOFT_I2C_DEMO_H

#include <stdint.h>

#include "bsp_soft_i2c.h"
#include "bsp_uart.h"

/*
 * 初始化软件 I2C 地址扫描 Demo。
 *
 * 地址范围为闭区间，必须位于 7 位 I2C 地址范围内。传入 debug_uart 后，
 * Demo 会通过该串口输出扫描到的 ACK 地址和每轮汇总结果。
 */
int bsp_soft_i2c_demo_init(const bsp_soft_i2c_t *test_bus,
                           const bsp_uart_t *debug_uart,
                           uint8_t address_start,
                           uint8_t address_end);

/* 在主循环调用，每轮扫描一个地址，避免长时间阻塞业务循环。 */
void bsp_soft_i2c_demo_poll(void);

#endif
