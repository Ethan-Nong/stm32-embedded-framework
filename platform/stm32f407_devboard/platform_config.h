#ifndef PLATFORM_STM32F407_DEVBOARD_CONFIG_H
#define PLATFORM_STM32F407_DEVBOARD_CONFIG_H

#define PLATFORM_NAME                 "stm32f407_devboard"
#define PLATFORM_MCU                  "STM32F407VET6"
#define PLATFORM_SOC_FAMILY           "STM32F407"

/* Compatibility aliases for existing application modules. */
#define TARGET_NAME                   PLATFORM_NAME
#define TARGET_SOC_STM32F407
#define TARGET_BOARD_STM32F407_DEVBOARD

#define PLATFORM_HAS_GPIO             1
#define PLATFORM_HAS_UART1            1
#define PLATFORM_HAS_UART2            1
#define PLATFORM_HAS_I2C1             1
#define PLATFORM_HAS_SPI1             1
#define PLATFORM_HAS_ADC1             1
#define PLATFORM_HAS_TIM3             1

#define PLATFORM_HAS_ETHERNET         0
#define PLATFORM_HAS_USB              0
#define PLATFORM_HAS_SDIO             0

#endif
