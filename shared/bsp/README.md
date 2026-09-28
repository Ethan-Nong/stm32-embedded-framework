# 板级支持包

- `include/` 保存 GPIO、UART、SPI 等统一接口。
- `port/` 保存基于 STM32 HAL 和 CubeMX 句柄的默认实现。
- `test/` 保存 BSP 验证程序，不属于正式器件驱动。

GPIO 接口除普通输入输出外，还提供 `bsp_gpio_config_alternate`，供
Board 在 GPIO、UART、SPI 和 I2C 模式间配置 AF 引脚。

平台特有实现放在：

```text
platform/<平台名>/overrides/
```
