# test

这里放统一 BSP 接口的基础验证 demo。

这层的定位不是“正式设备驱动”，而是：

- 验证 `../include/` 的统一接口是否可用
- 验证当前平台 BSP 是否已经接通
- 为新平台 bring-up 提供最小外设测试入口

当前建议放这类 demo：

- `bsp_uart_demo`
- `bsp_gpio_demo`
- `bsp_adc_demo`
- `bsp_i2c_demo`
- `bsp_soft_i2c_demo`
- `bsp_spi_demo`

推荐规则：

- demo 只依赖 BSP、board 和 shared/common
- demo 不直接依赖 HAL 和 CubeMX 生成头文件
- demo 入口统一使用：
  - `xxx_demo_init(...)`
  - `xxx_demo_poll(void)`

当前仓库第一批基础 demo：

- `bsp_uart_demo`
- `bsp_gpio_demo`
- `bsp_adc_demo`
- `bsp_soft_i2c_demo`

`bsp_soft_i2c_demo` 只扫描指定的 7 位地址范围，不会向未知设备写入数据。推荐先扫描
`0x08`~`0x77`；工装项目的 MCP4725 可直接扫描 `0x60`~`0x61`。
