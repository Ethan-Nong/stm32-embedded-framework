# STM32 嵌入式软件框架

这是一个面向多板卡、多项目的 STM32 嵌入式软件框架。

框架的核心原则是：

```text
一块物理板卡只保留一个 CubeMX 工程
一个应用功能只保留一套业务代码
一个固件构建组合只保留一份项目配置
可复用代码统一放在 shared
```

## 1. 核心概念

```text
app        统一入口和具体应用
platform   硬件平台、CubeMX、板级资源和平台特殊适配
projects   固件构建组合和功能配置
shared     可复用 board、BSP、驱动、模块、组件和公共代码
```

典型关系如下：

```text
projects/project_a
        |
        v
app/project_a
        |
        v
shared/modules + shared/drivers
        |
        v
shared/board + shared/bsp
        |
        v
platform/stm32f407_devboard
        |
        v
CubeMX + STM32 HAL
```

## 2. 完整目录结构

```text
stm32-embedded-framework/
├── app/
│   ├── main/
│   │   ├── app_interface.h
│   │   ├── app_main.c
│   │   └── app_main.h
│   ├── project_a/
│   │   ├── app_project_a.c
│   │   └── app_project_a.h
│   └── project_b/
│       ├── app_project_b.c
│       └── app_project_b.h
├── platform/
│   └── stm32f407_devboard/
│       ├── cube/
│       │   ├── Core/
│       │   ├── Drivers/
│       │   ├── MDK-ARM/
│       │   ├── .mxproject
│       │   └── stm32f407_devboard.ioc
│       ├── overrides/
│       ├── platform_config.h
│       ├── board_config.h
│       └── README.md
├── projects/
│   ├── project_a/
│   │   ├── project.json
│   │   ├── project_config.h
│   │   └── README.md
│   └── project_b/
│       ├── project.json
│       ├── project_config.h
│       └── README.md
├── shared/
│   ├── board/
│   │   ├── board.c
│   │   ├── board.h
│   │   └── README.md
│   ├── bsp/
│   │   ├── include/
│   │   ├── port/
│   │   ├── test/
│   │   └── README.md
│   ├── common/
│   │   ├── include/
│   │   ├── src/
│   │   └── README.md
│   ├── drivers/
│   │   ├── ds211/
│   │   └── README.md
│   ├── modules/
│   │   ├── control/
│   │   ├── logger/
│   │   ├── protocol/
│   │   ├── storage/
│   │   └── README.md
│   └── components/
│       ├── freertos/
│       ├── lvgl/
│       ├── lwip/
│       ├── mbedtls/
│       └── README.md
├── scripts/
│   ├── sync_keil.py
│   ├── new_project.py
│   ├── validate.py
│   └── build_keil.ps1
├── tools/
├── tests/
├── docs/
├── build/
├── CHANGELOG.md
├── .gitignore
└── README.md
```

## 3. 每个目录说明

| 目录 | 作用 |
|---|---|
| `app/main/` | 整个框架唯一的 `app_main.c/app_main.h`，连接 CubeMX 的 `main.c` 和项目应用。 |
| `app/<应用名>/` | 具体应用实现，例如项目 A、项目 B、电机控制或传感器网关。 |
| `platform/` | 所有物理板卡平台。每块板卡一个目录，一个平台只保存一个 CubeMX 工程。 |
| `platform/<平台名>/cube/` | CubeMX 生成的 Core、HAL、CMSIS、启动文件和 Keil 工作区。 |
| `platform/<平台名>/overrides/` | 平台特有的 BSP 覆盖实现。通常为空，只有公共 BSP 不适用时才添加。 |
| `platform/<平台名>/platform_config.h` | 平台名称、MCU 型号和硬件能力宏。 |
| `platform/<平台名>/board_config.h` | GPIO、UART、SPI、I2C、CS 和逻辑资源到物理资源的映射。 |
| `projects/` | 固件构建组合。每个项目只保存配置，不保存重复的 `app_main.c`。 |
| `projects/<项目名>/project.json` | 项目清单，声明使用的 platform、app、defines、modules、drivers 和 components。 |
| `projects/<项目名>/project_config.h` | 项目功能开关，并指定实际应用的入口头文件和初始化、循环函数。 |
| `shared/board/` | 所有平台共用的 board 实现，读取各平台的 `board_config.h`。 |
| `shared/bsp/include/` | 统一 BSP 接口，例如 GPIO、UART、SPI、I2C、ADC 和 Timer。 |
| `shared/bsp/port/` | 基于 STM32 HAL 和 CubeMX 默认句柄的公共 BSP 实现。 |
| `shared/bsp/test/` | BSP 基础验证示例，不属于正式设备驱动。 |
| `shared/common/` | 与具体 MCU、HAL 和项目无关的公共工具、类型和宏。 |
| `shared/drivers/` | 可跨项目、跨板卡复用的器件驱动，例如 Flash、屏幕和传感器。 |
| `shared/modules/` | 可复用的业务模块，例如协议、日志、控制算法和存储管理。 |
| `shared/components/` | FreeRTOS、LVGL、lwIP、mbedTLS 等第三方组件。 |
| `scripts/` | 项目创建、Keil 工程同步、结构校验和编译脚本。 |
| `tools/` | 运行在电脑上的串口监控、协议测试和数据分析工具。 |
| `tests/` | PC 端测试和硬件集成测试。 |
| `docs/` | 架构与开发说明。 |
| `build/` | 本地构建日志和临时输出，不提交到 Git。 |

## 4. 项目清单

`projects/project_a/project.json` 示例：

```json
{
  "name": "project_a",
  "platform": "stm32f407_devboard",
  "app": "project_a",
  "defines": [
    "PROJECT_A=1"
  ],
  "modules": [],
  "drivers": [],
  "components": []
}
```

字段说明：

| 字段 | 说明 |
|---|---|
| `name` | 项目名称，必须与 `projects/<name>` 目录一致。 |
| `platform` | 使用的硬件平台，对应 `platform/<platform>`。 |
| `app` | 使用的应用，对应 `app/<app>`。 |
| `defines` | 传入 Keil 的编译宏。 |
| `modules` | 使用的共享业务模块，对应 `shared/modules/<name>`。 |
| `drivers` | 使用的共享驱动，对应 `shared/drivers/<name>`。 |
| `components` | 使用的第三方组件，对应 `shared/components/<name>`。 |

`sync_keil.py` 会自动收集这些目录的 `src/**/*.c` 和 `include` 路径。

## 5. 应用入口机制

`app/main/app_main.c` 是整个仓库唯一的公共入口。它负责初始化 Board，
获取项目选择的应用接口，并统一调用应用的 `init` 和 `loop`：

```c
void app_main_init(void)
{
    board_init();
    g_app = APP_INTERFACE_GET_FN();
    g_app_init_status = g_app->init();
}

void app_main_loop(void)
{
    if (g_app_init_status == 0)
    {
        g_app->loop();
    }
}
```

每个应用导出统一接口：

```c
static const app_interface_t g_project_a_app = {
    "project_a",
    app_project_a_init,
    app_project_a_loop
};

const app_interface_t *app_project_a_get_interface(void)
{
    return &g_project_a_app;
}
```

项目配置只选择应用接口：

```c
#define APP_INTERFACE_GET_FN  app_project_a_get_interface
```

因此新增项目和驱动时不需要修改 `app_main.c`。驱动由具体应用初始化，
`app_main` 不直接调用器件驱动。

## 6. 环境要求

- Python 3
- Keil MDK-ARM
- Git
- STM32CubeMX，仅修改硬件初始化时需要

当前脚本默认 Keil 路径：

```text
D:\Keil_v5\UV4\UV4.exe
```

如果路径不同，在 [scripts/build_keil.ps1](scripts/build_keil.ps1) 中修改参数。

## 7. 生成 Keil 工程

同步项目 A：

```powershell
python scripts/sync_keil.py project_a
```

同步项目 B：

```powershell
python scripts/sync_keil.py project_b
```

生成的 Keil 工程位于：

```text
platform/stm32f407_devboard/cube/MDK-ARM/project_a.uvprojx
platform/stm32f407_devboard/cube/MDK-ARM/project_b.uvprojx
```

## 8. 编译项目

```powershell
powershell -File scripts/build_keil.ps1 -Project project_a
powershell -File scripts/build_keil.ps1 -Project project_b
```

脚本会先同步 Keil 工程，再调用 UV4 编译，并在 `build/keil/` 中保存日志。

## 9. 创建新项目

例如创建 `sensor_gateway`：

```powershell
python scripts/new_project.py `
  --name sensor_gateway `
  --platform stm32f407_devboard
```

脚本会创建：

```text
app/sensor_gateway/
projects/sensor_gateway/
```

然后执行：

```powershell
python scripts/sync_keil.py sensor_gateway
powershell -File scripts/build_keil.ps1 -Project sensor_gateway
```

## 10. 新增共享模块或驱动

业务模块：

```text
shared/modules/<模块名>/
├── include/
├── src/
└── README.md
```

器件驱动：

```text
shared/drivers/<驱动名>/
├── include/
├── src/
└── README.md
```

在项目清单中启用：

```json
{
  "modules": ["logger", "protocol"],
  "drivers": ["w25q256", "ili9341"]
}
```

同步脚本会自动加入对应的源文件和头文件目录。

## 11. 新增板卡平台

每块物理板卡创建一个独立平台：

```text
platform/<平台名>/
├── cube/
├── overrides/
├── platform_config.h
└── board_config.h
```

步骤：

1. 在该平台的 `cube/` 中创建并生成 CubeMX 工程。
2. 修改 `platform_config.h`，填写 MCU 型号和硬件能力。
3. 修改 `board_config.h`，填写 GPIO、外设和片选映射。
4. 只有公共 BSP 不适用时，才在 `overrides/` 中添加覆盖实现。
5. 创建项目清单并指向该平台。

CubeMX 工程不需要复制到每个项目中。

## 12. Board 接口与 SPI 总线共享

Board 是板级接口资源的唯一入口：

- `board_config.h` 描述逻辑接口、引脚、AF 和可用模式。
- `board.c` 负责接口申请、释放、引脚冲突检测和模式切换。
- BSP 负责 GPIO、UART、SPI 和 I2C 的底层收发。
- 器件驱动只实现具体器件协议。

应用通过接口 ID 和模式申请资源：

```c
if (board_interface_acquire(BOARD_INTERFACE_ID_SENSOR_I2C,
                            BOARD_INTERFACE_MODE_I2C) == 0)
{
    const board_i2c_cfg_t *i2c = board_get_sensor_i2c_cfg();
}
```

接口释放前，不能切换到其他模式。某个模式被占用时，其他接口不能使用
相同引脚。

多个器件可以共享同一条 SPI 总线，但每个器件必须有独立 CS：

```text
SPI1 SCK/MISO/MOSI
├── 器件 A，独立 CS_A
└── 器件 B，独立 CS_B
```

标准事务流程：

```text
获取总线锁
设置器件 SPI 参数
拉低 CS
读取或发送数据
拉高 CS
释放总线锁
```

项目代码不要直接使用 `hspi1`，统一通过 `bsp_spi.h` 访问。

## 13. 结构检查

```powershell
python scripts/validate.py
```

检查内容：

- 核心目录是否存在
- 平台必需文件是否齐全
- 项目清单是否合法
- 项目引用的 platform 和 app 是否存在

## 14. Git 提交与推送

查看修改：

```powershell
git status
git diff
```

提交本次 Markdown 和代码修改：

```powershell
git add .
git commit -m "docs: 更新中文说明并完善使用文档"
git push
```

查看提交历史：

```powershell
git log --oneline -5
```

## 15. 开发规则

- 一个物理板卡版本对应一个 `platform`。
- 一个固件组合对应一个 `projects/<name>`。
- 一个应用功能对应一个 `app/<name>`。
- 通用代码必须放入 `shared`。
- SPI、I2C、UART 的板级引脚、AF、模式和复用统一由 `board` 管理。
- 器件驱动不能直接操作 GPIO AF 或 CubeMX 句柄。
- 不修改 CubeMX 生成文件中的非 USER CODE 区域。
- 项目代码不直接依赖 HAL 句柄。
- 并发访问共享 SPI 总线时必须使用总线锁。
