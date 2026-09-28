# 板级抽象

这里保存所有平台共用的 board 实现。

每个平台通过 `board_config.h` 提供：

- 调试串口和通信串口对应的逻辑角色
- SPI 总线、引脚、AF 和片选
- I2C、UART 的总线、引脚和 AF
- GPIO、ADC 和定时器角色
- 状态灯和按键映射

`board.c` 读取这些宏，并向驱动和应用提供稳定的配置访问接口。

每块新板卡只增加 `board_config.h`，不需要复制 `board.c` 和 `board.h`。

## 接口资源和引脚复用

Board 使用逻辑接口描述 SPI、I2C、UART 和 GPIO。每个逻辑接口可以声明
多个模式，每个模式有独立的引脚表。例如同一个接口可以支持：

```text
GPIO
UART
SPI
I2C
```

应用操作流程：

```c
if (board_interface_acquire(BOARD_INTERFACE_ID_SENSOR_I2C,
                            BOARD_INTERFACE_MODE_I2C) == 0)
{
    const board_i2c_cfg_t *i2c = board_get_sensor_i2c_cfg();
    /* 使用 I2C 外设 */
}

board_interface_release(BOARD_INTERFACE_ID_SENSOR_I2C);
```

如果接口没有占用，可以切换模式：

```c
board_interface_set_mode(BOARD_INTERFACE_ID_SENSOR_I2C,
                         BOARD_INTERFACE_MODE_GPIO);
```

切换后，接口会通过 BSP 重新配置引脚和 AF。调用方必须确保外设、DMA
和中断已经停止。需要恢复外设通信时，再调用平台提供的外设初始化函数。

接口被 `acquire` 后，其他已经占用的接口不能使用相同引脚。这样可以在
运行期检测 SPI、I2C、UART 或 GPIO 之间的引脚冲突。

## 边界

- Board 管逻辑接口、物理引脚、AF、模式、占用和冲突。
- BSP 管 GPIO、UART、SPI、I2C 的底层收发。
- Driver 管具体器件的寄存器、命令和协议。
- 通用芯片驱动不要放进 Board。
