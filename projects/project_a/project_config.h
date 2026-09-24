#ifndef PROJECT_A_PROJECT_CONFIG_H
#define PROJECT_A_PROJECT_CONFIG_H

#include "bsp_spi.h"

#define PROJECT_NAME                  "project_a"
#define APP_HEADER                    "app_project_a.h"
#define APP_INIT_FN                   app_project_a_init
#define APP_LOOP_FN                   app_project_a_loop
#define PROJECT_ENABLE_DEBUG_UART     1
#define PROJECT_ENABLE_STATUS_LED     1
#define PROJECT_SHARED_SPI_ID         BSP_SPI_ID_1
#define PROJECT_SHARED_SPI_PRESCALER  BSP_SPI_PRESCALER_32

#endif
