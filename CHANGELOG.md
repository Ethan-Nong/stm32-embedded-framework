# 更新记录

## 0.6.0

- 增加公共 `app_interface_t`，统一应用生命周期接口。
- `app_main` 统一负责 Board 初始化和应用 `init/loop` 调度。
- 项目配置改为选择一个应用接口，不再直接展开应用初始化宏。
- DS211、project_a 和 project_b 统一接入 App 接口。

## 0.5.0

- 将 SPI、I2C、UART 的逻辑接口、引脚、AF 和可用模式统一纳入 Board。
- 增加 Board 接口申请、释放、模式切换和引脚冲突检测。
- 增加 GPIO 备用功能配置接口，支持运行期协议引脚复用。
- 移除未使用的 `spi_device_template`，驱动目录只保留真实器件驱动。

## 0.4.0

- 增加 `app/main/app_main.c` 作为整个框架的唯一公共入口。
- 将具体应用实现移动到 `app/<应用名>`。
- 将每个项目简化为 `project.json`、`project_config.h` 和说明文件。
- 更新 Keil 工程生成脚本，根据项目清单自动选择 app、模块、驱动和组件。
- 验证新建项目、工程同步和完整 Keil 重新编译流程。

## 0.3.0

- 将公共 board 实现和默认 BSP 移动到 `shared`。
- 将每个平台简化为 CubeMX、`platform_config.h`、`board_config.h` 和可选的 `overrides`。
- 增加唯一的公共 `shared/board/board.c`。
- 重新编译两个示例项目并通过。

## 0.2.0

- 将仓库简化为 `platform`、`projects` 和 `shared`。
- 删除旧的多层目录嵌套。
- 使用项目清单代替 target 和 board 命令行参数。
- 将 Keil 同步简化为 `python scripts/sync_keil.py <项目名>`。

## 0.1.0

- 建立平台与项目分离的仓库结构。
- 增加 STM32F407 的 CubeMX、HAL、board 和 BSP 平台。
- 增加共享同一个 SPI1 配置的 `project_a` 和 `project_b`。
- 增加项目感知的 Keil 工程同步功能。
- 增加 SPI 总线锁和 SPI 器件驱动模板。
- 增加项目创建和仓库结构校验脚本。
