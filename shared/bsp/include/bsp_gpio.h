#ifndef BASEOS_BSP_GPIO_H
#define BASEOS_BSP_GPIO_H

#include <stdint.h>

/* GPIO 端口编号，与芯片实际 GPIOA~GPIOH 对应。 */
typedef enum
{
    BSP_GPIO_PORT_A = 0,
    BSP_GPIO_PORT_B,
    BSP_GPIO_PORT_C,
    BSP_GPIO_PORT_D,
    BSP_GPIO_PORT_E,
    BSP_GPIO_PORT_F,
    BSP_GPIO_PORT_G,
    BSP_GPIO_PORT_H
} bsp_gpio_port_t;

/* GPIO 引脚电平定义。 */
typedef enum
{
    BSP_GPIO_LEVEL_LOW = 0,
    BSP_GPIO_LEVEL_HIGH = 1
} bsp_gpio_level_t;

/* GPIO 工作模式，BSP 层只描述通用语义，具体寄存器配置由 port 层适配。 */
typedef enum
{
    BSP_GPIO_MODE_INPUT = 0,
    BSP_GPIO_MODE_OUTPUT_PUSH_PULL,
    BSP_GPIO_MODE_OUTPUT_OPEN_DRAIN,
    BSP_GPIO_MODE_ANALOG
} bsp_gpio_mode_t;

/* GPIO 内部上下拉配置。 */
typedef enum
{
    BSP_GPIO_PULL_NONE = 0,
    BSP_GPIO_PULL_UP,
    BSP_GPIO_PULL_DOWN
} bsp_gpio_pull_t;

/* GPIO 输出速度配置，输入模式下通常由底层 HAL 忽略。 */
typedef enum
{
    BSP_GPIO_SPEED_LOW = 0,
    BSP_GPIO_SPEED_MEDIUM,
    BSP_GPIO_SPEED_HIGH
} bsp_gpio_speed_t;

/* GPIO 引脚描述，pin 使用 target HAL 的 GPIO_PIN_x 位掩码。 */
typedef struct
{
    bsp_gpio_port_t port;
    uint16_t pin;
} bsp_gpio_t;

/* GPIO 通用配置，调用方不直接依赖 GPIO_InitTypeDef。 */
typedef struct
{
    bsp_gpio_mode_t mode;
    bsp_gpio_pull_t pull;
    bsp_gpio_speed_t speed;
} bsp_gpio_config_t;

/* 输出指定电平；gpio 为空或端口无效时直接返回。 */
void bsp_gpio_write(const bsp_gpio_t *gpio, bsp_gpio_level_t level);
/* 读取输入/输出引脚当前电平；gpio 为空或端口无效时返回低电平。 */
bsp_gpio_level_t bsp_gpio_read(const bsp_gpio_t *gpio);
/* 翻转输出电平；gpio 为空或端口无效时直接返回。 */
void bsp_gpio_toggle(const bsp_gpio_t *gpio);
/* 按 mode/pull/speed 配置 GPIO；gpio 或 config 无效时直接返回。 */
void bsp_gpio_config(const bsp_gpio_t *gpio, const bsp_gpio_config_t *config);
void bsp_gpio_config_input(const bsp_gpio_t *gpio);

#endif
