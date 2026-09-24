# Architecture

```text
projects
   |
   v
app modules
   |
   v
shared modules and drivers
   |
   v
shared BSP and board
   |
   v
platform overrides and board_config
   |
   v
CubeMX and STM32 HAL
```

## platform

`platform/<name>` describes one physical board revision:

- `cube/` contains the only CubeMX project.
- `platform_config.h` identifies the MCU and hardware capabilities.
- `board_config.h` maps logical devices to pins and BSP resources.
- `overrides/` contains platform-specific BSP replacements when needed.

## projects

Every project owns:

- `project.json` for platform selection and build defines.
- `project_config.h` for project features.

The generated Keil project references the shared platform and can be rebuilt
without duplicating the CubeMX workspace.

## app

- `main/` contains the only `app_main.c/h`.
- `<name>/` contains the selected application implementation.
- The project supplies `APP_HEADER`, `APP_INIT_FN` and `APP_LOOP_FN`.

## shared

- `board/` contains the common board accessor implementation.
- `bsp/` contains portable APIs and default STM32 HAL implementations.
- `common/` contains target-independent utilities.
- `drivers/` contains reusable device drivers.
- `modules/` contains reusable business modules.
- `components/` contains FreeRTOS, LVGL, lwIP and similar libraries.

## SPI sharing

Multiple devices can use one SPI bus when each device has its own CS:

```text
lock bus -> select CS -> transfer -> deselect CS -> unlock bus
```

The platform owns pins and bus parameters. Projects select devices through
the board layer.
