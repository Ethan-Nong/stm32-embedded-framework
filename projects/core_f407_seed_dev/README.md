# core_f407_seed_dev DS211 示例

该项目运行在 `core_f407_seed_dev` 平台上。

生成并打开 Keil 工程：

```powershell
python scripts/sync_keil.py core_f407_seed_dev
```

```text
platform/core_f407_seed_dev/cube/MDK-ARM/core_f407_seed_dev.uvprojx
```

## 接线

| DS211 引脚/线色 | 信号 | STM32F407ZGT6 |
|---|---|---|
| 1 脚，红色 | Vsen | 独立常开 LDO 3.3V |
| 2 脚，黄色 | Touch | PB0，高电平输入 |
| 3 脚，棕色/深色 | VDD | 3.3V，低功耗时可通过开关控制 |
| 4 脚，绿色 | 模块 TX | PA3，USART2_RX |
| 5 脚，白色 | 模块 RX | PA2，USART2_TX |
| 6 脚，黑色 | GND | GND，与 STM32 共地 |

线色按照连接器引脚编号排列，不要按照照片中的视觉顺序判断。模块启动
期间，Vsen 和 VDD 都必须接入 3.3V。Vsen 必须由 LDO 供电，不能直接
使用 MCU 的 GPIO 供电。

USART1 连接电脑串口调试工具：

| USB-TTL 模块 | STM32F407ZGT6 |
|---|---|
| RX | PA9，USART1_TX |
| TX | PA10，USART1_RX |
| GND | GND |

USART1 使用 `115200 8N1`。该 DS211 模块的 USART2 使用
`115200 8N2`。

DS211 需要使用稳定的 3.3V 电源，供电能力至少 200mA。UART 引脚不能
接 5V，所有电源和信号必须共地。Touch 信号只有在模块按照手册完成低功耗
唤醒流程后才会有效。
