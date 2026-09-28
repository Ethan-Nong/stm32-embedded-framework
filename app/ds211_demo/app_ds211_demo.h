#ifndef APP_DS211_DEMO_H
#define APP_DS211_DEMO_H

#include "app_interface.h"

int app_ds211_demo_init(void);
void app_ds211_demo_loop(void);
const app_interface_t *app_ds211_demo_get_interface(void);

#endif
