#include "bsp_gpio.h"
#include "bsp_port_hal.h"

#ifdef HAL_GPIO_MODULE_ENABLED

/* 将 BSP 端口编号映射到当前芯片实际 GPIO 寄存器基地址。 */
static GPIO_TypeDef *bsp_gpio_resolve_port(bsp_gpio_port_t port)
{
    switch (port)
    {
#ifdef GPIOA
    case BSP_GPIO_PORT_A:
        return GPIOA;
#endif
#ifdef GPIOB
    case BSP_GPIO_PORT_B:
        return GPIOB;
#endif
#ifdef GPIOC
    case BSP_GPIO_PORT_C:
        return GPIOC;
#endif
#ifdef GPIOD
    case BSP_GPIO_PORT_D:
        return GPIOD;
#endif
#ifdef GPIOE
    case BSP_GPIO_PORT_E:
        return GPIOE;
#endif
#ifdef GPIOF
    case BSP_GPIO_PORT_F:
        return GPIOF;
#endif
#ifdef GPIOG
    case BSP_GPIO_PORT_G:
        return GPIOG;
#endif
#ifdef GPIOH
    case BSP_GPIO_PORT_H:
        return GPIOH;
#endif
    default:
        return 0;
    }
}

/* 配置 GPIO 前确保对应端口时钟已经开启。 */
static void bsp_gpio_enable_port_clock(bsp_gpio_port_t port)
{
    switch (port)
    {
#ifdef GPIOA
    case BSP_GPIO_PORT_A:
        __HAL_RCC_GPIOA_CLK_ENABLE();
        break;
#endif
#ifdef GPIOB
    case BSP_GPIO_PORT_B:
        __HAL_RCC_GPIOB_CLK_ENABLE();
        break;
#endif
#ifdef GPIOC
    case BSP_GPIO_PORT_C:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
#endif
#ifdef GPIOD
    case BSP_GPIO_PORT_D:
        __HAL_RCC_GPIOD_CLK_ENABLE();
        break;
#endif
#ifdef GPIOE
    case BSP_GPIO_PORT_E:
        __HAL_RCC_GPIOE_CLK_ENABLE();
        break;
#endif
#ifdef GPIOF
    case BSP_GPIO_PORT_F:
        __HAL_RCC_GPIOF_CLK_ENABLE();
        break;
#endif
#ifdef GPIOG
    case BSP_GPIO_PORT_G:
        __HAL_RCC_GPIOG_CLK_ENABLE();
        break;
#endif
#ifdef GPIOH
    case BSP_GPIO_PORT_H:
        __HAL_RCC_GPIOH_CLK_ENABLE();
        break;
#endif
    default:
        break;
    }
}

/* 将 BSP GPIO 模式转换为 STM32 HAL 的 GPIO_MODE_x。 */
static int bsp_gpio_resolve_mode(bsp_gpio_mode_t mode, uint32_t *hal_mode)
{
    if (hal_mode == 0)
    {
        return -1;
    }

    switch (mode)
    {
    case BSP_GPIO_MODE_INPUT:
        *hal_mode = GPIO_MODE_INPUT;
        return 0;
    case BSP_GPIO_MODE_OUTPUT_PUSH_PULL:
        *hal_mode = GPIO_MODE_OUTPUT_PP;
        return 0;
    case BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN:
        *hal_mode = GPIO_MODE_OUTPUT_OD;
        return 0;
    case BSP_GPIO_MODE_ANALOG:
        *hal_mode = GPIO_MODE_ANALOG;
        return 0;
    default:
        return -1;
    }
}

/* 将 BSP 上下拉配置转换为 STM32 HAL 的 GPIO_PULLx。 */
static int bsp_gpio_resolve_pull(bsp_gpio_pull_t pull, uint32_t *hal_pull)
{
    if (hal_pull == 0)
    {
        return -1;
    }

    switch (pull)
    {
    case BSP_GPIO_PULL_NONE:
        *hal_pull = GPIO_NOPULL;
        return 0;
    case BSP_GPIO_PULL_UP:
        *hal_pull = GPIO_PULLUP;
        return 0;
    case BSP_GPIO_PULL_DOWN:
        *hal_pull = GPIO_PULLDOWN;
        return 0;
    default:
        return -1;
    }
}

/* 将 BSP 速度配置转换为 STM32 HAL 的 GPIO_SPEED_FREQ_x。 */
static int bsp_gpio_resolve_speed(bsp_gpio_speed_t speed, uint32_t *hal_speed)
{
    if (hal_speed == 0)
    {
        return -1;
    }

    switch (speed)
    {
    case BSP_GPIO_SPEED_LOW:
        *hal_speed = GPIO_SPEED_FREQ_LOW;
        return 0;
    case BSP_GPIO_SPEED_MEDIUM:
        *hal_speed = GPIO_SPEED_FREQ_MEDIUM;
        return 0;
    case BSP_GPIO_SPEED_HIGH:
        *hal_speed = GPIO_SPEED_FREQ_HIGH;
        return 0;
    default:
        return -1;
    }
}

/* 写 GPIO 输出电平；端口无效时保持静默返回。 */
void bsp_gpio_write(const bsp_gpio_t *gpio, bsp_gpio_level_t level)
{
    GPIO_TypeDef *port;

    if (gpio == 0)
    {
        return;
    }

    port = bsp_gpio_resolve_port(gpio->port);
    if (port == 0)
    {
        return;
    }

    HAL_GPIO_WritePin(port, gpio->pin, (level == BSP_GPIO_LEVEL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* 读取 GPIO 当前电平；无效参数统一返回低电平。 */
bsp_gpio_level_t bsp_gpio_read(const bsp_gpio_t *gpio)
{
    GPIO_TypeDef *port;

    if (gpio == 0)
    {
        return BSP_GPIO_LEVEL_LOW;
    }

    port = bsp_gpio_resolve_port(gpio->port);
    if (port == 0)
    {
        return BSP_GPIO_LEVEL_LOW;
    }

    return (HAL_GPIO_ReadPin(port, gpio->pin) == GPIO_PIN_SET) ? BSP_GPIO_LEVEL_HIGH : BSP_GPIO_LEVEL_LOW;
}

/* 翻转 GPIO 输出电平；端口无效时保持静默返回。 */
void bsp_gpio_toggle(const bsp_gpio_t *gpio)
{
    GPIO_TypeDef *port;

    if (gpio == 0)
    {
        return;
    }

    port = bsp_gpio_resolve_port(gpio->port);
    if (port == 0)
    {
        return;
    }

    HAL_GPIO_TogglePin(port, gpio->pin);
}

/* 使用 BSP 通用配置结构初始化 GPIO，避免上层直接依赖 GPIO_InitTypeDef。 */
void bsp_gpio_config(const bsp_gpio_t *gpio, const bsp_gpio_config_t *config)
{
    GPIO_TypeDef *port;
    GPIO_InitTypeDef init = { 0 };

    if ((gpio == 0) || (config == 0))
    {
        return;
    }

    port = bsp_gpio_resolve_port(gpio->port);
    if (port == 0)
    {
        return;
    }

    if ((bsp_gpio_resolve_mode(config->mode, &init.Mode) != 0) ||
        (bsp_gpio_resolve_pull(config->pull, &init.Pull) != 0) ||
        (bsp_gpio_resolve_speed(config->speed, &init.Speed) != 0))
    {
        return;
    }

    bsp_gpio_enable_port_clock(gpio->port);
    init.Pin = gpio->pin;
    HAL_GPIO_Init(port, &init);
}

void bsp_gpio_config_input(const bsp_gpio_t *gpio)
{
    const bsp_gpio_config_t config = {
        BSP_GPIO_MODE_INPUT,
        BSP_GPIO_PULL_NONE,
        BSP_GPIO_SPEED_LOW
    };

    bsp_gpio_config(gpio, &config);
}

#endif
