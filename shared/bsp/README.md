# BSP

`include/` contains portable interfaces such as GPIO, UART and SPI.

`port/` contains the default STM32 HAL + CubeMX handle implementations.

Platform-specific replacements belong in `platform/<name>/overrides/`.

`test/` contains BSP validation demos and is not part of a production project
unless explicitly selected.
