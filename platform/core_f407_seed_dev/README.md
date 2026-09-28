# core_f407_seed_dev 平台

该平台基于原始的 `core_f407_seed_dev` CubeMX 工程，MCU 为
STM32F407ZGT6。

## 外设分配

| 功能 | 外设 | 引脚 |
|---|---|---|
| 电脑串口调试 | USART1 | PA9 TX、PA10 RX |
| DS211 指纹模块 | USART2 | PA2 TX、PA3 RX |
| DS211 Touch 输出 | 支持 EXTI 的 GPIO 输入 | PB0 |
| 板载 SPI | SPI1 | PA5 SCK、PA6 MISO、PA7 MOSI |
| 板载 I2C | I2C1 | PB6 SCL、PB7 SDA |

DS211 串口配置为 `115200 bps、8 数据位、无校验、2 停止位`，即
`115200 8N2`。

平台同时声明了公共 Board 接口注册表所需的引脚和备用功能。应用在
使用 UART、SPI 或 I2C 前，可以先申请对应资源：

```c
board_interface_acquire(BOARD_INTERFACE_ID_COMM_UART,
                        BOARD_INTERFACE_MODE_UART);
```
