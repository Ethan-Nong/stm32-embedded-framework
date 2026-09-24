# common

`common` 是通用基础工具层。

职责：

- 公共宏
- 通用类型
- 错误码
- 通用小工具函数

不应放在这里的内容：

- 业务逻辑
- 模块驱动逻辑
- HAL / CubeMX 绑定实现
- 板级资源映射

当前公共头文件位于：

```text
include/
```

`common` 只保留真正与 HAL、CubeMX 和具体平台无关的内容。
