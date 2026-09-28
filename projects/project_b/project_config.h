#ifndef PROJECT_B_PROJECT_CONFIG_H
#define PROJECT_B_PROJECT_CONFIG_H

#include "bsp_spi.h"
#include "app_project_b.h"

#define PROJECT_NAME                  "project_b"
#define APP_INTERFACE_GET_FN          app_project_b_get_interface
#define PROJECT_ENABLE_DEBUG_UART     1
#define PROJECT_ENABLE_STATUS_LED     1
#define PROJECT_SHARED_SPI_ID         BSP_SPI_ID_1
#define PROJECT_SHARED_SPI_PRESCALER  BSP_SPI_PRESCALER_64

#endif
