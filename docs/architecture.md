# 软件架构

## 分层关系

```text
projects
   |
   v
app
   |
   v
shared/modules 和 shared/drivers
   |
   v
shared/board 和 shared/bsp
   |
   v
platform/overrides 和 platform/board_config.h
   |
   v
CubeMX 和 STM32 HAL
```

依赖只能从上向下。底层代码不能反向依赖具体项目。

## projects

每个项目保存一份固件构建组合：

- `project.json` 选择平台、应用、模块、驱动和组件。
- `project_config.h` 保存项目开关，并指定应用入口。

项目目录不保存自己的 `app_main.c`。

## app

- `main/` 保存唯一的 `app_main.c/h` 和公共 `app_interface.h`。
- `<应用名>/` 保存具体应用实现。
- 每个应用导出统一的 `app_interface_t`，包含 `init` 和 `loop`。

`app_main.c` 先初始化 Board，再根据项目配置获取应用接口并调用。驱动
由应用初始化和使用，`app_main.c` 不直接依赖具体驱动。

## platform

`platform/<平台名>` 描述一个物理板卡版本：

- `cube/` 保存唯一的 CubeMX 工作区。
- `platform_config.h` 描述 MCU 和硬件能力。
- `board_config.h` 描述引脚、总线和逻辑资源映射。
- `overrides/` 保存平台特有的 BSP 覆盖实现。

只有板卡引脚、时钟或外设初始化发生变化时，才创建新的平台。

## shared

- `board/` 保存所有平台共用的 board 实现、接口资源表和引脚复用管理。
- `bsp/` 保存统一接口和默认 STM32 HAL 实现。
- `common/` 保存与平台无关的工具和类型。
- `drivers/` 保存可复用的器件驱动。
- `modules/` 保存可复用的业务模块。
- `components/` 保存 FreeRTOS、LVGL、lwIP 等第三方组件。

## Board 接口资源

Board 是板级接口资源的唯一入口。`board_config.h` 描述逻辑接口、引脚、
AF 和可用模式，`board.c` 负责资源占用、冲突检测和模式切换。

每个逻辑接口可以声明一个或多个模式：

```text
BOARD_INTERFACE_ID_SENSOR_I2C
├── BOARD_INTERFACE_MODE_I2C
└── BOARD_INTERFACE_MODE_GPIO
```

应用使用 `board_interface_acquire` 选择模式。只要另一个已占用接口使用
相同引脚，申请就会失败。释放后可以切换到其他模式。

引脚 AF 的底层配置由 `shared/bsp` 完成，协议收发仍由 BSP 或器件驱动
完成。Board 不实现 SPI、I2C、UART 协议。

## SPI 总线共享

多个器件可以共享一条 SPI 总线，但每个器件必须有独立 CS：

```text
获取总线锁
设置器件参数
拉低 CS
传输数据
拉高 CS
释放总线锁
```

平台负责引脚和总线初始化，Board 负责接口模式与资源占用，驱动通过
`shared/board` 获取配置并执行器件协议。
