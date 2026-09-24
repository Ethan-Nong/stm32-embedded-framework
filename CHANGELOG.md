# Changelog

## 0.4.0

- Added a single shared entry at `app/main/app_main.c`.
- Moved application implementations to `app/<name>`.
- Reduced each project to `project.json`, `project_config.h` and README.
- Updated the build generator to select app, module, driver and component
  sources from the project manifest.
- Verified project creation, synchronization and full Keil rebuild.

## 0.3.0

- Moved board implementation and default BSP to `shared`.
- Reduced each platform to CubeMX, `platform_config.h`, `board_config.h`
  and optional overrides.
- Added one common `shared/board/board.c`.
- Rebuilt both example projects successfully.

## 0.2.0

- Simplified the repository to `platform`, `projects` and `shared`.
- Removed the legacy multi-level repository nesting.
- Replaced target and board arguments with a single project manifest.
- Reduced Keil synchronization to `python scripts/sync_keil.py <project>`.

## 0.1.0

- Created the platform/project separated repository structure.
- Added the STM32F407 CubeMX, HAL, board and BSP platform.
- Added `project_a` and `project_b` sharing one SPI1 target configuration.
- Added project-aware Keil synchronization.
- Added SPI bus locking and an SPI device driver template.
- Added project generation and repository validation scripts.
