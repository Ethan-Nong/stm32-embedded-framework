# STM32F407 开发板平台

该目录描述一块 STM32F407 开发板版本的硬件配置。

```text
cube/                 唯一的 CubeMX 和 Keil 工作区
overrides/            平台特有的 BSP 覆盖实现
platform_config.h     MCU 型号和硬件能力
board_config.h        GPIO、外设和逻辑资源映射
```

使用相同引脚和外设配置的项目都引用这个平台，不需要复制 CubeMX 工程。

只有在不同项目需要不同引脚、时钟或外设初始化时，才创建新的平台目录。
