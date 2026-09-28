#ifndef CORE_F407_SEED_DEV_PROJECT_CONFIG_H
#define CORE_F407_SEED_DEV_PROJECT_CONFIG_H

#include "bsp_uart.h"
#include "app_ds211_demo.h"

#define PROJECT_NAME                  "core_f407_seed_dev"
#define APP_INTERFACE_GET_FN          app_ds211_demo_get_interface
#define PROJECT_ENABLE_DEBUG_UART     1
#define PROJECT_ENABLE_STATUS_LED     0
#define PROJECT_FINGERPRINT_UART_ID   BSP_UART_ID_2
#define PROJECT_DS211_BAUD_RATE       115200U

#endif
