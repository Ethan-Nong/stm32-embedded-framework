# 板级支持包

- `include/` 保存 GPIO、UART、SPI 等统一接口。
- `port/` 保存基于 STM32 HAL 和 CubeMX 句柄的默认实现。
- `test/` 保存 BSP 验证程序，不属于正式器件驱动。

平台特有实现放在：

```text
platform/<平台名>/overrides/
```
