# 应用入口

`app_main.c` 是整个固件唯一的公共应用入口，负责：

1. 初始化公共 Board。
2. 获取项目选择的应用接口。
3. 调用应用的 `init`。
4. 在主循环中调用应用的 `loop`。

所有应用都使用 `app_interface.h` 定义的统一接口：

```c
typedef struct
{
    const char *name;
    int (*init)(void);
    void (*loop)(void);
} app_interface_t;
```

项目在 `project_config.h` 中选择应用：

```c
#define APP_INTERFACE_GET_FN  app_ds211_demo_get_interface
```

驱动不能由 `app_main` 直接初始化。具体应用负责申请 Board 资源、初始化
需要的驱动，并在自己的 `loop` 中调用驱动接口。
