#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

typedef struct
{
    const char *name;
    int (*init)(void);
    void (*loop)(void);
} app_interface_t;

#endif
