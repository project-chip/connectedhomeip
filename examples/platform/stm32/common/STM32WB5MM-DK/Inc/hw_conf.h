/**
 ******************************************************************************
 * @file    hw_conf.h
 * @author  MCD Application Team
 * @brief   Configuration of hardware interface
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONF_H
#define __HW_CONF_H

/******************************************************************************
 * Semaphores
 * THIS SHALL NO BE CHANGED AS THESE SEMAPHORES ARE USED AS WELL ON THE CM0+
 *****************************************************************************/
/**
 *  Index of the semaphore used by CPU2 to prevent the CPU1 to either write or erase data in flash
 *  The CPU1 shall not either write or erase in flash when this semaphore is taken by the CPU2
 *  When the CPU1 needs to either write or erase in flash, it shall first get the semaphore and release it just
 *  after writing a raw (64bits data) or erasing one sector.
 *  On v1.4.0 and older CPU2 wireless firmware, this semaphore is unused and CPU2 is using PES bit.
 *  By default, CPU2 is using the PES bit to protect its timing. The CPU1 may request the CPU2 to use the semaphore
 *  instead of the PES bit by sending the system command SHCI_C2_SetFlashActivityControl()
 */
#define CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID 7

/**
 *  Index of the semaphore used by CPU1 to prevent the CPU2 to either write or erase data in flash
 *  In order to protect its timing, the CPU1 may get this semaphore to prevent the  CPU2 to either
 *  write or erase in flash (as this will stall both CPUs)
 *  The PES bit shall not be used as this may stall the CPU2 in some cases.
 */
#define CFG_HW_BLOCK_FLASH_REQ_BY_CPU1_SEMID 6

/**
 *  Index of the semaphore used to manage the CLK48 clock configuration
 *  When the USB is required, this semaphore shall be taken before configuring te CLK48 for USB
 *  and should be released after the application switch OFF the clock when the USB is not used anymore
 *  When using the RNG, it is good enough to use CFG_HW_RNG_SEMID to control CLK48.
 *  More details in AN5289
 */
#define CFG_HW_CLK48_CONFIG_SEMID 5

/* Index of the semaphore used to manage the entry Stop Mode procedure */
#define CFG_HW_ENTRY_STOP_MODE_SEMID 4

/* Index of the semaphore used to access the RCC */
#define CFG_HW_RCC_SEMID 3

/* Index of the semaphore used to access the FLASH */
#define CFG_HW_FLASH_SEMID 2

/* Index of the semaphore used to access the PKA */
#define CFG_HW_PKA_SEMID 1

/* Index of the semaphore used to access the RNG */
#define CFG_HW_RNG_SEMID 0

/******************************************************************************
 * HW TIMER SERVER
 *****************************************************************************/
/**
 * The user may define the maximum number of virtual timers supported.
 * It shall not exceed 255
 */
#define CFG_HW_TS_MAX_NBR_CONCURRENT_TIMER 6

/**
 * The user may define the priority in the NVIC of the RTC_WKUP interrupt handler that is used to manage the
 * wakeup timer.
 * This setting is the preemptpriority part of the NVIC.
 */
#define CFG_HW_TS_NVIC_RTC_WAKEUP_IT_PREEMPTPRIO 3

/**
 * The user may define the priority in the NVIC of the RTC_WKUP interrupt handler that is used to manage the
 * wakeup timer.
 * This setting is the subpriority part of the NVIC. It does not exist on all processors. When it is not supported
 * on the CPU, the setting is ignored
 */
#define CFG_HW_TS_NVIC_RTC_WAKEUP_IT_SUBPRIO 0

/**
 *  Define a critical section in the Timer server
 *  The Timer server does not support the API to be nested
 *  The  Application shall either:
 *    a) Ensure this will never happen
 *    b) Define the critical section
 *  The default implementations is masking all interrupts using the PRIMASK bit
 *  The TimerServer driver uses critical sections to avoid context corruption. This is achieved with the macro
 *  TIMER_ENTER_CRITICAL_SECTION and TIMER_EXIT_CRITICAL_SECTION. When CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION is set
 *  to 1, all STM32 interrupts are masked with the PRIMASK bit of the CortexM CPU. It is possible to use the BASEPRI
 *  register of the CortexM CPU to keep allowed some interrupts with high priority. In that case, the user shall
 *  re-implement TIMER_ENTER_CRITICAL_SECTION and TIMER_EXIT_CRITICAL_SECTION and shall make sure that no TimerServer
 *  API are called when the TIMER critical section is entered
 */
#define CFG_HW_TS_USE_PRIMASK_AS_CRITICAL_SECTION 1

/**
 * This value shall reflect the maximum delay there could be in the application between the time the RTC interrupt
 * is generated by the Hardware and the time when the  RTC interrupt handler is called. This time is measured in
 * number of RTCCLK ticks.
 * A relaxed timing would be 10ms
 * When the value is too short, the timerserver will not be able to count properly and all timeout may be random.
 * When the value is too long, the device may wake up more often than the most optimal configuration. However, the
 * impact on power consumption would be marginal (unless the value selected is extremely too long). It is strongly
 * recommended to select a value large enough to make sure it is not too short to ensure reliability of the system
 * as this will have marginal impact on low power mode
 */
#define CFG_HW_TS_RTC_HANDLER_MAX_DELAY (10 * (LSI_VALUE / 1000))

/**
 * Interrupt ID in the NVIC of the RTC Wakeup interrupt handler
 * It shall be type of IRQn_Type
 */
#define CFG_HW_TS_RTC_WAKEUP_HANDLER_ID RTC_WKUP_IRQn

/******************************************************************************
 * HW UART
 *****************************************************************************/
/* For release: set to 1  CFG_HW_LPUART1_ENABLED and CFG_HW_LPUART1_DMA_TX_SUPPORTED */
#define CFG_HW_LPUART1_ENABLED 1
#define CFG_HW_LPUART1_DMA_TX_SUPPORTED 1

#define CFG_HW_USART1_ENABLED 1
#define CFG_HW_USART1_DMA_TX_SUPPORTED 1

/**
 * LPUART1
 */
#define CFG_HW_LPUART1_PREEMPTPRIORITY 0x0F
#define CFG_HW_LPUART1_SUBPRIORITY 0

/** < The application shall check the selected source clock is enable */
#define CFG_HW_LPUART1_SOURCE_CLOCK RCC_LPUART1CLKSOURCE_SYSCLK

#define CFG_HW_LPUART1_BAUDRATE 115200
#define CFG_HW_LPUART1_WORDLENGTH UART_WORDLENGTH_8B
#define CFG_HW_LPUART1_STOPBITS UART_STOPBITS_1
#define CFG_HW_LPUART1_PARITY UART_PARITY_NONE
#define CFG_HW_LPUART1_HWFLOWCTL UART_HWCONTROL_NONE
#define CFG_HW_LPUART1_MODE UART_MODE_TX_RX
#define CFG_HW_LPUART1_ADVFEATUREINIT UART_ADVFEATURE_NO_INIT
#define CFG_HW_LPUART1_OVERSAMPLING UART_OVERSAMPLING_8

#define CFG_HW_LPUART1_TX_PORT_CLK_ENABLE __HAL_RCC_GPIOA_CLK_ENABLE
#define CFG_HW_LPUART1_TX_PORT GPIOA
#define CFG_HW_LPUART1_TX_PIN GPIO_PIN_2
#define CFG_HW_LPUART1_TX_MODE GPIO_MODE_AF_PP
#define CFG_HW_LPUART1_TX_PULL GPIO_NOPULL
#define CFG_HW_LPUART1_TX_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_LPUART1_TX_ALTERNATE GPIO_AF8_LPUART1

#define CFG_HW_LPUART1_RX_PORT_CLK_ENABLE __HAL_RCC_GPIOA_CLK_ENABLE
#define CFG_HW_LPUART1_RX_PORT GPIOA
#define CFG_HW_LPUART1_RX_PIN GPIO_PIN_3
#define CFG_HW_LPUART1_RX_MODE GPIO_MODE_AF_PP
#define CFG_HW_LPUART1_RX_PULL GPIO_NOPULL
#define CFG_HW_LPUART1_RX_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_LPUART1_RX_ALTERNATE GPIO_AF8_LPUART1

#define CFG_HW_LPUART1_CTS_PORT_CLK_ENABLE __HAL_RCC_GPIOA_CLK_ENABLE
#define CFG_HW_LPUART1_CTS_PORT GPIOA
#define CFG_HW_LPUART1_CTS_PIN GPIO_PIN_6
#define CFG_HW_LPUART1_CTS_MODE GPIO_MODE_AF_PP
#define CFG_HW_LPUART1_CTS_PULL GPIO_PULLDOWN
#define CFG_HW_LPUART1_CTS_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_LPUART1_CTS_ALTERNATE GPIO_AF8_LPUART1

#define CFG_HW_LPUART1_DMA_TX_PREEMPTPRIORITY 0x0F
#define CFG_HW_LPUART1_DMA_TX_SUBPRIORITY 0

#define CFG_HW_LPUART1_DMAMUX_CLK_ENABLE __HAL_RCC_DMAMUX1_CLK_ENABLE
#define CFG_HW_LPUART1_DMA_CLK_ENABLE __HAL_RCC_DMA1_CLK_ENABLE
#define CFG_HW_LPUART1_TX_DMA_REQ DMA_REQUEST_LPUART1_TX
#define CFG_HW_LPUART1_TX_DMA_CHANNEL DMA1_Channel4
#define CFG_HW_LPUART1_TX_DMA_IRQn DMA1_Channel4_IRQn
#define CFG_HW_LPUART1_DMA_TX_IRQHandler DMA1_Channel4_IRQHandler

/**
 * UART1
 */
#define CFG_HW_USART1_PREEMPTPRIORITY 0x0F
#define CFG_HW_USART1_SUBPRIORITY 0

/** < The application shall check the selected source clock is enable */
#define CFG_HW_USART1_SOURCE_CLOCK RCC_USART1CLKSOURCE_SYSCLK

#define CFG_HW_USART1_BAUDRATE 115200
#define CFG_HW_USART1_WORDLENGTH UART_WORDLENGTH_8B
#define CFG_HW_USART1_STOPBITS UART_STOPBITS_1
#define CFG_HW_USART1_PARITY UART_PARITY_NONE
#define CFG_HW_USART1_HWFLOWCTL UART_HWCONTROL_NONE
#define CFG_HW_USART1_MODE UART_MODE_TX_RX
#define CFG_HW_USART1_ADVFEATUREINIT UART_ADVFEATURE_NO_INIT
#define CFG_HW_USART1_OVERSAMPLING UART_OVERSAMPLING_8

#define CFG_HW_USART1_TX_PORT_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE
#define CFG_HW_USART1_TX_PORT GPIOB
#define CFG_HW_USART1_TX_PIN GPIO_PIN_6
#define CFG_HW_USART1_TX_MODE GPIO_MODE_AF_PP
#define CFG_HW_USART1_TX_PULL GPIO_NOPULL
#define CFG_HW_USART1_TX_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_USART1_TX_ALTERNATE GPIO_AF7_USART1

#define CFG_HW_USART1_RX_PORT_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE
#define CFG_HW_USART1_RX_PORT GPIOB
#define CFG_HW_USART1_RX_PIN GPIO_PIN_7
#define CFG_HW_USART1_RX_MODE GPIO_MODE_AF_PP
#define CFG_HW_USART1_RX_PULL GPIO_NOPULL
#define CFG_HW_USART1_RX_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_USART1_RX_ALTERNATE GPIO_AF7_USART1

#define CFG_HW_USART1_CTS_PORT_CLK_ENABLE __HAL_RCC_GPIOA_CLK_ENABLE
#define CFG_HW_USART1_CTS_PORT GPIOA
#define CFG_HW_USART1_CTS_PIN GPIO_PIN_11
#define CFG_HW_USART1_CTS_MODE GPIO_MODE_AF_PP
#define CFG_HW_USART1_CTS_PULL GPIO_PULLDOWN
#define CFG_HW_USART1_CTS_SPEED GPIO_SPEED_FREQ_VERY_HIGH
#define CFG_HW_USART1_CTS_ALTERNATE GPIO_AF7_USART1

#define CFG_HW_USART1_DMA_TX_PREEMPTPRIORITY 0x0F
#define CFG_HW_USART1_DMA_TX_SUBPRIORITY 0

#define CFG_HW_USART1_DMAMUX_CLK_ENABLE __HAL_RCC_DMAMUX1_CLK_ENABLE
#define CFG_HW_USART1_DMA_CLK_ENABLE __HAL_RCC_DMA2_CLK_ENABLE
#define CFG_HW_USART1_TX_DMA_REQ DMA_REQUEST_USART1_TX
#define CFG_HW_USART1_TX_DMA_CHANNEL DMA2_Channel4
#define CFG_HW_USART1_TX_DMA_IRQn DMA2_Channel4_IRQn
#define CFG_HW_USART1_DMA_TX_IRQHandler DMA2_Channel4_IRQHandler

/******************************************************************************
 * External PA
 *****************************************************************************/
#define CFG_HW_EXTPA_ENABLED 0

// External PA enable pin is chosen by user
#define GPIO_EXT_PA_EN_PIN GPIO_PIN_9
#define GPIO_EXT_PA_EN_PORT GPIOB

// External PA TX/RX pin is fixed by the chip
#define GPIO_EXT_PA_TX_PIN GPIO_PIN_0
#define GPIO_EXT_PA_TX_PORT GPIOB
#endif /*__HW_CONF_H */
