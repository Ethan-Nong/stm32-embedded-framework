# board

Common board implementation shared by every platform.

Each platform provides `board_config.h` with:

- logical UART roles
- SPI buses and chip-select pins
- I2C, ADC and timer roles
- status LED and key mapping

`board.c` reads those macros and exposes stable getter functions to drivers
and application modules. A platform does not need its own copy of `board.c`.
