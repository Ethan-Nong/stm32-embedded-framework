#ifndef BASEOS_BSP_UART_H
#define BASEOS_BSP_UART_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑 UART 编号。 */
typedef enum
{
    BSP_UART_ID_1 = 1,
    BSP_UART_ID_2,
    BSP_UART_ID_3,
    BSP_UART_ID_4,
    BSP_UART_ID_5,
    BSP_UART_ID_6,
    BSP_UART_ID_7,
    BSP_UART_ID_8
} bsp_uart_id_t;

/* 脱离具体 target 的抽象 UART 句柄。 */
typedef struct
{
    bsp_uart_id_t id;
} bsp_uart_t;

/* UART中断接收模式；由使用该UART的驱动在注册回调时选择。 */
typedef enum
{
    BSP_UART_RX_MODE_BYTE_IT = 0,
    BSP_UART_RX_MODE_TO_IDLE_IT
} bsp_uart_rx_mode_t;

/* 中断逐字节接收时使用的回调函数。 */
typedef void (*bsp_uart_rx_callback_t)(const bsp_uart_t *uart, uint8_t byte, void *user_data);

/* 阻塞方式发送数据，成功返回发送字节数，失败返回负值。 */
int bsp_uart_write(const bsp_uart_t *uart, const void *data, uint32_t size);
/* 异步发送数据，优先使用 DMA，未配置 DMA 时使用 UART TX 中断。 */
int bsp_uart_write_dma(const bsp_uart_t *uart, const void *data, uint32_t size);
/* 查询当前 UART 异步发送是否忙。 */
int bsp_uart_tx_dma_busy(const bsp_uart_t *uart);
/* 注册字节接收回调，兼容旧驱动，默认使用逐字节中断接收。 */
int bsp_uart_register_rx_callback(const bsp_uart_t *uart, bsp_uart_rx_callback_t callback, void *user_data);
/* 注册字节接收回调并指定接收模式；TO_IDLE模式内部批量接收后仍逐字节回调。 */
int bsp_uart_register_rx_callback_mode(const bsp_uart_t *uart,
                                       bsp_uart_rx_callback_t callback,
                                       void *user_data,
                                       bsp_uart_rx_mode_t mode);
/* 中止旧接收、清除接收错误并按当前port模式重新挂起，用于半双工发送后的接收恢复。 */
int bsp_uart_restart_rx(const bsp_uart_t *uart);
/* 注销已经注册的接收回调。 */
int bsp_uart_unregister_rx_callback(const bsp_uart_t *uart);

#endif
