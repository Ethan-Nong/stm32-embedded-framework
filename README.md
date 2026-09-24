# STM32 Embedded Framework

一个硬件平台对应一个 CubeMX 工程，多个项目复用同一个平台。

## 核心目录

```text
app/        统一入口和具体应用
platform/   硬件平台、CubeMX 和板级配置
projects/   构建组合、功能配置和 Keil 工程描述
shared/     board、BSP、驱动、模块、组件和公共代码
```

完整结构：

```text
stm32-embedded-framework/
├── app/
│   ├── main/
│   │   ├── app_main.c
│   │   └── app_main.h
│   ├── project_a/
│   │   ├── app_project_a.c
│   │   └── app_project_a.h
│   └── project_b/
├── platform/
│   └── stm32f407_devboard/
│       ├── cube/
│       ├── overrides/
│       ├── platform_config.h
│       └── board_config.h
├── projects/
│   ├── project_a/
│   │   ├── project.json
│   │   ├── project_config.h
│   │   └── README.md
│   └── project_b/
├── shared/
│   ├── board/
│   ├── bsp/
│   │   ├── include/
│   │   ├── port/
│   │   └── test/
│   ├── common/
│   ├── drivers/
│   ├── modules/
│   └── components/
├── scripts/
├── tools/
├── tests/
└── docs/
```

## 使用方式

生成项目 A 的 Keil 工程：

```powershell
python scripts/sync_keil.py project_a
```

生成项目 B 的 Keil 工程：

```powershell
python scripts/sync_keil.py project_b
```

编译项目：

```powershell
powershell -File scripts/build_keil.ps1 -Project project_a
```

创建新项目：

```powershell
python scripts/new_project.py `
  --name project_c `
  --platform stm32f407_devboard
```

检查结构：

```powershell
python scripts/validate.py
```

## 复用规则

- 同一块板子和同一套引脚配置，只创建一个 `platform`。
- 不同构建组合只新增 `projects/<name>`，应用实现放在 `app/<name>`。
- SPI1 等总线由 `platform` 初始化一次，项目通过 `board_config.h` 和 CS 复用。
- 项目代码不能直接使用 `hspi1`，统一通过 `bsp_spi.h` 访问。
- 多任务同时访问 SPI 总线时，使用 `bsp_spi_lock()` 和 `bsp_spi_unlock()`。
