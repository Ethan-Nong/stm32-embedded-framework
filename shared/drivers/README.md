# drivers

这里放可跨项目复用的器件驱动。驱动器只能依赖 platform BSP 和
`shared/common`，不能直接依赖 CubeMX 句柄或具体项目。
