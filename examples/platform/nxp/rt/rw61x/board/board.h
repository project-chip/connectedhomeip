/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "RD-RW61X-BGA"

#ifndef DEBUG_CONSOLE_UART_INDEX
#define DEBUG_CONSOLE_UART_INDEX 3
#endif

/*! @brief Macro to judge XIP */
#define BOARD_IS_XIP()                                                                                                             \
    ((((uint32_t) BOARD_InitDebugConsole >= 0x08000000U) && ((uint32_t) BOARD_InitDebugConsole < 0x10000000U)) ||                  \
     (((uint32_t) BOARD_InitDebugConsole >= 0x18000000U) && ((uint32_t) BOARD_InitDebugConsole < 0x20000000U)))

/*! @brief The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE kSerialPort_Uart
#if DEBUG_CONSOLE_UART_INDEX == 0
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) FLEXCOMM0
#define BOARD_DEBUG_UART_INSTANCE 0U
#define BOARD_DEBUG_UART USART0
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetFlexCommClkFreq(0)
#define BOARD_DEBUG_UART_FRG_CLK                                                                                                   \
    (&(const clock_frg_clk_config_t){ 0, kCLOCK_FrgPllDiv, 255, 0 }) /*!< Select FRG0 mux as frg_pll                               \
                                                                      */
#define BOARD_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM0
#define BOARD_DEBUG_UART_RST kFC0_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_CLKSRC kCLOCK_Flexcomm0
#define BOARD_UART_IRQ_HANDLER FLEXCOMM0_IRQHandler
#define BOARD_UART_IRQ FLEXCOMM0_IRQn
#elif DEBUG_CONSOLE_UART_INDEX == 3
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) FLEXCOMM3
#define BOARD_DEBUG_UART_INSTANCE 3U
#define BOARD_DEBUG_UART USART3
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetFlexCommClkFreq(3)
#define BOARD_DEBUG_UART_FRG_CLK                                                                                                   \
    (&(const clock_frg_clk_config_t){ 3, kCLOCK_FrgPllDiv, 255, 0 }) /*!< Select FRG3 mux as frg_pll                               \
                                                                      */
#define BOARD_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM3
#define BOARD_DEBUG_UART_RST kFC3_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_CLKSRC kCLOCK_Flexcomm3
#define BOARD_UART_IRQ_HANDLER FLEXCOMM3_IRQHandler
#define BOARD_UART_IRQ FLEXCOMM3_IRQn
#else
#error "UART not supported"
#endif

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */

#define BOARD_FLEXSPI_PSRAM FLEXSPI
#ifndef BOARD_ENABLE_PSRAM_CACHE
#define BOARD_ENABLE_PSRAM_CACHE 1
#endif

/* Board I2C for codec */
#define BOARD_CODEC_I2C_BASEADDR I2C2
#define BOARD_CODEC_I2C_CLOCK_FREQ CLOCK_GetFlexCommClkFreq(2U)
#define BOARD_CODEC_I2C_INSTANCE 2
#define BOARD_CODEC_I2C_SDA_PORT 0
#define BOARD_CODEC_I2C_SCL_PORT 0
#define BOARD_CODEC_I2C_SDA_PIN 16
#define BOARD_CODEC_I2C_SCL_PIN 17

/* Board led color mapping */
#define LOGIC_LED_ON 1U
#define LOGIC_LED_OFF 0U

/* A fake led on GPIO header */
#ifndef BOARD_LED_BLUE_GPIO
#define BOARD_LED_BLUE_GPIO GPIO
#endif
#define BOARD_LED_BLUE_GPIO_PORT 0U
#ifndef BOARD_LED_BLUE_GPIO_PIN
#define BOARD_LED_BLUE_GPIO_PIN 2U
#endif

#define LED_BLUE_INIT(output)                                                                                                      \
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN,                                           \
                 &(gpio_pin_config_t){ kGPIO_DigitalOutput, (output) }) /*!< Enable target LED_BLUE */
#define LED_BLUE_ON()                                                                                                              \
    GPIO_PortSet(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn on target LED_BLUE */
#define LED_BLUE_OFF()                                                                                                             \
    GPIO_PortClear(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn off target LED_BLUE */
#define LED_BLUE_TOGGLE()                                                                                                          \
    GPIO_PortToggle(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Toggle on target LED_BLUE   \
                                                                                                   */

/* Board SW PIN */
#ifndef BOARD_SW1_GPIO
#define BOARD_SW1_GPIO GPIO
#endif
#define BOARD_SW1_GPIO_PORT 0U
#ifndef BOARD_SW1_GPIO_PIN
#define BOARD_SW1_GPIO_PIN 13U
#endif

#ifndef BOARD_SW2_GPIO
#define BOARD_SW2_GPIO GPIO
#endif
#define BOARD_SW2_GPIO_PORT 0U
#ifndef BOARD_SW2_GPIO_PIN
#define BOARD_SW2_GPIO_PIN 14U
#endif

#ifndef BOARD_SW3_GPIO
#define BOARD_SW3_GPIO GPIO
#endif
#define BOARD_SW3_GPIO_PORT 0U
#ifndef BOARD_SW3_GPIO_PIN
#define BOARD_SW3_GPIO_PIN 24U
#endif

#ifndef BOARD_SW4_GPIO
#define BOARD_SW4_GPIO GPIO
#endif
#define BOARD_SW4_GPIO_PORT 0U
#ifndef BOARD_SW4_GPIO_PIN
#define BOARD_SW4_GPIO_PIN 25U
#endif

#define BOARD_ENET0_PHY_ADDRESS (0x02U)

/*! @brief The USIM SMARTCARD PHY configuration. */
#define BOARD_SMARTCARD_MODULE (USIM)          /*!< SMARTCARD communicational module instance */
#define BOARD_SMARTCARD_MODULE_IRQ (USIM_IRQn) /*!< SMARTCARD communicational module IRQ handler */
#define BOARD_SMARTCARD_CLOCK_MODULE_CLK_FREQ (CLOCK_GetUsimClkFreq())
#define BOARD_SMARTCARD_CLOCK_VALUE (4000000U) /*!< SMARTCARD clock frequency (4Mhz) */
#define BOARD_SMARTCARD_IRQ_PORT (0)
#define BOARD_SMARTCARD_IRQ_PIN (19)
#define BOARD_SMARTCARD_TS_TIMER_IRQ (CTIMER0_IRQn)

/* CLI clock config */
#define BOARD_CLI_FRG_CLK (&(const clock_frg_clk_config_t){ 3, kCLOCK_FrgPllDiv, 255, 0 }) /*!< Select FRG3 mux as frg_pll */
#define BOARD_CLI_CLK_ATTACH kFRG_to_FLEXCOMM3

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);
status_t BOARD_InitPsRam(void);
void BOARD_InitSleepPinConfig(void);
void BOARD_FlexspiClockSafeConfig(void);
void BOARD_CLIAttachClk(void);
AT_QUICKACCESS_SECTION_CODE(void BOARD_SetFlexspiClock(FLEXSPI_Type * base, uint32_t src, uint32_t divider));
AT_QUICKACCESS_SECTION_CODE(void BOARD_DeinitFlash(FLEXSPI_Type * base));
AT_QUICKACCESS_SECTION_CODE(void BOARD_InitFlash(FLEXSPI_Type * base));

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
