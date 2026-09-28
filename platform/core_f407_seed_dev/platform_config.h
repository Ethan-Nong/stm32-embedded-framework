#ifndef PLATFORM_CORE_F407_SEED_DEV_CONFIG_H
#define PLATFORM_CORE_F407_SEED_DEV_CONFIG_H

#define PLATFORM_NAME                 "core_f407_seed_dev"
#define PLATFORM_MCU                  "STM32F407ZGT6"
#define PLATFORM_SOC_FAMILY           "STM32F407"

/* Compatibility aliases used by existing framework code. */
#define TARGET_NAME                   PLATFORM_NAME
#define TARGET_SOC_STM32F407
#define TARGET_BOARD_CORE_F407_SEED_DEV

#define PLATFORM_HAS_GPIO             1
#define PLATFORM_HAS_UART1            1
#define PLATFORM_HAS_UART2            1
#define PLATFORM_HAS_I2C1             1
#define PLATFORM_HAS_SPI1             1

#define PLATFORM_HAS_ADC1             0
#define PLATFORM_HAS_TIM3             0
#define PLATFORM_HAS_ETHERNET         0
#define PLATFORM_HAS_USB              0
#define PLATFORM_HAS_SDIO             0

#endif
