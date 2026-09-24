#ifndef BASEOS_BSP_EXTI_H
#define BASEOS_BSP_EXTI_H

#include "bsp_gpio.h"

/* 已注册 EXTI GPIO 触发时调用的回调。 */
typedef void (*bsp_exti_callback_t)(const bsp_gpio_t *gpio, void *user_data);

/* 为指定 GPIO 注册 EXTI 边沿触发回调。 */
int bsp_exti_register_callback(const bsp_gpio_t *gpio, bsp_exti_callback_t callback, void *user_data);
/* 注销已经注册的 EXTI 回调。 */
int bsp_exti_unregister_callback(const bsp_gpio_t *gpio);

#endif
