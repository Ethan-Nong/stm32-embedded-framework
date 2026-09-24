#ifndef BASEOS_BSP_PWM_H
#define BASEOS_BSP_PWM_H

#include <stdint.h>

/* 板级和驱动层使用的逻辑 PWM 定时器编号。 */
typedef enum
{
    BSP_PWM_TIMER_ID_1 = 1,
    BSP_PWM_TIMER_ID_2,
    BSP_PWM_TIMER_ID_3,
    BSP_PWM_TIMER_ID_4,
    BSP_PWM_TIMER_ID_5,
    BSP_PWM_TIMER_ID_6,
    BSP_PWM_TIMER_ID_7,
    BSP_PWM_TIMER_ID_8,
    BSP_PWM_TIMER_ID_9,
    BSP_PWM_TIMER_ID_10,
    BSP_PWM_TIMER_ID_11,
    BSP_PWM_TIMER_ID_12,
    BSP_PWM_TIMER_ID_13,
    BSP_PWM_TIMER_ID_14,
    BSP_PWM_TIMER_ID_15,
    BSP_PWM_TIMER_ID_16,
    BSP_PWM_TIMER_ID_17
} bsp_pwm_timer_id_t;

/* PWM 输出通道编号，与 TIM_CHANNEL_1~4 对应。 */
typedef enum
{
    BSP_PWM_CHANNEL_1 = 1,
    BSP_PWM_CHANNEL_2,
    BSP_PWM_CHANNEL_3,
    BSP_PWM_CHANNEL_4
} bsp_pwm_channel_t;

/* 脱离具体 target 的抽象 PWM 句柄。 */
typedef struct
{
    bsp_pwm_timer_id_t timer_id;
    bsp_pwm_channel_t channel;
} bsp_pwm_t;

/* 启动 PWM 输出。 */
int bsp_pwm_start(const bsp_pwm_t *pwm);
/* 停止 PWM 输出。 */
int bsp_pwm_stop(const bsp_pwm_t *pwm);
/* 设置占空比，单位为千分比，超过 1000 时按 1000 处理。 */
int bsp_pwm_set_duty(const bsp_pwm_t *pwm, uint16_t duty_permille);
/* 设置 PWM 频率和占空比，保持当前定时器预分频配置不变。 */
int bsp_pwm_set_frequency(const bsp_pwm_t *pwm, uint32_t frequency_hz, uint16_t duty_permille);
/* 直接设置比较值，常用于计数频率为 1 MHz 的舵机脉宽场景。 */
int bsp_pwm_set_pulse_us(const bsp_pwm_t *pwm, uint16_t pulse_us);

#endif
