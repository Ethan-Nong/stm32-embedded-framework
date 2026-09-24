#ifndef BASEOS_COMMON_BSP_PORT_HAL_H
#define BASEOS_COMMON_BSP_PORT_HAL_H

#include <stdint.h>

#include "main.h"

/*
 * BSP port 用弱符号声明 CubeMX 句柄，使同一份端口代码可以跨 target 编译；
 * 使用前必须通过本宏确认当前 target 确实提供了对应句柄。
 */
#ifndef BSP_PORT_WEAK_HANDLE_DEFINED
#define BSP_PORT_WEAK_HANDLE_DEFINED(handle) ((uintptr_t)(&(handle)) != 0U)
#endif

#ifdef HAL_UART_MODULE_ENABLED
extern __WEAK UART_HandleTypeDef huart1;
extern __WEAK UART_HandleTypeDef huart2;
extern __WEAK UART_HandleTypeDef huart3;
extern __WEAK UART_HandleTypeDef huart4;
extern __WEAK UART_HandleTypeDef huart5;
extern __WEAK UART_HandleTypeDef huart6;
extern __WEAK UART_HandleTypeDef huart7;
extern __WEAK UART_HandleTypeDef huart8;
#endif

#ifdef HAL_TIM_MODULE_ENABLED
extern __WEAK TIM_HandleTypeDef htim1;
extern __WEAK TIM_HandleTypeDef htim2;
extern __WEAK TIM_HandleTypeDef htim3;
extern __WEAK TIM_HandleTypeDef htim4;
extern __WEAK TIM_HandleTypeDef htim5;
extern __WEAK TIM_HandleTypeDef htim6;
extern __WEAK TIM_HandleTypeDef htim7;
extern __WEAK TIM_HandleTypeDef htim8;
extern __WEAK TIM_HandleTypeDef htim9;
extern __WEAK TIM_HandleTypeDef htim10;
extern __WEAK TIM_HandleTypeDef htim11;
extern __WEAK TIM_HandleTypeDef htim12;
extern __WEAK TIM_HandleTypeDef htim13;
extern __WEAK TIM_HandleTypeDef htim14;
extern __WEAK TIM_HandleTypeDef htim15;
extern __WEAK TIM_HandleTypeDef htim16;
extern __WEAK TIM_HandleTypeDef htim17;
#endif

#ifdef HAL_SPI_MODULE_ENABLED
extern __WEAK SPI_HandleTypeDef hspi1;
extern __WEAK SPI_HandleTypeDef hspi2;
extern __WEAK SPI_HandleTypeDef hspi3;
#endif

#ifdef HAL_I2C_MODULE_ENABLED
extern __WEAK I2C_HandleTypeDef hi2c1;
extern __WEAK I2C_HandleTypeDef hi2c2;
extern __WEAK I2C_HandleTypeDef hi2c3;
#endif

#ifdef HAL_ADC_MODULE_ENABLED
extern __WEAK ADC_HandleTypeDef hadc1;
extern __WEAK ADC_HandleTypeDef hadc2;
extern __WEAK ADC_HandleTypeDef hadc3;
#endif

#ifdef HAL_SAI_MODULE_ENABLED
extern __WEAK SAI_HandleTypeDef hsai_BlockA1;
#endif

#endif
