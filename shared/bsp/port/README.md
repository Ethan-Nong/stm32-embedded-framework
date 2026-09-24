# port

这里放 BSP 的默认公共实现。

它的职责是：

- 把统一的 BSP 接口接到 STM32 HAL + CubeMX 默认句柄命名上
- 提供跨平台可复用的默认 `.c` 实现

适合放这里的通常是：

- `bsp_gpio_port.c`
- `bsp_tick_port.c`
- `bsp_uart_port.c`
- `bsp_i2c_port.c`
- `bsp_soft_i2c_port.c`
- `bsp_spi_port.c`

使用规则：

- 如果平台没有特殊差异，Keil 直接引用这里的实现
- 如果平台有特殊需求，就在 `../overrides/` 放差异化实现
