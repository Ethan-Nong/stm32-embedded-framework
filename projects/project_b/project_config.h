#ifndef PROJECT_B_PROJECT_CONFIG_H
#define PROJECT_B_PROJECT_CONFIG_H

#include "bsp_spi.h"

#define PROJECT_NAME                  "project_b"
#define APP_HEADER                    "app_project_b.h"
#define APP_INIT_FN                   app_project_b_init
#define APP_LOOP_FN                   app_project_b_loop
#define PROJECT_ENABLE_DEBUG_UART     1
#define PROJECT_ENABLE_STATUS_LED     1
#define PROJECT_SHARED_SPI_ID         BSP_SPI_ID_1
#define PROJECT_SHARED_SPI_PRESCALER  BSP_SPI_PRESCALER_64

#endif
