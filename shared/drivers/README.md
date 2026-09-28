# 驱动

这里放可跨项目复用的器件驱动。驱动器只能依赖 platform BSP 和
`shared/common`，不能直接依赖 CubeMX 句柄或具体项目。

板级接口、引脚复用和 SPI/I2C/UART 资源由 `shared/board` 管理。
器件驱动只处理具体器件的命令和协议，不复制总线资源管理逻辑。
