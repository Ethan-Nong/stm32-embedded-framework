#ifndef BASEOS_BSP_SPI_H
#define BASEOS_BSP_SPI_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑 SPI 总线编号。 */
typedef enum
{
    BSP_SPI_ID_1 = 1,
    BSP_SPI_ID_2,
    BSP_SPI_ID_3
} bsp_spi_id_t;

/* 脱离具体 target 的抽象 SPI 总线句柄。 */
typedef struct
{
    bsp_spi_id_t id;
} bsp_spi_t;

/* 通用 SPI 时钟分频，具体寄存器值由 target port 层转换。 */
typedef enum
{
    BSP_SPI_PRESCALER_2 = 2,
    BSP_SPI_PRESCALER_4 = 4,
    BSP_SPI_PRESCALER_8 = 8,
    BSP_SPI_PRESCALER_16 = 16,
    BSP_SPI_PRESCALER_32 = 32,
    BSP_SPI_PRESCALER_64 = 64,
    BSP_SPI_PRESCALER_128 = 128,
    BSP_SPI_PRESCALER_256 = 256
} bsp_spi_prescaler_t;

typedef void (*bsp_spi_tx_callback_t)(const bsp_spi_t *bus, int status, void *user_data);

/* 获取或释放逻辑 SPI 总线，同一总线上的多个设备必须串行访问。 */
int bsp_spi_lock(const bsp_spi_t *bus, uint32_t timeout_ms);
void bsp_spi_unlock(const bsp_spi_t *bus);
/* 全双工传输，成功返回传输字节数，失败返回负值。 */
int bsp_spi_transfer(const bsp_spi_t *bus, const void *tx_data, void *rx_data, uint16_t size);
/* 仅发送写接口，成功返回传输字节数，失败返回负值。 */
int bsp_spi_write(const bsp_spi_t *bus, const void *tx_data, uint16_t size);
/* DMA 仅发送接口，成功启动返回传输字节数，完成后调用 callback。 */
int bsp_spi_write_dma(const bsp_spi_t *bus,
                      const void *tx_data,
                      uint16_t size,
                      bsp_spi_tx_callback_t callback,
                      void *user_data);
/* 查询当前 SPI TX DMA 是否忙。 */
int bsp_spi_tx_dma_busy(const bsp_spi_t *bus);
/* 等待总线空闲并切换 SPI 时钟分频；成功返回 0，失败返回负值。 */
int bsp_spi_set_prescaler(const bsp_spi_t *bus, bsp_spi_prescaler_t prescaler);

#endif
