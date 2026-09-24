#include "bsp_exti.h"
#include "bsp_port_hal.h"

#if defined(HAL_GPIO_MODULE_ENABLED) && defined(HAL_EXTI_MODULE_ENABLED)

/* 每条 EXTI line 保存一个 GPIO 描述和上层回调。 */
typedef struct
{
    bsp_gpio_t gpio;
    bsp_exti_callback_t callback;
    void *user_data;
} bsp_exti_entry_t;

static bsp_exti_entry_t g_exti_entries[16];

/* GPIO_PIN_x 是位掩码，这里转换成 EXTI line 的 0~15 索引。 */
static int bsp_exti_pin_index(uint16_t pin)
{
    int index;

    for (index = 0; index < 16; ++index)
    {
        if (pin == (uint16_t)(1U << index))
        {
            return index;
        }
    }

    return -1;
}

/* 注册指定 GPIO 对应 EXTI line 的上层回调。 */
int bsp_exti_register_callback(const bsp_gpio_t *gpio, bsp_exti_callback_t callback, void *user_data)
{
    int index;

    if ((gpio == 0) || (callback == 0))
    {
        return -1;
    }

    index = bsp_exti_pin_index(gpio->pin);
    if (index < 0)
    {
        return -1;
    }

    g_exti_entries[index].gpio = *gpio;
    g_exti_entries[index].callback = callback;
    g_exti_entries[index].user_data = user_data;
    return 0;
}

/* 注销指定 GPIO 对应 EXTI line 的回调。 */
int bsp_exti_unregister_callback(const bsp_gpio_t *gpio)
{
    int index;

    if (gpio == 0)
    {
        return -1;
    }

    index = bsp_exti_pin_index(gpio->pin);
    if (index < 0)
    {
        return -1;
    }

    g_exti_entries[index].callback = 0;
    g_exti_entries[index].user_data = 0;
    return 0;
}

/* 将 HAL 的 pin 回调分发到 BSP 注册的回调。 */
static void bsp_exti_dispatch(uint16_t pin)
{
    int index;

    index = bsp_exti_pin_index(pin);
    if ((index < 0) || (g_exti_entries[index].callback == 0))
    {
        return;
    }

    g_exti_entries[index].callback(&g_exti_entries[index].gpio, g_exti_entries[index].user_data);
}

/* 兼容不同 STM32 HAL 版本的通用 EXTI 回调。 */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    bsp_exti_dispatch(pin);
}

/* 兼容提供独立上升沿回调的 HAL 版本。 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
    bsp_exti_dispatch(pin);
}

/* 兼容提供独立下降沿回调的 HAL 版本。 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
    bsp_exti_dispatch(pin);
}

#endif
