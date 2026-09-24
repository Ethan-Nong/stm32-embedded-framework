# 平台覆盖

这里只放当前硬件平台对外设 HAL 的差异化适配。

优先复用 `shared/bsp/port/` 中的公共实现；只有当前平台需要覆盖 GPIO、
UART、SPI 等实现时，才在这里增加 `override_*_port.c`。
