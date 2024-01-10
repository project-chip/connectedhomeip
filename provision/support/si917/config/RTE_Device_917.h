/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2016 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * $Date:        1. December 2016
 * $Revision:    V2.4.4
 *
 * Project:      RTE Device Configuration for Si91x 2.0 B0 BRD4338A
 * -------------------------------------------------------------------------- */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H
#include "rsi_ccp_user_config.h"

#define GPIO_PORT_0       0  // GPIO port 0
#define ULP_GPIO_MODE_6   6  // ULP GPIO mode 6
#define HOST_PAD_GPIO_MIN 25 // GPIO host pad minimum pin number
#define HOST_PAD_GPIO_MAX 30 // GPIO host pad maximum pin number
#define GPIO_MAX_PIN      64 // GPIO maximum pin number

#define BUTTON_0_GPIO_PIN 2

#define RTE_BUTTON0_PORT   0
#define RTE_BUTTON0_NUMBER 0
#define RTE_BUTTON0_PIN    (2U)

#define RTE_BUTTON1_PORT   0
#define RTE_BUTTON1_NUMBER 1
#define RTE_BUTTON1_PIN    (11U)
#define RTE_BUTTON1_PAD    6

#define RTE_LED0_PORT   0
#define RTE_LED0_NUMBER 0
#define RTE_LED0_PIN    (2U)

#define RTE_LED1_PORT      0
#define RTE_LED1_NUMBER    1
#define RTE_LED1_PIN       (10U)
#define BOARD_ACTIVITY_LED (2U) // LED0
#define RTE_LED1_PAD       5

// <e> USART0  [Driver_USART0]
// <i> Configuration settings for Driver_USART0 in component ::CMSIS Driver:USART
#define RTE_ENABLE_FIFO 1

#define RTE_USART0 1

#define RTE_USART0_CLK_SRC      USART_ULPREFCLK
#define RTE_USART0_CLK_DIV_FACT 1
#define RTE_USART0_FRAC_DIV_EN  0

#define RTE_USART_MODE            0 //!Usart mode macros
#define RTE_CONTINUOUS_CLOCK_MODE 0

#define RTE_USART0_LOOPBACK   0
#define RTE_USART0_DTR_EANBLE 0

#define RTE_USART0_DMA_MODE1_EN 0 //!dma mode

#define RTE_USART0_TX_FIFO_THRESHOLD USART_TRIGGER_TX_EMPTY
#define RTE_USART0_RX_FIFO_THRESHOLD USART_TRIGGER_RX_AEMPTY

#define RTE_USART0_DMA_TX_LEN_PER_DES 1024
#define RTE_USART0_DMA_RX_LEN_PER_DES 1024

#define RTE_USART0_CHNL_UDMA_TX_CH 25

#define RTE_USART0_CHNL_UDMA_RX_CH 24

//     <o> USART0_CLK <0=>P0_8  <1=>P0_25 <2=>P0_52 <3=>P0_64
//     <i> CLK of USART0
#define RTE_USART0_CLK_PORT_ID 0

#if (RTE_USART0_CLK_PORT_ID == 0)
#define RTE_USART0_CLK_PORT 0
#define RTE_USART0_CLK_PIN  8
#define RTE_USART0_CLK_MUX  2
#define RTE_USART0_CLK_PAD  3
#elif (RTE_USART0_CLK_PORT_ID == 1)
#define RTE_USART0_CLK_PORT 0
#define RTE_USART0_CLK_PIN  25
#define RTE_USART0_CLK_MUX  2
#define RTE_USART0_CLK_PAD  0 //NO PAD
#elif (RTE_USART0_CLK_PORT_ID == 2)
#define RTE_USART0_CLK_PORT 0
#define RTE_USART0_CLK_PIN  52
#define RTE_USART0_CLK_MUX  2
#define RTE_USART0_CLK_PAD  16
#else
#error "Invalid USART0 RTE_USART0_CLK_PIN Pin Configuration!"
#endif

//     <o> USART0_TX <0=>P0_15 <1=>P0_30  <2=>P0_54  <3=>P0_68 <4=>P0_71
//     <i> TX for USART0

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_USART0_TX_PORT_ID 1
#else
#define RTE_USART0_TX_PORT_ID 0
#endif

#if (RTE_USART0_TX_PORT_ID == 0)
#define RTE_USART0_TX_PORT 0
#define RTE_USART0_TX_PIN  15
#define RTE_USART0_TX_MUX  2
#define RTE_USART0_TX_PAD  8
#elif (RTE_USART0_TX_PORT_ID == 1)
#define RTE_USART0_TX_PORT 0
#define RTE_USART0_TX_PIN  30
#define RTE_USART0_TX_MUX  2
#define RTE_USART0_TX_PAD  0 //NO PAD
#elif (RTE_USART0_TX_PORT_ID == 2)
#define RTE_USART0_TX_PORT 0
#define RTE_USART0_TX_PIN  54
#define RTE_USART0_TX_MUX  2
#define RTE_USART0_TX_PAD  18
#elif (RTE_USART0_TX_PORT_ID == 3)
#define RTE_USART0_TX_PORT 0
#define RTE_USART0_TX_PIN  71
#define RTE_USART0_TX_MUX  4
#define RTE_USART0_TX_PAD  29
#else
#error "Invalid USART0 RTE_USART0_TX_PIN Pin Configuration!"
#endif

//     <o> USART0_RX <0=>P0_10 <1=>P0_29 <2=>P0_55 <3=>P0_65 <4=>P0_70
//     <i> RX for USART0

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_USART0_RX_PORT_ID 1
#else
#define RTE_USART0_RX_PORT_ID 0
#endif

#if (RTE_USART0_RX_PORT_ID == 0)
#define RTE_USART0_RX_PORT 0
#define RTE_USART0_RX_PIN  10
#define RTE_USART0_RX_MUX  2
#define RTE_USART0_RX_PAD  5
#elif (RTE_USART0_RX_PORT_ID == 1)
#define RTE_USART0_RX_PORT 0
#define RTE_USART0_RX_PIN  29
#define RTE_USART0_RX_MUX  2
#define RTE_USART0_RX_PAD  0 //no pad
#elif (RTE_USART0_RX_PORT_ID == 2)
#define RTE_USART0_RX_PORT 0
#define RTE_USART0_RX_PIN  55
#define RTE_USART0_RX_MUX  2
#define RTE_USART0_RX_PAD  19
#elif (RTE_USART0_RX_PORT_ID == 3)
#define RTE_USART0_RX_PORT 0
#define RTE_USART0_RX_PIN  65
#define RTE_USART0_RX_MUX  2
#define RTE_USART0_RX_PAD  24
#elif (RTE_USART0_RX_PORT_ID == 4)
#define RTE_USART0_RX_PORT 0
#define RTE_USART0_RX_PIN  70
#define RTE_USART0_RX_MUX  4
#define RTE_USART0_RX_PAD  28
#else
#error "Invalid USART0 RTE_USART0_RX_PIN Pin Configuration!"
#endif

//     <o> USART0_CTS <0=>P0_6 <1=>P0_26 <2=>P0_56 <3=>P0_70
//     <i> CTS for USART0
#define RTE_USART0_CTS_PORT_ID 0

#if (RTE_USART0_CTS_PORT_ID == 0)
#define RTE_USART0_CTS_PORT 0
#define RTE_USART0_CTS_PIN  6
#define RTE_USART0_CTS_MUX  2
#define RTE_USART0_CTS_PAD  1
#elif (RTE_USART0_CTS_PORT_ID == 1)
#define RTE_USART0_CTS_PORT 0
#define RTE_USART0_CTS_PIN  26
#define RTE_USART0_CTS_MUX  2
#define RTE_USART0_CTS_PAD  0 //NO PAD
#elif (RTE_USART0_CTS_PORT_ID == 2)
#define RTE_USART0_CTS_PORT 0
#define RTE_USART0_CTS_PIN  56
#define RTE_USART0_CTS_MUX  2
#define RTE_USART0_CTS_PAD  20
#elif (RTE_USART0_CTS_PORT_ID == 3)
#define RTE_USART0_CTS_PORT 0
#define RTE_USART0_CTS_PIN  70
#define RTE_USART0_CTS_MUX  2
#define RTE_USART0_CTS_PAD  28
#else
#error "Invalid USART0 RTE_USART0_CTS_PIN Pin Configuration!"
#endif

//     <o> USART0_RTS <0=>P0_9  <1=>P0_28 <2=>P0_53 <3=>P0_69
//     <i> RTS for USART0
#define RTE_USART0_RTS_PORT_ID 0

#if (RTE_USART0_RTS_PORT_ID == 0)
#define RTE_USART0_RTS_PORT 0
#define RTE_USART0_RTS_PIN  9
#define RTE_USART0_RTS_MUX  2
#define RTE_USART0_RTS_PAD  4
#elif (RTE_USART0_RTS_PORT_ID == 1)
#define RTE_USART0_RTS_PORT 0
#define RTE_USART0_RTS_PIN  28
#define RTE_USART0_RTS_MUX  2
#define RTE_USART0_RTS_PAD  0 //NO PAD
#elif (RTE_USART0_RTS_PORT_ID == 2)
#define RTE_USART0_RTS_PORT 0
#define RTE_USART0_RTS_PIN  53
#define RTE_USART0_RTS_MUX  2
#define RTE_USART0_RTS_PAD  17
#else
#error "Invalid USART0 RTE_USART0_RTS_PIN Pin Configuration!"
#endif

//     <o> USART0_IR_TX <0=>P0_48 <1=>P0_72
//     <i> IR TX for USART0

#define RTE_IR_TX_PORT_ID 0
#if ((RTE_IR_TX_PORT_ID == 2))
#error "Invalid USART0 RTE_USART0_IR_TX_PIN Pin Configuration!"
#endif

#if (RTE_IR_TX_PORT_ID == 0)
#define RTE_USART0_IR_TX_PORT 0
#define RTE_USART0_IR_TX_PIN  48
#define RTE_USART0_IR_TX_MUX  2
#define RTE_USART0_IR_TX_PAD  12
#elif (RTE_IR_TX_PORT_ID == 1)
#define RTE_USART0_IR_TX_PORT 0
#define RTE_USART0_IR_TX_PIN  72
#define RTE_USART0_IR_TX_MUX  2
#define RTE_USART0_IR_TX_PAD  30
#elif (RTE_IR_TX_PORT_ID == 2)
#define RTE_USART0_IR_TX_PORT 0
#define RTE_USART0_IR_TX_PIN  26
#define RTE_USART0_IR_TX_MUX  13
#define RTE_USART0_IR_TX_PAD  0 //No pad
#else
#error "Invalid USART0 RTE_USART0_IR_TX_PIN Pin Configuration!"
#endif

//     <o> USART0_IR_RX <0=>P0_47 <1=>P0_71 <2=>P0_64 <3=>P0_25
//     <i> IR RX for USART0

#define RTE_IR_RX_PORT_ID 0
#if ((RTE_IR_RX_PORT_ID == 2))
#error "Invalid USART0 RTE_USART0_IR_RX_PIN Pin Configuration!"
#endif

#if (RTE_IR_RX_PORT_ID == 0)
#define RTE_USART0_IR_RX_PORT 0
#define RTE_USART0_IR_RX_PIN  47
#define RTE_USART0_IR_RX_MUX  2
#define RTE_USART0_IR_RX_PAD  11
#elif (RTE_IR_RX_PORT_ID == 1)
#define RTE_USART0_IR_RX_PORT 0
#define RTE_USART0_IR_RX_PIN  71
#define RTE_USART0_IR_RX_MUX  2
#define RTE_USART0_IR_RX_PAD  29
#elif (RTE_IR_RX_PORT_ID == 2)
#define RTE_USART0_IR_RX_PORT 0
#define RTE_USART0_IR_RX_PIN  25
#define RTE_USART0_IR_RX_MUX  13
#define RTE_USART0_IR_RX_PAD  0 //no pad
#else
#error "Invalid USART0 RTE_USART0_IR_RX_PIN Pin Configuration!"
#endif

//     <o> USART0_RI <0=>P0_27  <1=>P0_46 <2=>P0_68
//     <i> RI for USART0
#define RTE_RI_PORT_ID 0

#if (RTE_RI_PORT_ID == 0)
#define RTE_USART0_RI_PORT 0
#define RTE_USART0_RI_PIN  27
#define RTE_USART0_RI_MUX  2
#define RTE_USART0_RI_PAD  0 //no pad
#elif (RTE_RI_PORT_ID == 1)
#define RTE_USART0_RI_PORT 0
#define RTE_USART0_RI_PIN  46
#define RTE_USART0_RI_MUX  2
#define RTE_USART0_RI_PAD  10
#else
#error "Invalid USART0 RTE_USART0_RI_PIN Pin Configuration!"
#endif

//     <o> USART0_DSR <0=>P0_11 <1=>P0_57
//     <i> DSR for USART0
#define RTE_DSR_PORT_ID 0

#if (RTE_DSR_PORT_ID == 0)
#define RTE_USART0_DSR_PORT 0
#define RTE_USART0_DSR_PIN  11
#define RTE_USART0_DSR_MUX  2
#define RTE_USART0_DSR_PAD  6
#elif (RTE_DSR_PORT_ID == 1)
#define RTE_USART0_DSR_PORT 0
#define RTE_USART0_DSR_PIN  57
#define RTE_USART0_DSR_MUX  2
#define RTE_USART0_DSR_PAD  21
#else
#error "Invalid USART0 RTE_USART0_RI_PIN Pin Configuration!"
#endif
//     <o> USART0_DCD <0=>P0_12 <1=>P0_29
//     <i> DCD for USART0

#define RTE_USART0_DCD_PORT 0
#define RTE_USART0_DCD_PIN  12
#define RTE_USART0_DCD_MUX  2
#define RTE_USART0_DCD_PAD  7

//     <o> USART0_DTR <0=>P0_7
//     <i> DTR for USART0
#define RTE_USART0_DTR_PORT 0
#define RTE_USART0_DTR_PIN  7
#define RTE_USART0_DTR_MUX  2
#define RTE_USART0_DTR_PAD  2
// </e>

// <e> UART1  [Driver_UART1]
// <i> Configuration settings for Driver_UART1 in component ::CMSIS Driver:USART
#define RTE_UART1 1

#define RTE_UART1_CLK_SRC      USART_ULPREFCLK
#define RTE_UART1_CLK_DIV_FACT 1
#define RTE_UART1_FRAC_DIV_EN  0

#define RTE_UART1_LOOPBACK     0
#define RTE_UART1_DMA_MODE1_EN 0

#define RTE_UART1_TX_FIFO_THRESHOLD USART_TRIGGER_TX_EMPTY
#define RTE_UART1_RX_FIFO_THRESHOLD USART_TRIGGER_RX_AEMPTY

#define RTE_UART1_DMA_TX_LEN_PER_DES 1024
#define RTE_UART1_DMA_RX_LEN_PER_DES 1024

#define RTE_UART1_CHNL_UDMA_TX_CH 27

#define RTE_UART1_CHNL_UDMA_RX_CH 26

/*UART1 PINS*/
//     <o> UART1_TX <0=>P0_7 <1=>P0_30 <2=>P0_67 <3=>P0_69 <4=>P0_73 <5=>P0_75 <6=>P0_34
//     <i> TX of UART1

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_UART1_TX_PORT_ID 3
#else
#define RTE_UART1_TX_PORT_ID 0

#if ((RTE_UART1_TX_PORT_ID == 6))
#error "Invalid UART1 RTE_UART1_TX_PIN Configuration!"
#endif
#endif

#if (RTE_UART1_TX_PORT_ID == 0)
#define RTE_UART1_TX_PORT 0
#define RTE_UART1_TX_PIN  7
#define RTE_UART1_TX_MUX  6
#define RTE_UART1_TX_PAD  2
#elif (RTE_UART1_TX_PORT_ID == 1)
#define RTE_UART1_TX_PORT 0
#define RTE_UART1_TX_PIN  30
#define RTE_UART1_TX_MUX  6
#define RTE_UART1_TX_PAD  0 //no pad
#elif (RTE_UART1_TX_PORT_ID == 2)
#define RTE_UART1_TX_PORT 0
#define RTE_UART1_TX_PIN  67
#define RTE_UART1_TX_MUX  9
#define RTE_UART1_TX_PAD  25
#elif (RTE_UART1_TX_PORT_ID == 3)
#define RTE_UART1_TX_PORT 0
#define RTE_UART1_TX_PIN  73
#define RTE_UART1_TX_MUX  6
#define RTE_UART1_TX_PAD  31
#elif (RTE_UART1_TX_PORT_ID == 4)
#define RTE_UART1_TX_PORT 0
#define RTE_UART1_TX_PIN  75
#define RTE_UART1_TX_MUX  9
#define RTE_UART1_TX_PAD  33
#else
#error "Invalid UART1 RTE_UART1_TX_PIN Pin Configuration!"
#endif

//     <o> UART1_RX <0=>P0_6 <1=>P0_29 <2=>P0_66 <3=>P0_68 <4=>P0_72 <5=>P0_74  <6=>P0_33
//     <i> RX of UART1

#define RTE_UART1_RX_PORT_ID 0

#if (RTE_UART1_RX_PORT_ID == 0)
#define RTE_UART1_RX_PORT 0
#define RTE_UART1_RX_PIN  6
#define RTE_UART1_RX_MUX  6
#define RTE_UART1_RX_PAD  1
#elif (RTE_UART1_RX_PORT_ID == 1)
#define RTE_UART1_RX_PORT 0
#define RTE_UART1_RX_PIN  29
#define RTE_UART1_RX_MUX  6
#define RTE_UART1_RX_PAD  0 //no pad
#elif (RTE_UART1_RX_PORT_ID == 2)
#define RTE_UART1_RX_PORT 0
#define RTE_UART1_RX_PIN  66
#define RTE_UART1_RX_MUX  9
#define RTE_UART1_RX_PAD  24
#elif (RTE_UART1_RX_PORT_ID == 3)
#define RTE_UART1_RX_PORT 0
#define RTE_UART1_RX_PIN  72
#define RTE_UART1_RX_MUX  6
#define RTE_UART1_RX_PAD  30
#elif (RTE_UART1_RX_PORT_ID == 4)
#define RTE_UART1_RX_PORT 0
#define RTE_UART1_RX_PIN  74
#define RTE_UART1_RX_MUX  9
#define RTE_UART1_RX_PAD  32
#else
#error "Invalid UART1 RTE_UART1_RX_PIN Pin Configuration!"
#endif

//     <o> UART1_CTS <0=>P0_11 <1=>P0_28 <2=>P0_51 <3=>P0_65 <4=>P0_71 <5=>P0_73 <6=>P0_32
//     <i> CTS of UART1
#define RTE_UART1_CTS_PORT_ID 0

#if (RTE_UART1_CTS_PORT_ID == 0)
#define RTE_UART1_CTS_PORT 0
#define RTE_UART1_CTS_PIN  11
#define RTE_UART1_CTS_MUX  6
#define RTE_UART1_CTS_PAD  6
#elif (RTE_UART1_CTS_PORT_ID == 1)
#define RTE_UART1_CTS_PORT 0
#define RTE_UART1_CTS_PIN  28
#define RTE_UART1_CTS_MUX  6
#define RTE_UART1_CTS_PAD  0 //no pad
#elif (RTE_UART1_CTS_PORT_ID == 2)
#define RTE_UART1_CTS_PORT 0
#define RTE_UART1_CTS_PIN  51
#define RTE_UART1_CTS_MUX  9
#define RTE_UART1_CTS_PAD  15
#elif (RTE_UART1_CTS_PORT_ID == 3)
#define RTE_UART1_CTS_PORT 0
#define RTE_UART1_CTS_PIN  65
#define RTE_UART1_CTS_MUX  9
#define RTE_UART1_CTS_PAD  23
#elif (RTE_UART1_CTS_PORT_ID == 4)
#define RTE_UART1_CTS_PORT 0
#define RTE_UART1_CTS_PIN  71
#define RTE_UART1_CTS_MUX  6
#define RTE_UART1_CTS_PAD  29
#else
#error "Invalid UART1 RTE_UART1_CTS_PIN Pin Configuration!"
#endif

//     <o> UART1_RTS  <0=>P0_10 <1=>P0_27 <2=>P0_50 <3=>P0_64  <4=>P0_70 <5=>P0_72 <6=>P0_31
//     <i> RTS of UART1

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_UART1_RTS_PORT_ID 0
#else
#define RTE_UART1_RTS_PORT_ID 0
#endif

#if (RTE_UART1_RTS_PORT_ID == 0)
#define RTE_UART1_RTS_PORT 0
#define RTE_UART1_RTS_PIN  10
#define RTE_UART1_RTS_MUX  6
#define RTE_UART1_RTS_PAD  5
#elif (RTE_UART1_RTS_PORT_ID == 1)
#define RTE_UART1_RTS_PORT 0
#define RTE_UART1_RTS_PIN  27
#define RTE_UART1_RTS_MUX  6
#define RTE_UART1_RTS_PAD  0 //no pad
#elif (RTE_UART1_RTS_PORT_ID == 2)
#define RTE_UART1_RTS_PORT 0
#define RTE_UART1_RTS_PIN  50
#define RTE_UART1_RTS_MUX  9
#define RTE_UART1_RTS_PAD  14
#elif (RTE_UART1_RTS_PORT_ID == 3)
#define RTE_UART1_RTS_PORT 0
#define RTE_UART1_RTS_PIN  70
#define RTE_UART1_RTS_MUX  6
#define RTE_UART1_RTS_PAD  28
#elif (RTE_UART1_RTS_PORT_ID == 4)
#define RTE_UART1_RTS_PORT 0
#define RTE_UART1_RTS_PIN  72
#define RTE_UART1_RTS_MUX  9
#define RTE_UART1_RTS_PAD  30
#else
#error "Invalid UART1 RTE_UART1_RTS_PIN Pin Configuration!"
#endif
// </e>

// <e> ULP_UART  [Driver_ULP_UART]
// <i> Configuration settings for Driver_ULP_UART in component ::CMSIS Driver:USART
#define RTE_ULP_UART 1

#define RTE_ULP_UART_CLK_SRC      ULP_UART_REF_CLK
#define RTE_ULP_UART_CLK_DIV_FACT 0
#define RTE_ULP_UART_FRAC_SEL     0

#define RTE_ULP_UART_LOOPBACK     0
#define RTE_ULP_UART_DMA_MODE1_EN 0

#define RTE_ULP_UART_TX_FIFO_THRESHOLD USART_TRIGGER_TX_EMPTY
#define RTE_ULP_UART_RX_FIFO_THRESHOLD USART_TRIGGER_RX_AEMPTY

#define RTE_ULP_UART_DMA_TX_LEN_PER_DES 1024
#define RTE_ULP_UART_DMA_RX_LEN_PER_DES 1024

#define RTE_ULPUART_CHNL_UDMA_TX_CH 1

#define RTE_ULPUART_CHNL_UDMA_RX_CH 0

/*ULPSS UART PINS*/
//     <o> UART1_TX <0=>P0_3 <1=>P0_7 <2=>P0_11
//     <i> TX of ULPSS UART
#define RTE_ULP_UART_TX_PORT_ID 1
#if (RTE_ULP_UART_TX_PORT_ID == 0)
#define RTE_ULP_UART_TX_PORT 0
#define RTE_ULP_UART_TX_PIN  7
#define RTE_ULP_UART_TX_MUX  3
#elif (RTE_ULP_UART_TX_PORT_ID == 1)
#define RTE_ULP_UART_TX_PORT 0
#define RTE_ULP_UART_TX_PIN  11
#define RTE_ULP_UART_TX_MUX  3
#else
#error "Invalid ULPSS UART RTE_ULP_UART_TX_PIN Pin Configuration!"
#endif

//     <o> UART1_RX <0=>P0_2 <1=>P0_6 <2=>P0_9
//     <i> RX of ULPSS UART
#define RTE_ULP_UART_RX_PORT_ID 2
#if (RTE_ULP_UART_RX_PORT_ID == 0)
#define RTE_ULP_UART_RX_PORT 0
#define RTE_ULP_UART_RX_PIN  2
#define RTE_ULP_UART_RX_MUX  3
#elif (RTE_ULP_UART_RX_PORT_ID == 1)
#define RTE_ULP_UART_RX_PORT 0
#define RTE_ULP_UART_RX_PIN  6
#define RTE_ULP_UART_RX_MUX  3
#elif (RTE_ULP_UART_RX_PORT_ID == 2)
#define RTE_ULP_UART_RX_PORT 0
#define RTE_ULP_UART_RX_PIN  9
#define RTE_ULP_UART_RX_MUX  3
#else
#error "Invalid ULPSS UART RTE_ULP_UART_RX_PIN Pin Configuration!"
#endif

//     <o> UART1_CTS <0=>P0_1 <1=>P0_5 <2=>P0_8
//     <i> CTS of ULPSS UART
#define RTE_ULP_UART_CTS_PORT_ID 0
#if (RTE_ULP_UART_CTS_PORT_ID == 0)
#define RTE_ULP_UART_CTS_PORT 0
#define RTE_ULP_UART_CTS_PIN  1
#define RTE_ULP_UART_CTS_MUX  3
#elif (RTE_ULP_UART_CTS_PORT_ID == 1)
#define RTE_ULP_UART_CTS_PORT 0
#define RTE_ULP_UART_CTS_PIN  8
#define RTE_ULP_UART_CTS_MUX  3
#else
#error "Invalid ULPSS UART RTE_ULP_UART_CTS_PIN Pin Configuration!"
#endif

//     <o> UART1_RTS <0=>P0_0 <1=>P0_4 <2=>P0_10
//     <i> RTS of ULPSS UART
#define RTE_ULP_UART_RTS_PORT_ID 0
#if (RTE_ULP_UART_RTS_PORT_ID == 0)
#define RTE_ULP_UART_RTS_PORT 0
#define RTE_ULP_UART_RTS_PIN  10
#define RTE_ULP_UART_RTS_MUX  8
#else
#error "Invalid ULPSS UART RTE_ULP_UART_RTS_PIN Pin Configuration!"
#endif
//   </e>

// <e> SSI_MASTER (Serial Peripheral Interface 1) [Driver_SSI_MASTER]
// <i> Configuration settings for Driver_SSI_MASTER in component ::CMSIS Driver:SPI
#define RTE_SSI_MASTER 1

#define RTE_SSI_MASTER_INPUT_CLOCK SSI_SOCPLLCLK

// <o> SSI_MASTER_MISO Pin <0=>GPIO_12 <1=>GPIO_27 <2=>GPIO_57

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SSI_MASTER_MISO_PORT_ID 1
#else
#define RTE_SSI_MASTER_MISO_PORT_ID 0
#endif

#if (RTE_SSI_MASTER_MISO_PORT_ID == 0)
#define RTE_SSI_MASTER_MISO        1
#define RTE_SSI_MASTER_MISO_PORT   0
#define RTE_SSI_MASTER_MISO_PIN    12
#define RTE_SSI_MASTER_MISO_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MISO_PADSEL 7
#elif (RTE_SSI_MASTER_MISO_PORT_ID == 1)
#define RTE_SSI_MASTER_MISO        1
#define RTE_SSI_MASTER_MISO_PORT   0
#define RTE_SSI_MASTER_MISO_PIN    27
#define RTE_SSI_MASTER_MISO_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MISO_PADSEL 0 //NO PAD
#elif (RTE_SSI_MASTER_MISO_PORT_ID == 2)
#define RTE_SSI_MASTER_MISO        1
#define RTE_SSI_MASTER_MISO_PORT   0
#define RTE_SSI_MASTER_MISO_PIN    57
#define RTE_SSI_MASTER_MISO_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MISO_PADSEL 21
#else
#error "Invalid SSI_MASTER_MISO Pin Configuration!"
#endif

// <o> SSI_MASTER_MOSI Pin <0=>GPIO_11 <1=>GPIO_26 <2=>GPIO_56
#define RTE_SSI_MASTER_MOSI_PORT_ID 1

#if (RTE_SSI_MASTER_MOSI_PORT_ID == 0)
#define RTE_SSI_MASTER_MOSI        1
#define RTE_SSI_MASTER_MOSI_PORT   0
#define RTE_SSI_MASTER_MOSI_PIN    11
#define RTE_SSI_MASTER_MOSI_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MOSI_PADSEL 6
#elif (RTE_SSI_MASTER_MOSI_PORT_ID == 1)
#define RTE_SSI_MASTER_MOSI        1
#define RTE_SSI_MASTER_MOSI_PORT   0
#define RTE_SSI_MASTER_MOSI_PIN    26
#define RTE_SSI_MASTER_MOSI_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MOSI_PADSEL 0 //NO PAD
#elif (RTE_SSI_MASTER_MOSI_PORT_ID == 2)
#define RTE_SSI_MASTER_MOSI        1
#define RTE_SSI_MASTER_MOSI_PORT   0
#define RTE_SSI_MASTER_MOSI_PIN    56
#define RTE_SSI_MASTER_MOSI_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_MOSI_PADSEL 20
#else
#error "Invalid SSI_MASTER_MOSI Pin Configuration!"
#endif

// <o> SSI_MASTER_SCK Pin <0=>GPIO_8 <1=>GPIO_25 <2=>GPIO_52
#define RTE_SSI_MASTER_SCK_PORT_ID 1

#if (RTE_SSI_MASTER_SCK_PORT_ID == 0)
#define RTE_SSI_MASTER_SCK        1
#define RTE_SSI_MASTER_SCK_PORT   0
#define RTE_SSI_MASTER_SCK_PIN    8
#define RTE_SSI_MASTER_SCK_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_SCK_PADSEL 3
#elif (RTE_SSI_MASTER_SCK_PORT_ID == 1)
#define RTE_SSI_MASTER_SCK        1
#define RTE_SSI_MASTER_SCK_PORT   0
#define RTE_SSI_MASTER_SCK_PIN    25
#define RTE_SSI_MASTER_SCK_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_SCK_PADSEL 0 //NO PAD
#elif (RTE_SSI_MASTER_SCK_PORT_ID == 2)
#define RTE_SSI_MASTER_SCK        1
#define RTE_SSI_MASTER_SCK_PORT   0
#define RTE_SSI_MASTER_SCK_PIN    52
#define RTE_SSI_MASTER_SCK_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_SCK_PADSEL 16
#else
#error "Invalid SSI_MASTER_SCK Pin Configuration!"
#endif

#define M4_SSI_CS0 1
#define M4_SSI_CS1 0
#define M4_SSI_CS2 0
#define M4_SSI_CS3 0

// <o> SSI_MASTER_CS Pin <0=>GPIO_9 <1=>GPIO_28 <2=>GPIO_53 <3=>GPIO_10 <4=>GPIO_15 <5=>GPIO_50 <6=>GPIO_51
#define RTE_SSI_MASTER_CS0_PORT_ID 1

#if (RTE_SSI_MASTER_CS0_PORT_ID == 0)
#define RTE_SSI_MASTER_CS0        M4_SSI_CS0
#define RTE_SSI_MASTER_CS0_PORT   0
#define RTE_SSI_MASTER_CS0_PIN    9
#define RTE_SSI_MASTER_CS0_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS0_PADSEL 4
#elif (RTE_SSI_MASTER_CS0_PORT_ID == 1)
#define RTE_SSI_MASTER_CS0        M4_SSI_CS0
#define RTE_SSI_MASTER_CS0_PORT   0
#define RTE_SSI_MASTER_CS0_PIN    28
#define RTE_SSI_MASTER_CS0_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS0_PADSEL 0 //NO PAD
#elif (RTE_SSI_MASTER_CS0_PORT_ID == 2)
#define RTE_SSI_MASTER_CS0        M4_SSI_CS0
#define RTE_SSI_MASTER_CS0_PORT   0
#define RTE_SSI_MASTER_CS0_PIN    53
#define RTE_SSI_MASTER_CS0_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS0_PADSEL 17
#else
#error "Invalid SSI_MASTER_CS0 Pin Configuration!"
#endif

//CS1
#define RTE_SSI_MASTER_CS1_PORT_ID 0
#if (RTE_SSI_MASTER_CS1_PORT_ID == 0)
#define RTE_SSI_MASTER_CS1        M4_SSI_CS1
#define RTE_SSI_MASTER_CS1_PORT   0
#define RTE_SSI_MASTER_CS1_PIN    10
#define RTE_SSI_MASTER_CS1_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS1_PADSEL 5
#else
#error "Invalid SSI_MASTER_CS1 Pin Configuration!"
#endif

//CS2
#define RTE_SSI_MASTER_CS2_PORT_ID 1
#if (RTE_SSI_MASTER_CS2_PORT_ID == 0)
#define RTE_SSI_MASTER_CS2        M4_SSI_CS2
#define RTE_SSI_MASTER_CS2_PORT   0
#define RTE_SSI_MASTER_CS2_PIN    15
#define RTE_SSI_MASTER_CS2_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS2_PADSEL 8
#elif (RTE_SSI_MASTER_CS2_PORT_ID == 1)
#define RTE_SSI_MASTER_CS2        M4_SSI_CS2
#define RTE_SSI_MASTER_CS2_PORT   0
#define RTE_SSI_MASTER_CS2_PIN    50
#define RTE_SSI_MASTER_CS2_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS2_PADSEL 14
#else
#error "Invalid SSI_MASTER_CS2 Pin Configuration!"
#endif

//CS3
#define RTE_SSI_MASTER_CS3_PORT_ID 0
#if (RTE_SSI_MASTER_CS3_PORT_ID == 0)
#define RTE_SSI_MASTER_CS3        M4_SSI_CS3
#define RTE_SSI_MASTER_CS3_PORT   0
#define RTE_SSI_MASTER_CS3_PIN    51
#define RTE_SSI_MASTER_CS3_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS3_PADSEL 15
#else
#error "Invalid SSI_MASTER_CS3 Pin Configuration!"
#endif

//   <e> DMA Rx
//     <o3> Channel <28=>28
//     <i>  Selects DMA Channel (only Channel 28 can be used)
//   </e>
#define RTE_SSI_MASTER_UDMA_RX_CH 28

//   <e> DMA Tx
//     <o3> Channel <29=>29
//     <i>  Selects DMA Channel (only Channel 29 can be used)
//   </e>
#define RTE_SSI_MASTER_UDMA_TX_CH 29
// </e>

// <e> SSI_SLAVE (Serial Peripheral Interface 2) [Driver_SSI_SLAVE]
// <i> Configuration settings for Driver_SSI_SLAVE in component ::CMSIS Driver:SPI
#define RTE_SSI_SLAVE 1

#define RTE_SSI_SLAVE_INPUT_CLOCK SSISLAVE_CLK

// <o> SSI_SLAVE_MISO Pin <0=>Not Used <1=>GPIO_11  <2=>GPIO_28 <3=>GPIO_49 <4=>GPIO_57
#define RTE_SSI_SLAVE_MISO_PORT_ID 2

#if (RTE_SSI_SLAVE_MISO_PORT_ID == 0)
#define RTE_SSI_SLAVE_MISO 0
#elif (RTE_SSI_SLAVE_MISO_PORT_ID == 1)
#define RTE_SSI_SLAVE_MISO        1
#define RTE_SSI_SLAVE_MISO_PORT   0
#define RTE_SSI_SLAVE_MISO_PIN    11
#define RTE_SSI_SLAVE_MISO_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MISO_PADSEL 6
#elif (RTE_SSI_SLAVE_MISO_PORT_ID == 2)
#define RTE_SSI_SLAVE_MISO        1
#define RTE_SSI_SLAVE_MISO_PORT   0
#define RTE_SSI_SLAVE_MISO_PIN    28
#define RTE_SSI_SLAVE_MISO_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MISO_PADSEL 0 //no pad
#elif (RTE_SSI_SLAVE_MISO_PORT_ID == 3)
#define RTE_SSI_SLAVE_MISO        1
#define RTE_SSI_SLAVE_MISO_PORT   0
#define RTE_SSI_SLAVE_MISO_PIN    49
#define RTE_SSI_SLAVE_MISO_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MISO_PADSEL 13
#elif (RTE_SSI_SLAVE_MISO_PORT_ID == 4)
#define RTE_SSI_SLAVE_MISO        1
#define RTE_SSI_SLAVE_MISO_PORT   0
#define RTE_SSI_SLAVE_MISO_PIN    57
#define RTE_SSI_SLAVE_MISO_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MISO_PADSEL 21
#else
#error "Invalid SSI_SLAVE_MISO Pin Configuration!"
#endif

// <o> SSI_SLAVE_MOSI Pin <0=>Not Used <1=>GPIO_10 <2=>GPIO_27 <3=>GPIO_48 <4=>GPIO_56

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SSI_SLAVE_MOSI_PORT_ID 2
#else
#define RTE_SSI_SLAVE_MOSI_PORT_ID 1
#endif

#if (RTE_SSI_SLAVE_MOSI_PORT_ID == 0)
#define RTE_SSI_SLAVE_MOSI 0
#elif (RTE_SSI_SLAVE_MOSI_PORT_ID == 1)
#define RTE_SSI_SLAVE_MOSI        1
#define RTE_SSI_SLAVE_MOSI_PORT   0
#define RTE_SSI_SLAVE_MOSI_PIN    10
#define RTE_SSI_SLAVE_MOSI_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MOSI_PADSEL 5
#elif (RTE_SSI_SLAVE_MOSI_PORT_ID == 2)
#define RTE_SSI_SLAVE_MOSI        1
#define RTE_SSI_SLAVE_MOSI_PORT   0
#define RTE_SSI_SLAVE_MOSI_PIN    27
#define RTE_SSI_SLAVE_MOSI_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MOSI_PADSEL 0 //no pad
#elif (RTE_SSI_SLAVE_MOSI_PORT_ID == 3)
#define RTE_SSI_SLAVE_MOSI        1
#define RTE_SSI_SLAVE_MOSI_PORT   0
#define RTE_SSI_SLAVE_MOSI_PIN    48
#define RTE_SSI_SLAVE_MOSI_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MOSI_PADSEL 12
#elif (RTE_SSI_SLAVE_MOSI_PORT_ID == 4)
#define RTE_SSI_SLAVE_MOSI        1
#define RTE_SSI_SLAVE_MOSI_PORT   0
#define RTE_SSI_SLAVE_MOSI_PIN    56
#define RTE_SSI_SLAVE_MOSI_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_MOSI_PADSEL 20
#else
#error "Invalid SSI_SLAVE_MOSI Pin Configuration!"
#endif

// <o> SSI_SLAVE_SCK Pin <0=>Not Used <1=>GPIO_8 <2=>GPIO_26 <3=>GPIO_47 <4=>GPIO_52
#define RTE_SSI_SLAVE_SCK_PORT_ID 2

#if (RTE_SSI_SLAVE_SCK_PORT_ID == 0)
#define RTE_SSI_SLAVE_SCK 0
#elif (RTE_SSI_SLAVE_SCK_PORT_ID == 1)
#define RTE_SSI_SLAVE_SCK        1
#define RTE_SSI_SLAVE_SCK_PORT   0
#define RTE_SSI_SLAVE_SCK_PIN    8
#define RTE_SSI_SLAVE_SCK_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_SCK_PADSEL 3
#elif (RTE_SSI_SLAVE_SCK_PORT_ID == 2)
#define RTE_SSI_SLAVE_SCK        1
#define RTE_SSI_SLAVE_SCK_PORT   0
#define RTE_SSI_SLAVE_SCK_PIN    26
#define RTE_SSI_SLAVE_SCK_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_SCK_PADSEL 0 //no pad
#elif (RTE_SSI_SLAVE_SCK_PORT_ID == 3)
#define RTE_SSI_SLAVE_SCK        1
#define RTE_SSI_SLAVE_SCK_PORT   0
#define RTE_SSI_SLAVE_SCK_PIN    47
#define RTE_SSI_SLAVE_SCK_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_SCK_PADSEL 11
#elif (RTE_SSI_SLAVE_SCK_PORT_ID == 4)
#define RTE_SSI_SLAVE_SCK        1
#define RTE_SSI_SLAVE_SCK_PORT   0
#define RTE_SSI_SLAVE_SCK_PIN    52
#define RTE_SSI_SLAVE_SCK_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_SCK_PADSEL 16
#else
#error "Invalid SSI_SLAVE_SCK Pin Configuration!"
#endif

// <o> SSI_SLAVE_CS Pin <0=>Not Used <1=>GPIO_9 <2=>GPIO_25 <3=>GPIO_46 <4=>GPIO_53
#define RTE_SSI_SLAVE_CS_PORT_ID 1

#if (RTE_SSI_SLAVE_CS_PORT_ID == 0)
#define RTE_SSI_SLAVE_CS 0
#elif (RTE_SSI_SLAVE_CS_PORT_ID == 1)
#define RTE_SSI_SLAVE_CS        1
#define RTE_SSI_SLAVE_CS_PORT   0
#define RTE_SSI_SLAVE_CS_PIN    9
#define RTE_SSI_SLAVE_CS_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_CS_PADSEL 4
#elif (RTE_SSI_SLAVE_CS_PORT_ID == 2)
#define RTE_SSI_SLAVE_CS        1
#define RTE_SSI_SLAVE_CS_PORT   0
#define RTE_SSI_SLAVE_CS_PIN    25
#define RTE_SSI_SLAVE_CS_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_CS_PADSEL 0 //no pad
#elif (RTE_SSI_SLAVE_CS_PORT_ID == 3)
#define RTE_SSI_SLAVE_CS        1
#define RTE_SSI_SLAVE_CS_PORT   0
#define RTE_SSI_SLAVE_CS_PIN    46
#define RTE_SSI_SLAVE_CS_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_CS_PADSEL 10
#elif (RTE_SSI_SLAVE_CS_PORT_ID == 4)
#define RTE_SSI_SLAVE_CS        1
#define RTE_SSI_SLAVE_CS_PORT   0
#define RTE_SSI_SLAVE_CS_PIN    53
#define RTE_SSI_SLAVE_CS_MODE   EGPIO_PIN_MUX_MODE8
#define RTE_SSI_SLAVE_CS_PADSEL 17
#else
#error "Invalid SSI_SLAVE_CS Pin Configuration!"
#endif

//   <e> DMA Rx
//     <o3> Channel <22=>22
//     <i>  Selects DMA Channel (only Channel 22 can be used)
//   </e>
#define RTE_SSI_SLAVE_UDMA_RX_CH         22
#define RTE_SSI_SLAVE_DMA_RX_LEN_PER_DES 1024

//   <e> DMA Tx
//     <o3> Channel <23=>23
//     <i>  Selects DMA Channel (only Channel 23 can be used)
//   </e>
#define RTE_SSI_SLAVE_UDMA_TX_CH         23
#define RTE_SSI_SLAVE_DMA_TX_LEN_PER_DES 1024

// </e>

// <e> SSI_ULP_MASTER (Serial Peripheral Interface 3) [Driver_SSI_ULP_MASTER]
// <i> Configuration settings for Driver_SSI_ULP_MASTER in component ::CMSIS Driver:SPI
#define RTE_SSI_ULP_MASTER 1

#define RTE_SSI_ULP_MASTER_INPUT_CLOCK ULP_SSI_ULP_32MHZ_RC_CLK

// <e> Enable multiple CSN lines
#define ULP_SSI_CS0 1
#define ULP_SSI_CS1 0
#define ULP_SSI_CS2 0

// <o> SSI_ULP_MASTER_MISO Pin <0=>Not Used <1=>ULP_GPIO_2 <2=>ULP_GPIO_9
#define RTE_SSI_ULP_MASTER_MISO_PORT_ID 0
#if (RTE_SSI_ULP_MASTER_MISO_PORT_ID == 0)
#define RTE_SSI_ULP_MASTER_MISO      1
#define RTE_SSI_ULP_MASTER_MISO_PORT 0
#define RTE_SSI_ULP_MASTER_MISO_PIN  2
#define RTE_SSI_ULP_MASTER_MISO_MODE 1
#elif (RTE_SSI_ULP_MASTER_MISO_PORT_ID == 1)
#define RTE_SSI_ULP_MASTER_MISO      1
#define RTE_SSI_ULP_MASTER_MISO_PORT 0
#define RTE_SSI_ULP_MASTER_MISO_PIN  9
#define RTE_SSI_ULP_MASTER_MISO_MODE 1
#else
#error "Invalid SSI_ULP_MISO Pin Configuration!"
#endif

// <o> SSI_ULP_MASTER_MOSI Pin <0=>Not Used <1=>ULP_GPIO_1 <2=>ULP_GPIO_11
#define RTE_SSI_ULP_MASTER_MOSI_PORT_ID 0
#if (RTE_SSI_ULP_MASTER_MOSI_PORT_ID == 0)
#define RTE_SSI_ULP_MASTER_MOSI      1
#define RTE_SSI_ULP_MASTER_MOSI_PORT 0
#define RTE_SSI_ULP_MASTER_MOSI_PIN  1
#define RTE_SSI_ULP_MASTER_MOSI_MODE 1
#elif (RTE_SSI_ULP_MASTER_MOSI_PORT_ID == 1)
#define RTE_SSI_ULP_MASTER_MOSI      1
#define RTE_SSI_ULP_MASTER_MOSI_PORT 0
#define RTE_SSI_ULP_MASTER_MOSI_PIN  11
#define RTE_SSI_ULP_MASTER_MOSI_MODE 1
#else
#error "Invalid SSI_ULP_MOSI Pin Configuration!"
#endif

// <o> SSI_ULP_MASTER_SCK Pin <0=>Not Used <1=>ULP_GPIO_0 <2=>ULP_GPIO_8
#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SSI_ULP_MASTER_SCK_PORT_ID 2
#else
#define RTE_SSI_ULP_MASTER_SCK_PORT_ID 2
#endif
#if (RTE_SSI_ULP_MASTER_SCK_PORT_ID == 0)
#define RTE_SSI_ULP_MASTER_SCK 0
#elif (RTE_SSI_ULP_MASTER_SCK_PORT_ID == 1)
#define RTE_SSI_ULP_MASTER_SCK      1
#define RTE_SSI_ULP_MASTER_SCK_PORT 0
#define RTE_SSI_ULP_MASTER_SCK_PIN  0
#define RTE_SSI_ULP_MASTER_SCK_MODE 1
#elif (RTE_SSI_ULP_MASTER_SCK_PORT_ID == 2)
#define RTE_SSI_ULP_MASTER_SCK      1
#define RTE_SSI_ULP_MASTER_SCK_PORT 0
#define RTE_SSI_ULP_MASTER_SCK_PIN  8
#define RTE_SSI_ULP_MASTER_SCK_MODE 1
#else
#error "Invalid SSI_ULP_SCK Pin Configuration!"
#endif

// CS0
#define RTE_SSI_ULP_MASTER_CS0_PORT_ID 1
#if (RTE_SSI_ULP_MASTER_CS0_PORT_ID == 0)
#define RTE_SSI_ULP_MASTER_CS0      ULP_SSI_CS0
#define RTE_SSI_ULP_MASTER_CS0_PORT 0
#define RTE_SSI_ULP_MASTER_CS0_PIN  3
#define RTE_SSI_ULP_MASTER_CS0_MODE 1
#elif (RTE_SSI_ULP_MASTER_CS0_PORT_ID == 1)
#define RTE_SSI_ULP_MASTER_CS0      ULP_SSI_CS0
#define RTE_SSI_ULP_MASTER_CS0_PORT 0
#define RTE_SSI_ULP_MASTER_CS0_PIN  10
#define RTE_SSI_ULP_MASTER_CS0_MODE 1
#else
#error "Change RTE_SSI_ULP_MASTER_CS_PORT_ID other than  '1' or'3' as per PACKAGE_TYPE"
#endif

// CS1
#define RTE_SSI_ULP_MASTER_CS1      ULP_SSI_CS1
#define RTE_SSI_ULP_MASTER_CS1_PORT 0
#define RTE_SSI_ULP_MASTER_CS1_PIN  4
#define RTE_SSI_ULP_MASTER_CS1_MODE 1

// CS2
#define RTE_SSI_ULP_MASTER_CS2      ULP_SSI_CS2
#define RTE_SSI_ULP_MASTER_CS2_PORT 0
#define RTE_SSI_ULP_MASTER_CS2_PIN  6
#define RTE_SSI_ULP_MASTER_CS2_MODE 1

//   <e> DMA Rx
//     <o3> Channel <2=>2
//     <i>  Selects DMA Channel (only Channel 2 can be used)
//   </e>
#define RTE_SSI_ULP_MASTER_UDMA_RX_CH         2
#define RTE_SSI_ULP_MASTER_DMA_RX_LEN_PER_DES 96

//   <e> DMA Tx
//     <o3> Channel <3=>3
//     <i>  Selects DMA Channel (only Channel 3 can be used)
//   </e>
#define RTE_SSI_ULP_MASTER_UDMA_TX_CH         3
#define RTE_SSI_ULP_MASTER_DMA_TX_LEN_PER_DES 96

// </e>
/*===================================================================
                            UDMA Defines
====================================================================*/
// <e> UDMA  [Driver_UDMA]
#define DESC_MAX_LEN        0x400
#define RTE_UDMA0           1
#define UDMA0_IRQHandler    IRQ033_Handler
#define CHNL_MASK_REQ0      0
#define CHNL_PRIORITY0      0
#define DMA_PERI_ACK0       0
#define BURST_REQ0_EN       1
#define UDMA0_CHNL_PRIO_LVL 1
#define UDMA0_SRAM_BASE     0x1FC00

#define RTE_UDMA1           1
#define UDMA1_IRQHandler    IRQ010_Handler
#define CHNL_MASK_REQ1      0
#define CHNL_PRIORITY1      0
#define BURST_REQ1_EN       1
#define CHNL_HIGH_PRIO_EN1  1
#define UDMA1_CHNL_PRIO_LVL 1
#define ULP_SRAM_START_ADDR 0x24060000
#define ULP_SRAM_END_ADDR   0x24063E00
//   <o> RTE_UDMA1_BASE_MEM <0=>PS2 <1=>PS4
#define RTE_UDMA1_BASE_MEM 0
#if (RTE_UDMA1_BASE_MEM == 0)
#define UDMA1_SRAM_BASE 0x24061C00
#elif (RTE_UDMA1_BASE_MEM == 1)
#define UDMA1_SRAM_BASE 0x1CC00
#else
#error "Invalid UDMA1 Control Base Address!"
#endif
// </e>

// <e> I2S0  [Driver_I2S0]
// <i> Configuration settings for Driver_I2S0 in component ::CMSIS Driver:I2S

#define RTE_I2S0        1
#define I2S0_IRQHandler IRQ064_Handler
/*I2S0 PINS*/

//     <o> I2S0_SCLK <0=>P0_8 <1=>P0_25 <2=>P0_46 <3=>P0_52
//     <i> SCLK of I2S0
#define RTE_I2S0_SCLK_PORT_ID 1

#if (RTE_I2S0_SCLK_PORT_ID == 0)
#define RTE_I2S0_SCLK_PORT 0
#define RTE_I2S0_SCLK_PIN  8
#define RTE_I2S0_SCLK_MUX  7
#define RTE_I2S0_SCLK_PAD  3
#elif (RTE_I2S0_SCLK_PORT_ID == 1)
#define RTE_I2S0_SCLK_PORT 0
#define RTE_I2S0_SCLK_PIN  25
#define RTE_I2S0_SCLK_MUX  7
#define RTE_I2S0_SCLK_PAD  0 //no pad
#elif (RTE_I2S0_SCLK_PORT_ID == 2)
#define RTE_I2S0_SCLK_PORT 0
#define RTE_I2S0_SCLK_PIN  46
#define RTE_I2S0_SCLK_MUX  7
#define RTE_I2S0_SCLK_PAD  10
#elif (RTE_I2S0_SCLK_PORT_ID == 3)
#define RTE_I2S0_SCLK_PORT 0
#define RTE_I2S0_SCLK_PIN  52
#define RTE_I2S0_SCLK_MUX  7
#define RTE_I2S0_SCLK_PAD  16
#else
#error "Invalid I2S0 RTE_I2S0_SCLK Pin Configuration!"
#endif

//     <o> I2S0_WSCLK <0=>P0_9 <1=>P0_26  <2=>P0_47  <3=>P0_53
//     <i> WSCLK for I2S0
#define RTE_I2S0_WSCLK_PORT_ID 1

#if (RTE_I2S0_WSCLK_PORT_ID == 0)
#define RTE_I2S0_WSCLK_PORT 0
#define RTE_I2S0_WSCLK_PIN  9
#define RTE_I2S0_WSCLK_MUX  7
#define RTE_I2S0_WSCLK_PAD  4
#elif (RTE_I2S0_WSCLK_PORT_ID == 1)
#define RTE_I2S0_WSCLK_PORT 0
#define RTE_I2S0_WSCLK_PIN  26
#define RTE_I2S0_WSCLK_MUX  7
#define RTE_I2S0_WSCLK_PAD  0 //no pad
#elif (RTE_I2S0_WSCLK_PORT_ID == 2)
#define RTE_I2S0_WSCLK_PORT 0
#define RTE_I2S0_WSCLK_PIN  47
#define RTE_I2S0_WSCLK_MUX  7
#define RTE_I2S0_WSCLK_PAD  11
#elif (RTE_I2S0_WSCLK_PORT_ID == 3)
#define RTE_I2S0_WSCLK_PORT 0
#define RTE_I2S0_WSCLK_PIN  53
#define RTE_I2S0_WSCLK_MUX  7
#define RTE_I2S0_WSCLK_PAD  17
#else
#error "Invalid I2S0 RTE_I2S0_WSCLK Pin Configuration!"
#endif

//     <o> I2S0_DOUT0 <0=>P0_11 <1=>P0_28  <2=>P0_49  <3=>P0_57
//     <i> DOUT0 for I2S0
#define RTE_I2S0_DOUT0_PORT_ID 1

#if (RTE_I2S0_DOUT0_PORT_ID == 0)
#define RTE_I2S0_DOUT0_PORT 0
#define RTE_I2S0_DOUT0_PIN  11
#define RTE_I2S0_DOUT0_MUX  7
#define RTE_I2S0_DOUT0_PAD  6
#elif (RTE_I2S0_DOUT0_PORT_ID == 1)
#define RTE_I2S0_DOUT0_PORT 0
#define RTE_I2S0_DOUT0_PIN  28
#define RTE_I2S0_DOUT0_MUX  7
#define RTE_I2S0_DOUT0_PAD  0 // no pad
#elif (RTE_I2S0_DOUT0_PORT_ID == 2)
#define RTE_I2S0_DOUT0_PORT 0
#define RTE_I2S0_DOUT0_PIN  49
#define RTE_I2S0_DOUT0_MUX  7
#define RTE_I2S0_DOUT0_PAD  13
#elif (RTE_I2S0_DOUT0_PORT_ID == 3)
#define RTE_I2S0_DOUT0_PORT 0
#define RTE_I2S0_DOUT0_PIN  57
#define RTE_I2S0_DOUT0_MUX  7
#define RTE_I2S0_DOUT0_PAD  21
#else
#error "Invalid I2S0 RTE_I2S0_DOUT0 Pin Configuration!"
#endif

//     <o> I2S0_DIN0 <0=>P0_10 <1=>P0_27  <2=>P0_48  <3=>P0_56
//     <i> DIN0 for I2S0
#define RTE_I2S0_DIN0_PORT_ID 1

#if (RTE_I2S0_DIN0_PORT_ID == 0)
#define RTE_I2S0_DIN0_PORT 0
#define RTE_I2S0_DIN0_PIN  10
#define RTE_I2S0_DIN0_MUX  7
#define RTE_I2S0_DIN0_PAD  5
#elif (RTE_I2S0_DIN0_PORT_ID == 1)
#define RTE_I2S0_DIN0_PORT 0
#define RTE_I2S0_DIN0_PIN  27
#define RTE_I2S0_DIN0_MUX  7
#define RTE_I2S0_DIN0_PAD  0 // no pad
#elif (RTE_I2S0_DIN0_PORT_ID == 2)
#define RTE_I2S0_DIN0_PORT 0
#define RTE_I2S0_DIN0_PIN  48
#define RTE_I2S0_DIN0_MUX  7
#define RTE_I2S0_DIN0_PAD  12
#elif (RTE_I2S0_DIN0_PORT_ID == 3)
#define RTE_I2S0_DIN0_PORT 0
#define RTE_I2S0_DIN0_PIN  56
#define RTE_I2S0_DIN0_MUX  7
#define RTE_I2S0_DIN0_PAD  20
#else
#error "Invalid I2S0 RTE_I2S0_DIN0 Pin Configuration!"
#endif

//     <o> I2S0_DOUT1 <0=>P0_7 <1=>P0_30 <2=>P0_51 <3=>P0_55
//     <i> DOUT1 for I2S0

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_I2S0_DOUT1_PORT_ID 1
#else
#define RTE_I2S0_DOUT1_PORT_ID 0
#endif

#if (RTE_I2S0_DOUT1_PORT_ID == 0)
#define RTE_I2S0_DOUT1_PORT 0
#define RTE_I2S0_DOUT1_PIN  7
#define RTE_I2S0_DOUT1_MUX  7
#define RTE_I2S0_DOUT1_PAD  2
#elif (RTE_I2S0_DOUT1_PORT_ID == 1)
#define RTE_I2S0_DOUT1_PORT 0
#define RTE_I2S0_DOUT1_PIN  30
#define RTE_I2S0_DOUT1_MUX  7
#define RTE_I2S0_DOUT1_PAD  0 //no pad
#elif (RTE_I2S0_DOUT1_PORT_ID == 2)
#define RTE_I2S0_DOUT1_PORT 0
#define RTE_I2S0_DOUT1_PIN  51
#define RTE_I2S0_DOUT1_MUX  7
#define RTE_I2S0_DOUT1_PAD  15
#elif (RTE_I2S0_DOUT1_PORT_ID == 3)
#define RTE_I2S0_DOUT1_PORT 0
#define RTE_I2S0_DOUT1_PIN  55
#define RTE_I2S0_DOUT1_MUX  7
#define RTE_I2S0_DOUT1_PAD  19
#else
#error "Invalid I2S0 RTE_I2S0_DOUT1 Pin Configuration!"
#endif

//     <o> I2S0_DIN1 <0=>P0_6 <1=>P0_29 <2=>P0_50 <3=>P0_54
//     <i> DIN1 for I2S0
#define RTE_I2S0_DIN1_PORT_ID 0

#if (RTE_I2S0_DIN1_PORT_ID == 0)
#define RTE_I2S0_DIN1_PORT 0
#define RTE_I2S0_DIN1_PIN  6
#define RTE_I2S0_DIN1_MUX  7
#define RTE_I2S0_DIN1_PAD  1
#elif (RTE_I2S0_DIN1_PORT_ID == 1)
#define RTE_I2S0_DIN1_PORT 0
#define RTE_I2S0_DIN1_PIN  29
#define RTE_I2S0_DIN1_MUX  7
#define RTE_I2S0_DIN1_PAD  0 //no pad
#elif (RTE_I2S0_DIN1_PORT_ID == 2)
#define RTE_I2S0_DIN1_PORT 0
#define RTE_I2S0_DIN1_PIN  50
#define RTE_I2S0_DIN1_MUX  7
#define RTE_I2S0_DIN1_PAD  14
#elif (RTE_I2S0_DIN1_PORT_ID == 3)
#define RTE_I2S0_DIN1_PORT 0
#define RTE_I2S0_DIN1_PIN  54
#define RTE_I2S0_DIN1_MUX  7
#define RTE_I2S0_DIN1_PAD  18
#else
#error "Invalid I2S0 RTE_I2S0_DIN1 Pin Configuration!"
#endif
// FIFO level can have value 1 to 7
#define I2S0_TX_FIFO_LEVEL (2U)
#define I2S0_RX_FIFO_LEVEL (2U)

//   <o> I2S0_TX_RES         <0=>12
//                           <1=>16
//                           <2=>20
//                           <3=>24
#define RTE_I2S0_TX_RES 1
#if (RTE_I2S0_TX_RES == 0)
#define I2S0_TX_RES RES_12_BIT
#elif (RTE_I2S0_TX_RES == 1)
#define I2S0_TX_RES RES_16_BIT
#elif (RTE_I2S0_TX_RES == 2)
#define I2S0_TX_RES RES_20_BIT
#elif (RTE_I2S0_TX_RES == 3)
#define I2S0_TX_RES RES_24_BIT
#else
#error "Invalid I2S0 TX channel resolution!"
#endif

//   <o> I2S0_RX_RES         <0=>12
//                           <1=>16
//                           <2=>20
//                           <3=>24
#define RTE_I2S0_RX_RES 1
#if (RTE_I2S0_RX_RES == 0)
#define I2S0_RX_RES RES_12_BIT
#elif (RTE_I2S0_RX_RES == 1)
#define I2S0_RX_RES RES_16_BIT
#elif (RTE_I2S0_RX_RES == 2)
#define I2S0_RX_RES RES_20_BIT
#elif (RTE_I2S0_RX_RES == 3)
#define I2S0_RX_RES RES_24_BIT
#else
#error "Invalid I2S0 RX channel resolution!"
#endif

#define RTE_I2S0_CHNL_UDMA_TX_EN 1
#define RTE_I2S0_CHNL_UDMA_TX_CH 15

#define RTE_I2S0_CHNL_UDMA_RX_EN 1
#define RTE_I2S0_CHNL_UDMA_RX_CH 14

#define RTE_I2S0_CLK_SRC      0
#define RTE_I2S0_CLK_DIV_FACT 0
#define RTE_I2S_PLL_FREQ      2000

#define RTE_I2S0_DMA_TX_LEN_PER_DES 1024
#define RTE_I2S0_DMA_RX_LEN_PER_DES 1024

// </e>

// <e> I2S1  [Driver_I2S1]
// <i> Configuration settings for Driver_I2S1 in component ::Drivers:I2S
#define RTE_I2S1        1
#define I2S1_IRQHandler IRQ014_Handler

//   <o> I2S1_SCLK Pin <0=>P0_3 <1=>P0_7 <2=>P0_8
/*I2S1 PINS*/
#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_I2S1_SCLK_PORT_ID 0
#else
#define RTE_I2S1_SCLK_PORT_ID 2
#endif
#if (RTE_I2S1_SCLK_PORT_ID == 0)
#define RTE_I2S1_SCLK_PORT 0
#define RTE_I2S1_SCLK_PIN  3
#define RTE_I2S1_SCLK_MUX  2
#elif (RTE_I2S1_SCLK_PORT_ID == 1)
#define RTE_I2S1_SCLK_PORT 0
#define RTE_I2S1_SCLK_PIN  7
#define RTE_I2S1_SCLK_MUX  2
#elif (RTE_I2S1_SCLK_PORT_ID == 2)
#define RTE_I2S1_SCLK_PORT 0
#define RTE_I2S1_SCLK_PIN  8
#define RTE_I2S1_SCLK_MUX  2
#else
#error "Invalid I2S1 RTE_I2S1_SCLK Pin Configuration!"
#endif

//   <o> I2S1_WSCLK Pin <0=>P0_2 <1=>P0_4 <2=>P0_10
#define RTE_I2S1_WSCLK_PORT_ID 0
#if (RTE_I2S1_WSCLK_PORT_ID == 0)
#define RTE_I2S1_WSCLK_PORT 0
#define RTE_I2S1_WSCLK_PIN  2
#define RTE_I2S1_WSCLK_MUX  2
#elif (RTE_I2S1_WSCLK_PORT_ID == 1)
#define RTE_I2S1_WSCLK_PORT 0
#define RTE_I2S1_WSCLK_PIN  10
#define RTE_I2S1_WSCLK_MUX  2
#else
#error "Invalid I2S1 RTE_I2S1_WSCLK Pin Configuration!"
#endif

//   <o> I2S1_DOUT0 Pin <0=>P0_1 <1=>P0_5 <2=>P0_11
#define RTE_I2S1_DOUT0_PORT_ID 0
#if (RTE_I2S1_DOUT0_PORT_ID == 0)
#define RTE_I2S1_DOUT0_PORT 0
#define RTE_I2S1_DOUT0_PIN  1
#define RTE_I2S1_DOUT0_MUX  2
#elif (RTE_I2S1_DOUT0_PORT_ID == 1)
#define RTE_I2S1_DOUT0_PORT 0
#define RTE_I2S1_DOUT0_PIN  11
#define RTE_I2S1_DOUT0_MUX  2
#else
#error "Invalid I2S1 RTE_I2S1_DOUT0 Pin Configuration!"
#endif

//   <o> I2S1_DIN0 Pin <0=>P0_0 <1=>P0_6 <2=>P0_9 <3=>P0_13
#define RTE_I2S1_DIN0_PORT_ID 1
#if (RTE_I2S1_DIN0_PORT_ID == 0)
#define RTE_I2S1_DIN0_PORT 0
#define RTE_I2S1_DIN0_PIN  0
#define RTE_I2S1_DIN0_MUX  2
#elif (RTE_I2S1_DIN0_PORT_ID == 1)
#define RTE_I2S1_DIN0_PORT 0
#define RTE_I2S1_DIN0_PIN  6
#define RTE_I2S1_DIN0_MUX  2
#elif (RTE_I2S1_DIN0_PORT_ID == 2)
#define RTE_I2S1_DIN0_PORT 0
#define RTE_I2S1_DIN0_PIN  9
#define RTE_I2S1_DIN0_MUX  2
#else
#error "Invalid I2S1 RTE_I2S1_DIN0 Pin Configuration!"
#endif

// FIFO level can have value 1 to 7
#define I2S1_TX_FIFO_LEVEL (2U)
#define I2S1_RX_FIFO_LEVEL (2U)

//   <o> I2S1_TX_RES         <0=>12
//                           <1=>16
//                           <2=>20
//                           <3=>24
#define RTE_I2S1_TX_RES 1
#if (RTE_I2S1_TX_RES == 0)
#define I2S1_TX_RES RES_12_BIT
#elif (RTE_I2S1_TX_RES == 1)
#define I2S1_TX_RES RES_16_BIT
#elif (RTE_I2S1_TX_RES == 2)
#define I2S1_TX_RES RES_20_BIT
#elif (RTE_I2S1_TX_RES == 3)
#define I2S1_TX_RES RES_24_BIT
#else
#error "Invalid I2S1 TX channel resolution!"
#endif

//   <o> I2S1_RX_RES         <0=>12
//                           <1=>16
//                           <2=>20
//                           <3=>24
#define RTE_I2S1_RX_RES 1
#if (RTE_I2S1_RX_RES == 0)
#define I2S1_RX_RES RES_12_BIT
#elif (RTE_I2S1_RX_RES == 1)
#define I2S1_RX_RES RES_16_BIT
#elif (RTE_I2S1_RX_RES == 2)
#define I2S1_RX_RES RES_20_BIT
#elif (RTE_I2S1_RX_RES == 3)
#define I2S1_RX_RES RES_24_BIT
#else
#error "Invalid I2S1 RX channel resolution!"
#endif

#define RTE_I2S1_CHNL_UDMA_TX_EN 1
#define RTE_I2S1_CHNL_UDMA_TX_CH 7

#define RTE_I2S1_CHNL_UDMA_RX_EN 1
#define RTE_I2S1_CHNL_UDMA_RX_CH 6

#define I2S1_CLK_DIV_FACT 0
//   <o> I2S1_CLK_SRC        <0=>ULP_I2S_REF_CLK
//                           <1=>ULP_I2S_ULP_32KHZ_RO_CLK
//                           <2=>ULP_I2S_ULP_32KHZ_RC_CLK
//                           <3=>ULP_I2S_ULP_32KHZ_XTAL_CLK
//                           <4=>ULP_I2S_ULP_32MHZ_RC_CLK
//                           <5=>ULP_I2S_ULP_20MHZ_RO_CLK
//                           <6=>ULP_I2S_SOC_CLK
//                           <7=>ULP_I2S_ULP_DOUBLER_CLK
//                           <8=>ULP_I2S_PLL_CLK

#define RTE_I2S1_CLK_SEL_ID 5
#if (RTE_I2S1_CLK_SEL_ID == 0)
#define RTE_I2S1_CLK_SRC ULP_I2S_REF_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 1)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_32KHZ_RO_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 2)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_32KHZ_RC_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 3)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_32KHZ_XTAL_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 4)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_32MHZ_RC_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 5)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_20MHZ_RO_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 6)
#define RTE_I2S1_CLK_SRC ULP_I2S_SOC_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 7)
#define RTE_I2S1_CLK_SRC ULP_I2S_ULP_DOUBLER_CLK
#elif (RTE_I2S1_CLK_SEL_ID == 8)
#define RTE_I2S1_CLK_SRC ULP_I2S_PLL_CLK
#else
#error "Invalid I2S1 Clock source selection!"
#endif
#define RTE_I2S1_DMA_TX_LEN_PER_DES 1024
#define RTE_I2S1_DMA_RX_LEN_PER_DES 1024

// </e> I2S1 [Driver_I2S1]

// <e> I2C0 (Inter-integrated Circuit Interface 0) [Driver_I2C0]
// <i> Configuration settings for Driver_I2C0 in component ::Drivers:I2C

#define RTE_I2C0        1
#define I2C0_IRQHandler IRQ042_Handler

//   <o> I2C0_SCL Pin <0=>P0_7 <1=>P0_65 <2=>P0_66 <3=>P0_75  <4=>P0_32

#define RTE_I2C0_SCL_PORT_ID 1

#if (RTE_I2C0_SCL_PORT_ID == 0)
#define RTE_I2C0_SCL_PORT    0
#define RTE_I2C0_SCL_PIN     7
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     2
#define RTE_I2C0_SCL_I2C_REN 7
#elif (RTE_I2C0_SCL_PORT_ID == 1)
#define RTE_I2C0_SCL_PORT    0
#define RTE_I2C0_SCL_PIN     65
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     23
#define RTE_I2C0_SCL_I2C_REN 1
#elif (RTE_I2C0_SCL_PORT_ID == 2)
#define RTE_I2C0_SCL_PORT    0
#define RTE_I2C0_SCL_PIN     66
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     24
#define RTE_I2C0_SCL_I2C_REN 2
#elif (RTE_I2C0_SCL_PORT_ID == 3)
#define RTE_I2C0_SCL_PORT    0
#define RTE_I2C0_SCL_PIN     75
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     33
#define RTE_I2C0_SCL_I2C_REN 11
#else
#error "Invalid I2C0 RTE_I2C0_SCL Pin Configuration!"
#endif

//   <o> I2C0_SCL Pin <0=>P0_6 <1=>P0_64 <2=>P0_67 <3=>P0_74 <4=>P0_31

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_I2C0_SDA_PORT_ID 2
#else
#define RTE_I2C0_SDA_PORT_ID 0
#endif

#if (RTE_I2C0_SDA_PORT_ID == 0)
#define RTE_I2C0_SDA_PORT    0
#define RTE_I2C0_SDA_PIN     6
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     1
#define RTE_I2C0_SDA_I2C_REN 6
#elif (RTE_I2C0_SDA_PORT_ID == 1)
#define RTE_I2C0_SDA_PORT    0
#define RTE_I2C0_SDA_PIN     67
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     25
#define RTE_I2C0_SDA_I2C_REN 3
#elif (RTE_I2C0_SDA_PORT_ID == 2)
#define RTE_I2C0_SDA_PORT    0
#define RTE_I2C0_SDA_PIN     74
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     32
#define RTE_I2C0_SDA_I2C_REN 10
#else
#error "Invalid I2C0 RTE_I2C0_SDA Pin Configuration!"
#endif

#define IC_SCL_STUCK_TIMEOUT 20
#define IC_SDA_STUCK_TIMEOUT 20

#define I2C_DMA 0
#if (I2C_DMA == 1)
#define DMA_TX_TL 1
#define DMA_RX_TL 1
#endif
// </e> I2C0 [Driver_I2C0]

// <e> I2C1 (Inter-integrated Circuit Interface 1) [Driver_I2C1]
// <i> Configuration settings for Driver_I2C1 in component ::Drivers:I2C

#define RTE_I2C1        1
#define I2C1_IRQHandler IRQ061_Handler
//   <o> I2C1_SCL Pin <0=>P0_6 <1=>P0_29  <2=>P0_50 <3=>P0_54 <4=>P0_64 <4=>P0_66 <4=>P0_70 <7=>P0_33
#define RTE_I2C1_SCL_PORT_ID 2

#if (RTE_I2C1_SCL_PORT_ID == 0)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  6
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  1
#define RTE_I2C1_SCL_REN  6
#elif (RTE_I2C1_SCL_PORT_ID == 1)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  29
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  0 //no pad
#define RTE_I2C1_SCL_REN  29
#elif (RTE_I2C1_SCL_PORT_ID == 2)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  50
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  14
#define RTE_I2C1_SCL_REN  50
#elif (RTE_I2C1_SCL_PORT_ID == 3)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  54
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  18
#define RTE_I2C1_SCL_REN  54
#elif (RTE_I2C1_SCL_PORT_ID == 5)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  66
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  24
#define RTE_I2C1_SCL_REN  2
#elif (RTE_I2C1_SCL_PORT_ID == 6)
#define RTE_I2C1_SCL_PORT 0
#define RTE_I2C1_SCL_PIN  70
#define RTE_I2C1_SCL_MUX  5
#define RTE_I2C1_SCL_PAD  29
#define RTE_I2C1_SCL_REN  6
#else
#error "Invalid I2C1_SCL Pin Configuration!"
#endif

// <o> I2C1_SCL Pin <0=>P0_7 <1=>P0_30 <2=>P0_51 <3=>P0_55  <4=>P0_65 <4=>P0_67 <4=>P0_71 <7=>P0_34

#define RTE_I2C1_SDA_PORT_ID 2

#if (RTE_I2C1_SDA_PORT_ID == 0)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  7
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  2
#define RTE_I2C1_SDA_REN  7
#elif (RTE_I2C1_SDA_PORT_ID == 1)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  30
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  0 //no pad
#define RTE_I2C1_SDA_REN  30
#elif (RTE_I2C1_SDA_PORT_ID == 2)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  51
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  15
#define RTE_I2C1_SDA_REN  51
#elif (RTE_I2C1_SDA_PORT_ID == 3)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  55
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  19
#define RTE_I2C1_SDA_REN  55
#elif (RTE_I2C1_SDA_PORT_ID == 4)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  65
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  23
#define RTE_I2C1_SDA_REN  1
#elif (RTE_I2C1_SDA_PORT_ID == 5)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  67
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  25
#define RTE_I2C1_SDA_REN  3
#elif (RTE_I2C1_SDA_PORT_ID == 6)
#define RTE_I2C1_SDA_PORT 0
#define RTE_I2C1_SDA_PIN  71
#define RTE_I2C1_SDA_MUX  5
#define RTE_I2C1_SDA_PAD  29
#define RTE_I2C1_SDA_REN  7
#else
#error "Invalid I2C1_SDA Pin Configuration!"
#endif

#define IC_SCL_STUCK_TIMEOUT 20
#define IC_SDA_STUCK_TIMEOUT 20

#define DMA_EN 0
#if (DMA_EN == 1)
#define DMA_TX_TL 1
#define DMA_RX_TL 1
#endif

// </e> I2C1 [Driver_I2C1]

// <e> I2C2 (Inter-integrated Circuit Interface 2) [Driver_I2C2]
// <i> Configuration settings for Driver_I2C2 in component ::Drivers:I2C
#define RTE_I2C2        1
#define I2C2_IRQHandler IRQ013_Handler

// <o> I2C2_SCL Pin <0=>P0_1 <1=>P0_5 <2=>P0_7 <3=>P0_8 <4=>P0_13
#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_I2C2_SCL_PORT_ID 0
#else
#define RTE_I2C2_SCL_PORT_ID 0
#endif
#if (RTE_I2C2_SCL_PORT_ID == 0)
#define RTE_I2C2_SCL_PORT 0
#define RTE_I2C2_SCL_PIN  7
#define RTE_I2C2_SCL_MUX  4
#define RTE_I2C2_SCL_REN  7
#elif (RTE_I2C2_SCL_PORT_ID == 1)
#define RTE_I2C2_SCL_PORT 0
#define RTE_I2C2_SCL_PIN  8
#define RTE_I2C2_SCL_MUX  4
#define RTE_I2C2_SCL_REN  8
#else
#error "Invalid I2C2_SCL Pin Configuration!"
#endif

// <o> I2C2_SDA Pin <0=>P0_0 <1=>P0_4 <2=>P0_6 <3=>P0_9 <4=>P0_11 <5=>P0_12
#define RTE_I2C2_SDA_PORT_ID 0
#if (RTE_I2C2_SDA_PORT_ID == 0)
#define RTE_I2C2_SDA_PORT 0
#define RTE_I2C2_SDA_PIN  6
#define RTE_I2C2_SDA_MUX  4
#define RTE_I2C2_SDA_REN  6
#elif (RTE_I2C2_SDA_PORT_ID == 1)
#define RTE_I2C2_SDA_PORT    0
#define RTE_I2C2_SDA_PIN     9
#define RTE_I2C2_SDA_MUX     4
#define RTE_I2C2_SDA_I2C_REN 9
#elif (RTE_I2C2_SDA_PORT_ID == 2)
#define RTE_I2C2_SDA_PORT 0
#define RTE_I2C2_SDA_PIN  11
#define RTE_I2C2_SDA_MUX  4
#define RTE_I2C2_SDA_REN  11
#else
#error "Invalid I2C2_SDA Pin Configuration!"
#endif

#define IC_SCL_STUCK_TIMEOUT 20
#define IC_SDA_STUCK_TIMEOUT 20

#define DMA_EN 0
#if (DMA_EN == 1)
#define DMA_TX_TL 1
#define DMA_RX_TL 1
#endif

// </e> I2C2 [Driver_I2C2]

// <e> GSPI (Generic SPI master) [Driver_GSPI_MASTER]
// <i> Configuration settings for Driver_GSPI_MASTER in component ::Drivers:GSPI
#define RTE_GSPI_MASTER 1

//     <o> GSPI_MASTER_CLK <0=>P0_8 <1=>P0_25 <2=>P0_46 <3=>P0_52
//     <i> CLK of GSPI0
#define RTE_GSPI_MASTER_CLK_PORT_ID 1

#if (RTE_GSPI_MASTER_CLK_PORT_ID == 0)
#define RTE_GSPI_MASTER_CLK_PORT 0
#define RTE_GSPI_MASTER_CLK_PIN  8
#define RTE_GSPI_MASTER_CLK_MUX  4
#define RTE_GSPI_MASTER_CLK_PAD  3
#elif (RTE_GSPI_MASTER_CLK_PORT_ID == 1)
#define RTE_GSPI_MASTER_CLK_PORT 0
#define RTE_GSPI_MASTER_CLK_PIN  25
#define RTE_GSPI_MASTER_CLK_MUX  4
#define RTE_GSPI_MASTER_CLK_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_CLK_PORT_ID == 2)
#define RTE_GSPI_MASTER_CLK_PORT 0
#define RTE_GSPI_MASTER_CLK_PIN  46
#define RTE_GSPI_MASTER_CLK_MUX  4
#define RTE_GSPI_MASTER_CLK_PAD  10
#elif (RTE_GSPI_MASTER_CLK_PORT_ID == 3)
#define RTE_GSPI_MASTER_CLK_PORT 0
#define RTE_GSPI_MASTER_CLK_PIN  52
#define RTE_GSPI_MASTER_CLK_MUX  4
#define RTE_GSPI_MASTER_CLK_PAD  16
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_CLK_PIN Pin Configuration!"
#endif

//   <e> GSPI_MASTER_CS0
//   <o>  <0=>P0_9 <1=>P0_28 <2=>P0_49 <3=>P0_53
//   <i> CS0 of GSPI0
//   </e>

#define RTE_GSPI_MASTER_CS0_PORT_ID 2

#if (RTE_GSPI_MASTER_CS0_PORT_ID == 0)
#define RTE_GSPI_MASTER_CS0      1
#define RTE_GSPI_MASTER_CS0_PORT 0
#define RTE_GSPI_MASTER_CS0_PIN  9
#define RTE_GSPI_MASTER_CS0_MUX  4
#define RTE_GSPI_MASTER_CS0_PAD  4
#elif (RTE_GSPI_MASTER_CS0_PORT_ID == 1)
#define RTE_GSPI_MASTER_CS0      1
#define RTE_GSPI_MASTER_CS0_PORT 0
#define RTE_GSPI_MASTER_CS0_PIN  28
#define RTE_GSPI_MASTER_CS0_MUX  4
#define RTE_GSPI_MASTER_CS0_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_CS0_PORT_ID == 2)
#define RTE_GSPI_MASTER_CS0      1
#define RTE_GSPI_MASTER_CS0_PORT 0
#define RTE_GSPI_MASTER_CS0_PIN  49
#define RTE_GSPI_MASTER_CS0_MUX  4
#define RTE_GSPI_MASTER_CS0_PAD  13
#elif (RTE_GSPI_MASTER_CS0_PORT_ID == 3)
#define RTE_GSPI_MASTER_CS0      1
#define RTE_GSPI_MASTER_CS0_PORT 0
#define RTE_GSPI_MASTER_CS0_PIN  53
#define RTE_GSPI_MASTER_CS0_MUX  4
#define RTE_GSPI_MASTER_CS0_PAD  17
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_CS0_PIN Pin Configuration!"
#endif

//   <e> GSPI_MASTER_CS1
//   <o>  <0=>P0_10 <1=>P0_29 <2=>P0_50 <3=>P0_54
//   <i> CS1 of GSPI0
//   </e>
#define RTE_GSPI_MASTER_CS1_PORT_ID 2
#if (RTE_GSPI_MASTER_CS1_PORT_ID == 0)
#define RTE_GSPI_MASTER_CS1      1
#define RTE_GSPI_MASTER_CS1_PORT 0
#define RTE_GSPI_MASTER_CS1_PIN  10
#define RTE_GSPI_MASTER_CS1_MUX  4
#define RTE_GSPI_MASTER_CS1_PAD  5
#elif (RTE_GSPI_MASTER_CS1_PORT_ID == 1)
#define RTE_GSPI_MASTER_CS1      1
#define RTE_GSPI_MASTER_CS1_PORT 0
#define RTE_GSPI_MASTER_CS1_PIN  29
#define RTE_GSPI_MASTER_CS1_MUX  4
#define RTE_GSPI_MASTER_CS1_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_CS1_PORT_ID == 2)
#define RTE_GSPI_MASTER_CS1      1
#define RTE_GSPI_MASTER_CS1_PORT 0
#define RTE_GSPI_MASTER_CS1_PIN  50
#define RTE_GSPI_MASTER_CS1_MUX  4
#define RTE_GSPI_MASTER_CS1_PAD  14
#elif (RTE_GSPI_MASTER_CS1_PORT_ID == 3)
#define RTE_GSPI_MASTER_CS1      1
#define RTE_GSPI_MASTER_CS1_PORT 0
#define RTE_GSPI_MASTER_CS1_PIN  54
#define RTE_GSPI_MASTER_CS1_MUX  4
#define RTE_GSPI_MASTER_CS1_PAD  18
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_CS1_PIN Pin Configuration!"
#endif

//   <e> GSPI_MASTER_CS2
//   <o>  <0=>P0_15 <1=>P0_30 <2=>P0_51 <3=>P0_55
//   <i> CS2 of GSPI0
//   </e>
#define RTE_GSPI_MASTER_CS2_PORT_ID 1
#if (RTE_GSPI_MASTER_CS2_PORT_ID == 0)
#define RTE_GSPI_MASTER_CS2      1
#define RTE_GSPI_MASTER_CS2_PORT 0
#define RTE_GSPI_MASTER_CS2_PIN  15
#define RTE_GSPI_MASTER_CS2_MUX  4
#define RTE_GSPI_MASTER_CS2_PAD  8
#elif (RTE_GSPI_MASTER_CS2_PORT_ID == 1)
#define RTE_GSPI_MASTER_CS2      1
#define RTE_GSPI_MASTER_CS2_PORT 0
#define RTE_GSPI_MASTER_CS2_PIN  30
#define RTE_GSPI_MASTER_CS2_MUX  4
#define RTE_GSPI_MASTER_CS2_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_CS2_PORT_ID == 2)
#define RTE_GSPI_MASTER_CS2      1
#define RTE_GSPI_MASTER_CS2_PORT 0
#define RTE_GSPI_MASTER_CS2_PIN  51
#define RTE_GSPI_MASTER_CS2_MUX  4
#define RTE_GSPI_MASTER_CS2_PAD  15
#elif (RTE_GSPI_MASTER_CS2_PORT_ID == 3)
#define RTE_GSPI_MASTER_CS2      1
#define RTE_GSPI_MASTER_CS2_PORT 0
#define RTE_GSPI_MASTER_CS2_PIN  55
#define RTE_GSPI_MASTER_CS2_MUX  4
#define RTE_GSPI_MASTER_CS2_PAD  19
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_CS2_PIN Pin Configuration!"
#endif

//     <o> GSPI_MASTER_MOSI <0=>P0_12 <1=>P0_27 <2=>P0_48 <3=>P0_57 <4=>P0_6
//     <i> MOSI of GSPI0

#define RTE_GSPI_MASTER_MOSI_PORT_ID 1
#if ((RTE_GSPI_MASTER_MOSI_PORT_ID == 4))
#error "Invalid GSPI0 RTE_GSPI_MASTER_MOSI_PIN pin Configuration!"
#endif

#if (RTE_GSPI_MASTER_MOSI_PORT_ID == 0)
#define RTE_GSPI_MASTER_MOSI_PORT 0
#define RTE_GSPI_MASTER_MOSI_PIN  12
#define RTE_GSPI_MASTER_MOSI_MUX  4
#define RTE_GSPI_MASTER_MOSI_PAD  7
#elif (RTE_GSPI_MASTER_MOSI_PORT_ID == 1)
#define RTE_GSPI_MASTER_MOSI_PORT 0
#define RTE_GSPI_MASTER_MOSI_PIN  27
#define RTE_GSPI_MASTER_MOSI_MUX  4
#define RTE_GSPI_MASTER_MOSI_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_MOSI_PORT_ID == 2)
#define RTE_GSPI_MASTER_MOSI_PORT 0
#define RTE_GSPI_MASTER_MOSI_PIN  48
#define RTE_GSPI_MASTER_MOSI_MUX  4
#define RTE_GSPI_MASTER_MOSI_PAD  12
#elif (RTE_GSPI_MASTER_MOSI_PORT_ID == 3)
#define RTE_GSPI_MASTER_MOSI_PORT 0
#define RTE_GSPI_MASTER_MOSI_PIN  57
#define RTE_GSPI_MASTER_MOSI_MUX  4
#define RTE_GSPI_MASTER_MOSI_PAD  21
#elif (RTE_GSPI_MASTER_MOSI_PORT_ID == 4)
#define RTE_GSPI_MASTER_MOSI_PORT 0
#define RTE_GSPI_MASTER_MOSI_PIN  6
#define RTE_GSPI_MASTER_MOSI_MUX  12
#define RTE_GSPI_MASTER_MOSI_PAD  1
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_MOSI_PIN Pin Configuration!"
#endif

//     <o> GSPI_MASTER_MISO <0=>P0_11 <1=>P0_26 <2=>P0_47 <3=>P0_56
//     <i> MISO of GSPI0

#define RTE_GSPI_MASTER_MISO_PORT_ID 1

#if (RTE_GSPI_MASTER_MISO_PORT_ID == 0)
#define RTE_GSPI_MASTER_MISO_PORT 0
#define RTE_GSPI_MASTER_MISO_PIN  11
#define RTE_GSPI_MASTER_MISO_MUX  4
#define RTE_GSPI_MASTER_MISO_PAD  6
#elif (RTE_GSPI_MASTER_MISO_PORT_ID == 1)
#define RTE_GSPI_MASTER_MISO_PORT 0
#define RTE_GSPI_MASTER_MISO_PIN  26
#define RTE_GSPI_MASTER_MISO_MUX  4
#define RTE_GSPI_MASTER_MISO_PAD  0 //NO PAD
#elif (RTE_GSPI_MASTER_MISO_PORT_ID == 2)
#define RTE_GSPI_MASTER_MISO_PORT 0
#define RTE_GSPI_MASTER_MISO_PIN  47
#define RTE_GSPI_MASTER_MISO_MUX  4
#define RTE_GSPI_MASTER_MISO_PAD  11
#elif (RTE_GSPI_MASTER_MISO_PORT_ID == 3)
#define RTE_GSPI_MASTER_MISO_PORT 0
#define RTE_GSPI_MASTER_MISO_PIN  56
#define RTE_GSPI_MASTER_MISO_MUX  4
#define RTE_GSPI_MASTER_MISO_PAD  20
#else
#error "Invalid GSPI0 RTE_GSPI_MASTER_MISO_PIN Pin Configuration!"
#endif

#if HIGH_THROUGHPUT_EN
#define RTE_GSPI_MASTER_CHNL_UDMA_TX_EN 1
#define RTE_GSPI_MASTER_CHNL_UDMA_TX_CH 11

#define RTE_GSPI_MASTER_CHNL_UDMA_RX_EN 1
#define RTE_GSPI_MASTER_CHNL_UDMA_RX_CH 10

#define RTE_GSPI_CLOCK_SOURCE GSPI_SOC_PLL_CLK

#define RTE_FIFO_AFULL_THRLD  3
#define RTE_FIFO_AEMPTY_THRLD 7

#define TX_DMA_ARB_SIZE ARBSIZE_4
#define RX_DMA_ARB_SIZE ARBSIZE_8
#else
#define RTE_GSPI_MASTER_CHNL_UDMA_TX_EN 0
#define RTE_GSPI_MASTER_CHNL_UDMA_TX_CH 11

#define RTE_GSPI_MASTER_CHNL_UDMA_RX_EN 0
#define RTE_GSPI_MASTER_CHNL_UDMA_RX_CH 10

#define RTE_GSPI_CLOCK_SOURCE GSPI_SOC_PLL_CLK

#define RTE_FIFO_AFULL_THRLD  0
#define RTE_FIFO_AEMPTY_THRLD 0

#define TX_DMA_ARB_SIZE ARBSIZE_1
#define RX_DMA_ARB_SIZE ARBSIZE_1
#endif

// </e>(Generic SPI master)[Driver_GSPI_MASTER]

// <o>(State Configurable Timer) Interface
#define SCT_CLOCK_SOURCE   M4_SOCCLKFOROTHERCLKSCT
#define SCT_CLOCK_DIV_FACT 1

//SCT_IN_0  <0=>GPIO_25  <1=>GPIO_64 <2=>GPIO_68

#define RTE_SCT_IN_0_PORT_ID 0

#if (RTE_SCT_IN_0_PORT_ID == 0)
#define RTE_SCT_IN_0_PORT 0
#define RTE_SCT_IN_0_PIN  25
#define RTE_SCT_IN_0_MUX  9
#define RTE_SCT_IN_0_PAD  0 //no pad
#else
#error "Invalid  RTE_SCT_IN_0_PIN Pin Configuration!"
#endif

//SCT_IN_1  <0=>GPIO_26  <1=>GPIO_65 <2=>GPIO_69

#define RTE_SCT_IN_1_PORT_ID 1

#if (RTE_SCT_IN_1_PORT_ID == 0)
#define RTE_SCT_IN_1_PORT 0
#define RTE_SCT_IN_1_PIN  26
#define RTE_SCT_IN_1_MUX  9
#define RTE_SCT_IN_1_PAD  0 //no pad
#elif (RTE_SCT_IN_1_PORT_ID == 1)
#define RTE_SCT_IN_1_PORT 0
#define RTE_SCT_IN_1_PIN  65
#define RTE_SCT_IN_1_MUX  7
#define RTE_SCT_IN_1_PAD  23
#else
#error "Invalid  RTE_SCT_IN_1_PIN Pin Configuration!"
#endif

//SCT_IN_2  <0=>GPIO_27  <1=>GPIO_66 <2=>GPIO_70

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SCT_IN_2_PORT_ID 0
#else
#define RTE_SCT_IN_2_PORT_ID 1
#endif

#if (RTE_SCT_IN_2_PORT_ID == 0)
#define RTE_SCT_IN_2_PORT 0
#define RTE_SCT_IN_2_PIN  27
#define RTE_SCT_IN_2_MUX  9
#define RTE_SCT_IN_2_PAD  0 //no pad
#elif (RTE_SCT_IN_2_PORT_ID == 1)
#define RTE_SCT_IN_2_PORT 0
#define RTE_SCT_IN_2_PIN  66
#define RTE_SCT_IN_2_MUX  7
#define RTE_SCT_IN_2_PAD  24
#elif (RTE_SCT_IN_2_PORT_ID == 2)
#define RTE_SCT_IN_2_PORT 0
#define RTE_SCT_IN_2_PIN  70
#define RTE_SCT_IN_2_MUX  9
#define RTE_SCT_IN_2_PAD  28
#else
#error "Invalid  RTE_SCT_IN_0_PIN Pin Configuration!"
#endif

//SCT_IN_3  <0=>GPIO_28  <1=>GPIO_67 <2=>GPIO_71

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SCT_IN_3_PORT_ID 0
#else
#define RTE_SCT_IN_3_PORT_ID 1
#endif

#if (RTE_SCT_IN_3_PORT_ID == 0)
#define RTE_SCT_IN_3_PORT 0
#define RTE_SCT_IN_3_PIN  28
#define RTE_SCT_IN_3_MUX  9
#define RTE_SCT_IN_3_PAD  0 //no pad
#elif (RTE_SCT_IN_3_PORT_ID == 1)
#define RTE_SCT_IN_3_PORT 0
#define RTE_SCT_IN_3_PIN  67
#define RTE_SCT_IN_3_MUX  7
#define RTE_SCT_IN_3_PAD  25
#elif (RTE_SCT_IN_3_PORT_ID == 2)
#define RTE_SCT_IN_3_PORT 0
#define RTE_SCT_IN_3_PIN  71
#define RTE_SCT_IN_3_MUX  9
#define RTE_SCT_IN_3_PAD  29
#else
#error "Invalid  RTE_SCT_IN_0_PIN Pin Configuration!"
#endif

//     SCT_OUT_0 <0=>GPIO_29 <1=>GPIO_68
#define RTE_SCT_OUT_0_PORT_ID 0
#if (RTE_SCT_OUT_0_PORT_ID == 0)
#define RTE_SCT_OUT_0_PORT 0
#define RTE_SCT_OUT_0_PIN  29
#define RTE_SCT_OUT_0_MUX  9
#define RTE_SCT_OUT_0_PAD  0 //no pad
#else
#error "Invalid  RTE_SCT_OUT_0_PIN Pin Configuration!"
#endif

//  SCT_OUT_1 <0=>GPIO_30  <1=>GPIO_69
#define RTE_SCT_OUT_1_PORT_ID 0
#if (RTE_SCT_OUT_1_PORT_ID == 0)
#define RTE_SCT_OUT_1_PORT 0
#define RTE_SCT_OUT_1_PIN  30
#define RTE_SCT_OUT_1_MUX  9
#define RTE_SCT_OUT_1_PAD  0 //no pad
#else
#error "Invalid  RTE_SCT_OUT_1_PIN Pin Configuration!"
#endif

/// SCT_OUT_2 <0=>GPIO_70 <1=>GPIO_8
#define RTE_SCT_OUT_2_PORT_ID 0
#if ((RTE_SCT_OUT_2_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_2_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_2_PORT_ID == 0)
#define RTE_SCT_OUT_2_PORT 0
#define RTE_SCT_OUT_2_PIN  70
#define RTE_SCT_OUT_2_MUX  7
#define RTE_SCT_OUT_2_PAD  28
#elif (RTE_SCT_OUT_2_PORT_ID == 1)
#define RTE_SCT_OUT_2_PORT 0
#define RTE_SCT_OUT_2_PIN  8
#define RTE_SCT_OUT_2_MUX  12
#define RTE_SCT_OUT_2_PAD  3
#else
#error "Invalid  RTE_SCT_OUT_2_PIN Pin Configuration!"
#endif
/**/
//SCT_OUT_3 <0=>GPIO_71 <1=>GPIO_9
#define RTE_SCT_OUT_3_PORT_ID 0
#if ((RTE_SCT_OUT_3_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_3_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_3_PORT_ID == 0)
#define RTE_SCT_OUT_3_PORT 0
#define RTE_SCT_OUT_3_PIN  71
#define RTE_SCT_OUT_3_MUX  7
#define RTE_SCT_OUT_3_PAD  29
#elif (RTE_SCT_OUT_3_PORT_ID == 1)
#define RTE_SCT_OUT_3_PORT 0
#define RTE_SCT_OUT_3_PIN  9
#define RTE_SCT_OUT_3_MUX  12
#define RTE_SCT_OUT_3_PAD  4
#else
#error "Invalid  RTE_SCT_OUT_3_PIN Pin Configuration!"
#endif

//SCT_OUT_4 <0=>GPIO_72 <1=>GPIO_68

#define RTE_SCT_OUT_4_PORT_ID 0
#if ((RTE_SCT_OUT_4_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_4_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_4_PORT_ID == 0)
/**/
#define RTE_SCT_OUT_4_PORT 0
#define RTE_SCT_OUT_4_PIN  72
#define RTE_SCT_OUT_4_MUX  7
#define RTE_SCT_OUT_4_PAD  30
#else
#error "Invalid  RTE_SCT_OUT_4_PIN Pin Configuration!"
#endif
//SCT_OUT_5 <0=>GPIO_73 <1=>GPIO_69

#define RTE_SCT_OUT_5_PORT_ID 0
#if ((RTE_SCT_OUT_5_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_5_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_5_PORT_ID == 0)
#define RTE_SCT_OUT_5_PORT 2
#define RTE_SCT_OUT_5_PIN  73
#define RTE_SCT_OUT_5_MUX  7
#define RTE_SCT_OUT_5_PAD  31
#else
#error "Invalid  RTE_SCT_OUT_5_PIN Pin Configuration!"
#endif

//SCT_OUT_6 <0=>GPIO_74 <1=>GPIO_70

#define RTE_SCT_OUT_6_PORT_ID 0
#if ((RTE_SCT_OUT_6_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_6_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_6_PORT_ID == 0)
#define RTE_SCT_OUT_6_PORT 0
#define RTE_SCT_OUT_6_PIN  74
#define RTE_SCT_OUT_6_MUX  7
#define RTE_SCT_OUT_6_PAD  32
#elif (RTE_SCT_OUT_6_PORT_ID == 1)
#define RTE_SCT_OUT_6_PORT 0
#define RTE_SCT_OUT_6_PIN  70
#define RTE_SCT_OUT_6_MUX  13
#define RTE_SCT_OUT_6_PAD  28
#else
#error "Invalid  RTE_SCT_OUT_6_PIN Pin Configuration!"
#endif

// SCT_OUT_7  <0=>GPIO_75 <1=>GPIO_71

#define RTE_SCT_OUT_7_PORT_ID 0

#if (RTE_SCT_OUT_7_PORT_ID == 0)
#define RTE_SCT_OUT_7_PORT 0
#define RTE_SCT_OUT_7_PIN  75
#define RTE_SCT_OUT_7_MUX  7
#define RTE_SCT_OUT_7_PAD  33
#else
#error "Invalid  RTE_SCT_OUT_7_PIN Pin Configuration!"
#endif

//  SIO //
//<>  Serial Input Output
//SIO_0 <0=>GPIO_6 <1=>GPIO_25  <2=>GPIO_64 <3=>GPIO_72

#define RTE_SIO_0_PORT_ID 0

#if (RTE_SIO_0_PORT_ID == 0)
#define RTE_SIO_0_PORT 0
#define RTE_SIO_0_PIN  6
#define RTE_SIO_0_MUX  1
#define RTE_SIO_0_PAD  1
#elif (RTE_SIO_0_PORT_ID == 1)
#define RTE_SIO_0_PORT 0
#define RTE_SIO_0_PIN  25
#define RTE_SIO_0_MUX  1
#define RTE_SIO_0_PAD  0 //no pad
#elif (RTE_SIO_0_PORT_ID == 2)
#define RTE_SIO_0_PORT 0
#define RTE_SIO_0_PIN  72
#define RTE_SIO_0_MUX  1
#define RTE_SIO_0_PAD  30
#else
#error "Invalid  RTE_SIO_0_PIN Pin Configuration!"
#endif

//SIO_1 <0=>GPIO_7 <1=>GPIO_26  <2=>GPIO_65 <3=>GPIO_73

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SIO_1_PORT_ID 1
#else
#define RTE_SIO_1_PORT_ID 0
#endif

#if (RTE_SIO_1_PORT_ID == 0)
#define RTE_SIO_1_PORT 0
#define RTE_SIO_1_PIN  7
#define RTE_SIO_1_MUX  1
#define RTE_SIO_1_PAD  2
#elif (RTE_SIO_1_PORT_ID == 1)
#define RTE_SIO_1_PORT 0
#define RTE_SIO_1_PIN  26
#define RTE_SIO_1_MUX  1
#define RTE_SIO_1_PAD  0 // no pad
#elif (RTE_SIO_1_PORT_ID == 2)
#define RTE_SIO_1_PORT 0
#define RTE_SIO_1_PIN  65
#define RTE_SIO_1_MUX  1
#define RTE_SIO_1_PAD  23
#elif (RTE_SIO_1_PORT_ID == 3)
#define RTE_SIO_1_PORT 0
#define RTE_SIO_1_PIN  73
#define RTE_SIO_1_MUX  1
#define RTE_SIO_1_PAD  31
#else
#error "Invalid  RTE_SIO_1_PIN Pin Configuration!"
#endif

// SIO_2  <0=>GPIO_8 <1=>GPIO_27  <2=>GPIO_66 <3=>GPIO_74

#define RTE_SIO_2_PORT_ID 1

#if (RTE_SIO_2_PORT_ID == 0)
#define RTE_SIO_2_PORT 0
#define RTE_SIO_2_PIN  8
#define RTE_SIO_2_MUX  1
#define RTE_SIO_2_PAD  3
#elif (RTE_SIO_2_PORT_ID == 1)
#define RTE_SIO_2_PORT 0
#define RTE_SIO_2_PIN  27
#define RTE_SIO_2_MUX  1
#define RTE_SIO_2_PAD  0 //no pad
#elif (RTE_SIO_2_PORT_ID == 2)
#define RTE_SIO_2_PORT 0
#define RTE_SIO_2_PIN  66
#define RTE_SIO_2_MUX  1
#define RTE_SIO_2_PAD  24
#elif (RTE_SIO_2_PORT_ID == 3)
#define RTE_SIO_2_PORT 0
#define RTE_SIO_2_PIN  74
#define RTE_SIO_2_MUX  1
#define RTE_SIO_2_PAD  32
#else
#error "Invalid  RTE_SIO_2_PIN Pin Configuration!"
#endif

//SIO_3 <0=>GPIO_9 <1=>GPIO_28  <2=>GPIO_67 <3=>GPIO_75

#define RTE_SIO_3_PORT_ID 1

#if (RTE_SIO_3_PORT_ID == 0)
#define RTE_SIO_3_PORT 0
#define RTE_SIO_3_PIN  9
#define RTE_SIO_3_MUX  1
#define RTE_SIO_3_PAD  4
#elif (RTE_SIO_3_PORT_ID == 1)
#define RTE_SIO_3_PORT 0
#define RTE_SIO_3_PIN  28
#define RTE_SIO_3_MUX  1
#define RTE_SIO_3_PAD  0 //no pad
#elif (RTE_SIO_3_PORT_ID == 2)
#define RTE_SIO_3_PORT 0
#define RTE_SIO_3_PIN  67
#define RTE_SIO_3_MUX  1
#define RTE_SIO_3_PAD  25
#elif (RTE_SIO_3_PORT_ID == 3)
#define RTE_SIO_3_PORT 0
#define RTE_SIO_3_PIN  75
#define RTE_SIO_3_MUX  1
#define RTE_SIO_3_PAD  33
#else
#error "Invalid  RTE_SIO_3_PIN Pin Configuration!"
#endif

//SIO_4 <0=>GPIO_10 <1=>GPIO_29 <2=>GPIO_68
#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SIO_4_PORT_ID 1
#else
#define RTE_SIO_4_PORT_ID 0
#endif
#if (RTE_SIO_4_PORT_ID == 0)
#define RTE_SIO_4_PORT 0
#define RTE_SIO_4_PIN  10
#define RTE_SIO_4_MUX  1
#define RTE_SIO_4_PAD  5
#elif (RTE_SIO_4_PORT_ID == 1)
#define RTE_SIO_4_PORT 0
#define RTE_SIO_4_PIN  29
#define RTE_SIO_4_MUX  1
#define RTE_SIO_4_PAD  0 //NO PAD
#else
#error "Invalid  RTE_SIO_3_PIN Pin Configuration!"
#endif

// SIO_5  <0=>GPIO_11 <1=>GPIO_30 <2=>GPIO_69
#define RTE_SIO_5_PORT_ID 0
#if (RTE_SIO_5_PORT_ID == 0)
#define RTE_SIO_5_PORT 0
#define RTE_SIO_5_PIN  11
#define RTE_SIO_5_MUX  1
#define RTE_SIO_5_PAD  6
#elif (RTE_SIO_5_PORT_ID == 1)
#define RTE_SIO_5_PORT 0
#define RTE_SIO_5_PIN  30
#define RTE_SIO_5_MUX  1
#define RTE_SIO_5_PAD  0 //no pad
#else
#error "Invalid  RTE_SIO_5_PIN Pin Configuration!"
#endif

// SIO_6  GPIO_70
#define RTE_SIO_6_PORT 0
#define RTE_SIO_6_PIN  70
#define RTE_SIO_6_MUX  1
#define RTE_SIO_6_PAD  28

// SIO_7  <0=>GPIO_15 <1=>GPIO_71

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_SIO_7_PORT_ID 1
#else
#define RTE_SIO_7_PORT_ID 0
#endif

#if (RTE_SIO_7_PORT_ID == 0)
#define RTE_SIO_7_PORT 0
#define RTE_SIO_7_PIN  15
#define RTE_SIO_7_MUX  1
#define RTE_SIO_7_PAD  8
#elif (RTE_SIO_7_PORT_ID == 1)
#define RTE_SIO_7_PORT 0
#define RTE_SIO_7_PIN  71
#define RTE_SIO_7_MUX  1
#define RTE_SIO_7_PAD  29
#else
#error "Invalid  RTE_SIO_7_PIN Pin Configuration!"
#endif

//<> Pulse Width Modulation
//PWM_1H  <0=>GPIO_7  <1=>GPIO_64 <2=>GPIO_65

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_PWM_1H_PORT_ID 0
#else
#define RTE_PWM_1H_PORT_ID 0
#endif

#if (RTE_PWM_1H_PORT_ID == 0)
#define RTE_PWM_1H_PORT 0
#define RTE_PWM_1H_PIN  7
#define RTE_PWM_1H_MUX  10
#define RTE_PWM_1H_PAD  2
#elif (RTE_PWM_1H_PORT_ID == 1)
#define RTE_PWM_1H_PORT 0
#define RTE_PWM_1H_PIN  65
#define RTE_PWM_1H_MUX  8
#define RTE_PWM_1H_PAD  22
#else
#error "Invalid  RTE_PWM_1H_PIN Pin Configuration!"
#endif

// PWM_1L <0=>GPIO_6  <1=>GPIO_64 <2=>GPIO_64

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_PWM_1L_PORT_ID 0
#else
#define RTE_PWM_1L_PORT_ID 1
#endif

#if (RTE_PWM_1L_PORT_ID == 0)
#define RTE_PWM_1L_PORT 0
#define RTE_PWM_1L_PIN  6
#define RTE_PWM_1L_MUX  10
#define RTE_PWM_1L_PAD  1
#else
#error "Invalid  RTE_PWM_1L_PIN Pin Configuration!"
#endif

//PWM_2H  <0=>GPIO_9  <1=>GPIO_67 <2=>GPIO_69

#define RTE_PWM_2H_PORT_ID 0
#if ((RTE_PWM_2H_PORT_ID == 2))
#error "Invalid RTE_PWM_2H_PIN pin Configuration!"
#endif

#if (RTE_PWM_2H_PORT_ID == 0)
#define RTE_PWM_2H_PORT 0
#define RTE_PWM_2H_PIN  9
#define RTE_PWM_2H_MUX  10
#define RTE_PWM_2H_PAD  4
#elif (RTE_PWM_2H_PORT_ID == 1)
#define RTE_PWM_2H_PORT 0
#define RTE_PWM_2H_PIN  67
#define RTE_PWM_2H_MUX  8
#define RTE_PWM_2H_PAD  25
#else
#error "Invalid  RTE_PWM_2H_PIN Pin Configuration!"
#endif

// PWM_2L <0=>GPIO_8  <1=>GPIO_66 <2=>GPIO_68

#define RTE_PWM_2L_PORT_ID 0
#if ((RTE_PWM_2L_PORT_ID == 2))
#error "Invalid RTE_PWM_2L_PIN pin Configuration!"
#endif

#if (RTE_PWM_2L_PORT_ID == 0)
#define RTE_PWM_2L_PORT 0
#define RTE_PWM_2L_PIN  8
#define RTE_PWM_2L_MUX  10
#define RTE_PWM_2L_PAD  3
#elif (RTE_PWM_2L_PORT_ID == 1)
#define RTE_PWM_2L_PORT 0
#define RTE_PWM_2L_PIN  66
#define RTE_PWM_2L_MUX  8
#define RTE_PWM_2L_PAD  24
#else
#error "Invalid  RTE_PWM_2L_PIN Pin Configuration!"
#endif

// PWM_3H <0=>GPIO_11   <1=>GPIO_69
#define RTE_PWM_3H_PORT_ID 0
#if (RTE_PWM_3H_PORT_ID == 0)
#define RTE_PWM_3H_PORT 0
#define RTE_PWM_3H_PIN  11
#define RTE_PWM_3H_MUX  10
#define RTE_PWM_3H_PAD  6
#else
#error "Invalid  RTE_PWM_3H_PIN Pin Configuration!"
#endif

// PWM_3L <0=>GPIO_10 <1=>GPIO_68
#define RTE_PWM_3L_PORT_ID 0

#if (RTE_PWM_3L_PORT_ID == 0)
#define RTE_PWM_3L_PORT 0
#define RTE_PWM_3L_PIN  10
#define RTE_PWM_3L_MUX  10
#define RTE_PWM_3L_PAD  5
#else
#error "Invalid  RTE_PWM_3L_PIN Pin Configuration!"
#endif

// PWM_4H <0=>GPIO_15 <1=>GPIO_71

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_PWM_4H_PORT_ID 1
#else
#define RTE_PWM_4H_PORT_ID 0
#endif

#if (RTE_PWM_4H_PORT_ID == 0)
#define RTE_PWM_4H_PORT 0
#define RTE_PWM_4H_PIN  15
#define RTE_PWM_4H_MUX  10
#define RTE_PWM_4H_PAD  8
#elif (RTE_PWM_4H_PORT_ID == 1)
#define RTE_PWM_4H_PORT 0
#define RTE_PWM_4H_PIN  71
#define RTE_PWM_4H_MUX  8
#define RTE_PWM_4H_PAD  29
#else
#error "Invalid  RTE_PWM_4H_PIN Pin Configuration!"
#endif

// PWM_4H <0=>GPIO_12 <1=>GPIO_70

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_PWM_4L_PORT_ID 1
#else
#define RTE_PWM_4L_PORT_ID 0
#endif

#if (RTE_PWM_4L_PORT_ID == 0)
#define RTE_PWM_4L_PORT 0
#define RTE_PWM_4L_PIN  12
#define RTE_PWM_4L_MUX  10
#define RTE_PWM_4L_PAD  7
#elif (RTE_PWM_4L_PORT_ID == 1)
#define RTE_PWM_4L_PORT 0
#define RTE_PWM_4L_PIN  70
#define RTE_PWM_4L_MUX  8
#define RTE_PWM_4L_PAD  28
#else
#error "Invalid  RTE_PWM_4L_PIN Pin Configuration!"
#endif

// PWM_FAULTA <0=>GPIO_25 <1=>GPIO_68 <1=>GPIO_73
#define RTE_PWM_FAULTA_PORT_ID 0

#if (RTE_PWM_FAULTA_PORT_ID == 0)
#define RTE_PWM_FAULTA_PORT 0
#define RTE_PWM_FAULTA_PIN  25
#define RTE_PWM_FAULTA_MUX  10
#define RTE_PWM_FAULTA_PAD  0 //no pad
#elif (RTE_PWM_FAULTA_PORT_ID == 2)
#define RTE_PWM_FAULTA_PORT 0
#define RTE_PWM_FAULTA_PIN  73
#define RTE_PWM_FAULTA_MUX  8
#define RTE_PWM_FAULTA_PAD  31
#else
#error "Invalid  RTE_PWM_FAULTA_PIN Pin Configuration!"
#endif

// PWM_FAULTB <0=>GPIO_26 <1=>GPIO_69 <1=>GPIO_74
#define RTE_PWM_FAULTB_PORT_ID 0

#if (RTE_PWM_FAULTB_PORT_ID == 0)
#define RTE_PWM_FAULTB_PORT 0
#define RTE_PWM_FAULTB_PIN  26
#define RTE_PWM_FAULTB_MUX  10
#define RTE_PWM_FAULTB_PAD  0 //no pad
#elif (RTE_PWM_FAULTB_PORT_ID == 2)
#define RTE_PWM_FAULTB_PORT 0
#define RTE_PWM_FAULTB_PIN  74
#define RTE_PWM_FAULTB_MUX  8
#define RTE_PWM_FAULTB_PAD  32
#else
#error "Invalid  RTE_PWM_FAULTB_PIN Pin Configuration!"
#endif
//PWM_SLP_EVENT_TRIG   GPIO_72
#define RTE_PWM_SLP_EVENT_TRIG_PORT 0
#define RTE_PWM_SLP_EVENT_TRIG_PIN  72
#define RTE_PWM_SLP_EVENT_TRIG_MUX  8
#define RTE_PWM_SLP_EVENT_TRIG_PAD  30

//PWM_TMR_EXT_TRIG_1 <0=>GPIO_27 <1=>GPIO_51 <2=>GPIO_70 <3=>GPIO_75
#define RTE_PWM_TMR_EXT_TRIG_1_PORT_ID 0

#if (RTE_PWM_TMR_EXT_TRIG_1_PORT_ID == 0)
#define RTE_PWM_TMR_EXT_TRIG_1_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_1_PIN  27
#define RTE_PWM_TMR_EXT_TRIG_1_MUX  10
#define RTE_PWM_TMR_EXT_TRIG_1_PAD  0 //no pad
#elif (RTE_PWM_TMR_EXT_TRIG_1_PORT_ID == 1)
#define RTE_PWM_TMR_EXT_TRIG_1_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_1_PIN  51
#define RTE_PWM_TMR_EXT_TRIG_1_MUX  8
#define RTE_PWM_TMR_EXT_TRIG_1_PAD  15
#elif (RTE_PWM_TMR_EXT_TRIG_1_PORT_ID == 2)
#define RTE_PWM_TMR_EXT_TRIG_1_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_1_PIN  70
#define RTE_PWM_TMR_EXT_TRIG_1_MUX  10
#define RTE_PWM_TMR_EXT_TRIG_1_PAD  28
#elif (RTE_PWM_TMR_EXT_TRIG_1_PORT_ID == 3)
#define RTE_PWM_TMR_EXT_TRIG_1_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_1_PIN  75
#define RTE_PWM_TMR_EXT_TRIG_1_MUX  8
#define RTE_PWM_TMR_EXT_TRIG_1_PAD  33
#else
#error "Invalid  RTE_PWM_TMR_EXT_TRIG_1_PIN Pin Configuration!"
#endif

//PWM_TMR_EXT_TRIG_2 <0=>GPIO_28 <1=>GPIO_54 <2=>GPIO_71
#define RTE_PWM_TMR_EXT_TRIG_2_PORT_ID 0

#if (RTE_PWM_TMR_EXT_TRIG_2_PORT_ID == 0)
#define RTE_PWM_TMR_EXT_TRIG_2_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_2_PIN  28
#define RTE_PWM_TMR_EXT_TRIG_2_MUX  10
#define RTE_PWM_TMR_EXT_TRIG_2_PAD  0 //no pad
#elif (RTE_PWM_TMR_EXT_TRIG_2_PORT_ID == 1)
#define RTE_PWM_TMR_EXT_TRIG_2_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_2_PIN  54
#define RTE_PWM_TMR_EXT_TRIG_2_MUX  8
#define RTE_PWM_TMR_EXT_TRIG_2_PAD  18
#elif (RTE_PWM_TMR_EXT_TRIG_2_PORT_ID == 2)
#define RTE_PWM_TMR_EXT_TRIG_2_PORT 0
#define RTE_PWM_TMR_EXT_TRIG_2_PIN  71
#define RTE_PWM_TMR_EXT_TRIG_2_MUX  10
#define RTE_PWM_TMR_EXT_TRIG_2_PAD  29
#else
#error "Invalid  RTE_PWM_TMR_EXT_TRIG_2_PIN Pin Configuration!"
#endif

//<> QEI (Quadrature Encode Interface)

//QEI_DIR <0=>GPIO_28 <1=>GPIO_49 <2=>GPIO_57 <3=>GPIO_67 <4=>GPIO_71 <5=>GPIO_73 <6=>GPIO_11 <7=>GPIO_34

#define RTE_QEI_DIR_PORT_ID 4

#if (RTE_QEI_DIR_PORT_ID == 0)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  28
#define RTE_QEI_DIR_MUX  5
#define RTE_QEI_DIR_PAD  0 //no pad
#elif (RTE_QEI_DIR_PORT_ID == 1)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  49
#define RTE_QEI_DIR_MUX  3
#define RTE_QEI_DIR_PAD  13
#elif (RTE_QEI_DIR_PORT_ID == 2)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  57
#define RTE_QEI_DIR_MUX  5
#define RTE_QEI_DIR_PAD  21
#elif (RTE_QEI_DIR_PORT_ID == 3)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  67
#define RTE_QEI_DIR_MUX  3
#define RTE_QEI_DIR_PAD  25
#elif (RTE_QEI_DIR_PORT_ID == 4)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  71
#define RTE_QEI_DIR_MUX  3
#define RTE_QEI_DIR_PAD  29
#elif (RTE_QEI_DIR_PORT_ID == 5)
#define RTE_QEI_DIR_PORT 0
#define RTE_QEI_DIR_PIN  73
#define RTE_QEI_DIR_MUX  3
#define RTE_QEI_DIR_PAD  31
#else
#error "Invalid  RTE_QEI_DIR_PIN Pin Configuration!"
#endif

//QEI_IDX <0=>GPIO_25 <1=>GPIO_46 <2=>GPIO_52 <3=>GPIO_64 <4=>GPIO_68 <5=>GPIO_72 <6=>GPIO_8 <7=>GPIO_13

#define RTE_QEI_IDX_PORT_ID 3

#if (RTE_QEI_IDX_PORT_ID == 0)
#define RTE_QEI_IDX_PORT 0
#define RTE_QEI_IDX_PIN  25
#define RTE_QEI_IDX_MUX  5
#define RTE_QEI_IDX_PAD  0 //no pad
#elif (RTE_QEI_IDX_PORT_ID == 1)
#define RTE_QEI_IDX_PORT 0
#define RTE_QEI_IDX_PIN  46
#define RTE_QEI_IDX_MUX  3
#define RTE_QEI_IDX_PAD  10
#elif (RTE_QEI_IDX_PORT_ID == 2)
#define RTE_QEI_IDX_PORT 0
#define RTE_QEI_IDX_PIN  52
#define RTE_QEI_IDX_MUX  5
#define RTE_QEI_IDX_PAD  16
#elif (RTE_QEI_IDX_PORT_ID == 3)
#define RTE_QEI_IDX_PORT 0
#define RTE_QEI_IDX_PIN  72
#define RTE_QEI_IDX_MUX  3
#define RTE_QEI_IDX_PAD  30
#else
#error "Invalid  RTE_QEI_IDX_PIN Pin Configuration!"
#endif

//QEI_PHA <0=>GPIO_26 <1=>GPIO_47 <2=>GPIO_53 <3=>GPIO_65 <4=>GPIO_69 <5=>GPIO_73 <6=>GPIO_9 <7=>GPIO_32

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_QEI_PHA_PORT_ID 3
#else
#define RTE_QEI_PHA_PORT_ID 5
#endif

#if (RTE_QEI_PHA_PORT_ID == 0)
#define RTE_QEI_PHA_PORT 0
#define RTE_QEI_PHA_PIN  26
#define RTE_QEI_PHA_MUX  5
#define RTE_QEI_PHA_PAD  0 //no pad
#elif (RTE_QEI_PHA_PORT_ID == 1)
#define RTE_QEI_PHA_PORT 0
#define RTE_QEI_PHA_PIN  47
#define RTE_QEI_PHA_MUX  3
#define RTE_QEI_PHA_PAD  11
#elif (RTE_QEI_PHA_PORT_ID == 2)
#define RTE_QEI_PHA_PORT 0
#define RTE_QEI_PHA_PIN  53
#define RTE_QEI_PHA_MUX  5
#define RTE_QEI_PHA_PAD  17
#elif (RTE_QEI_PHA_PORT_ID == 3)
#define RTE_QEI_PHA_PORT 0
#define RTE_QEI_PHA_PIN  65
#define RTE_QEI_PHA_MUX  3
#define RTE_QEI_PHA_PAD  23
#elif (RTE_QEI_PHA_PORT_ID == 4)
#define RTE_QEI_PHA_PORT 0
#define RTE_QEI_PHA_PIN  73
#define RTE_QEI_PHA_MUX  3
#define RTE_QEI_PHA_PAD  31
#else
#error "Invalid  RTE_QEI_PHA_PIN Pin Configuration!"
#endif

//QEI_PHB <0=>GPIO_27 <1=>GPIO_48 <1=>GPIO_56 <1=>GPIO_66 <1=>GPIO_70 <1=>GPIO_74 <7=>GPIO_33

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_QEI_PHB_PORT_ID 5
#else
#define RTE_QEI_PHB_PORT_ID 4
#endif

#if (RTE_QEI_PHB_PORT_ID == 0)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  27
#define RTE_QEI_PHB_MUX  5
#define RTE_QEI_PHB_PAD  0 //no pad
#elif (RTE_QEI_PHB_PORT_ID == 1)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  48
#define RTE_QEI_PHB_MUX  3
#define RTE_QEI_PHB_PAD  12
#elif (RTE_QEI_PHB_PORT_ID == 2)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  56
#define RTE_QEI_PHB_MUX  5
#define RTE_QEI_PHB_PAD  20
#elif (RTE_QEI_PHB_PORT_ID == 3)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  66
#define RTE_QEI_PHB_MUX  3
#define RTE_QEI_PHB_PAD  24
#elif (RTE_QEI_PHB_PORT_ID == 4)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  70
#define RTE_QEI_PHB_MUX  3
#define RTE_QEI_PHB_PAD  28
#elif (RTE_QEI_PHB_PORT_ID == 5)
#define RTE_QEI_PHB_PORT 0
#define RTE_QEI_PHB_PIN  74
#define RTE_QEI_PHB_MUX  3
#define RTE_QEI_PHB_PAD  32
#else
#error "Invalid  RTE_QEI_PHB_PIN Pin Configuration!"
#endif

#endif

#define RTE_GPIO_6_PORT 0
#define RTE_GPIO_6_PAD  1
#define RTE_GPIO_6_PIN  6
#define RTE_GPIO_6_MODE 0

#define RTE_GPIO_7_PORT 0
#define RTE_GPIO_7_PAD  2
#define RTE_GPIO_7_PIN  7
#define RTE_GPIO_7_MODE 0

#define RTE_GPIO_8_PORT 0
#define RTE_GPIO_8_PAD  3
#define RTE_GPIO_8_PIN  8
#define RTE_GPIO_8_MODE 0

#define RTE_GPIO_9_PORT 0
#define RTE_GPIO_9_PAD  4
#define RTE_GPIO_9_PIN  9
#define RTE_GPIO_9_MODE 0

#define RTE_GPIO_10_PORT 0
#define RTE_GPIO_10_PAD  5
#define RTE_GPIO_10_PIN  10
#define RTE_GPIO_10_MODE 0

#define RTE_GPIO_11_PORT 0
#define RTE_GPIO_11_PAD  6
#define RTE_GPIO_11_PIN  11
#define RTE_GPIO_11_MODE 0

#define RTE_GPIO_12_PORT 0
#define RTE_GPIO_12_PAD  7
#define RTE_GPIO_12_PIN  12
#define RTE_GPIO_12_MODE 0

#define RTE_GPIO_15_PORT 0
#define RTE_GPIO_15_PAD  8
#define RTE_GPIO_15_PIN  15
#define RTE_GPIO_15_MODE 0

#define RTE_GPIO_25_PORT 1
#define RTE_GPIO_25_PIN  25
#define RTE_GPIO_25_MODE 0

#define RTE_GPIO_26_PORT 1
#define RTE_GPIO_26_PIN  26
#define RTE_GPIO_26_MODE 0

#define RTE_GPIO_27_PORT 1
#define RTE_GPIO_27_PIN  27
#define RTE_GPIO_27_MODE 0

#define RTE_GPIO_28_PORT 1
#define RTE_GPIO_28_PIN  28
#define RTE_GPIO_28_MODE 0

#define RTE_GPIO_29_PORT 1
#define RTE_GPIO_29_PIN  29
#define RTE_GPIO_29_MODE 0

#define RTE_GPIO_30_PORT 1
#define RTE_GPIO_30_PIN  30
#define RTE_GPIO_30_MODE 0

#define RTE_GPIO_31_PORT 1
#define RTE_GPIO_31_PAD  9
#define RTE_GPIO_31_PIN  31
#define RTE_GPIO_31_MODE 0

#define RTE_GPIO_32_PORT 2
#define RTE_GPIO_32_PAD  9
#define RTE_GPIO_32_PIN  32
#define RTE_GPIO_32_MODE 0

#define RTE_GPIO_33_PORT 2
#define RTE_GPIO_33_PAD  9
#define RTE_GPIO_33_PIN  33
#define RTE_GPIO_33_MODE 0

#define RTE_GPIO_34_PORT 2
#define RTE_GPIO_34_PAD  9
#define RTE_GPIO_34_PIN  34
#define RTE_GPIO_34_MODE 0

#define RTE_GPIO_46_PORT 2
#define RTE_GPIO_46_PAD  10
#define RTE_GPIO_46_PIN  46
#define RTE_GPIO_46_MODE 0

#define RTE_GPIO_47_PORT 2
#define RTE_GPIO_47_PAD  11
#define RTE_GPIO_47_PIN  47
#define RTE_GPIO_47_MODE 0

#define RTE_GPIO_48_PORT 3
#define RTE_GPIO_48_PAD  12
#define RTE_GPIO_48_PIN  48
#define RTE_GPIO_48_MODE 0

#define RTE_GPIO_49_PORT 3
#define RTE_GPIO_49_PAD  13
#define RTE_GPIO_49_PIN  49
#define RTE_GPIO_49_MODE 0

#define RTE_GPIO_50_PORT 3
#define RTE_GPIO_50_PAD  14
#define RTE_GPIO_50_PIN  50
#define RTE_GPIO_50_MODE 0

#define RTE_GPIO_51_PORT 3
#define RTE_GPIO_51_PAD  15
#define RTE_GPIO_51_PIN  51
#define RTE_GPIO_51_MODE 0

#define RTE_GPIO_52_PORT 3
#define RTE_GPIO_52_PAD  16
#define RTE_GPIO_52_PIN  52
#define RTE_GPIO_52_MODE 0

#define RTE_GPIO_53_PORT 3
#define RTE_GPIO_53_PAD  17
#define RTE_GPIO_53_PIN  53
#define RTE_GPIO_53_MODE 0

#define RTE_GPIO_54_PORT 3
#define RTE_GPIO_54_PAD  18
#define RTE_GPIO_54_PIN  54
#define RTE_GPIO_54_MODE 0

#define RTE_GPIO_55_PORT 3
#define RTE_GPIO_55_PAD  19
#define RTE_GPIO_55_PIN  55
#define RTE_GPIO_55_MODE 0

#define RTE_GPIO_56_PORT 3
#define RTE_GPIO_56_PAD  20
#define RTE_GPIO_56_PIN  56
#define RTE_GPIO_56_MODE 0

#define RTE_GPIO_57_PORT 3
#define RTE_GPIO_57_PAD  21
#define RTE_GPIO_57_PIN  57
#define RTE_GPIO_57_MODE 0

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_0_PORT_ID 1
#else
#define RTE_ULP_GPIO_0_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_0_PORT_ID == 0)
#define RTE_ULP_GPIO_0_PORT 0
#define RTE_ULP_GPIO_0_PAD  22
#define RTE_ULP_GPIO_0_PIN  64
#define RTE_ULP_GPIO_0_MODE 0
#elif (RTE_ULP_GPIO_0_PORT_ID == 1)
#define RTE_ULP_GPIO_0_PORT 4
#define RTE_ULP_GPIO_0_PIN  0
#define RTE_ULP_GPIO_0_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_0_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_1_PORT_ID 1
#else
#define RTE_ULP_GPIO_1_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_1_PORT_ID == 0)
#define RTE_ULP_GPIO_1_PORT 0
#define RTE_ULP_GPIO_1_PAD  23
#define RTE_ULP_GPIO_1_PIN  65
#define RTE_ULP_GPIO_1_MODE 0
#elif (RTE_ULP_GPIO_1_PORT_ID == 1)
#define RTE_ULP_GPIO_1_PORT 4
#define RTE_ULP_GPIO_1_PIN  1
#define RTE_ULP_GPIO_1_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_1_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_2_PORT_ID 1
#else
#define RTE_ULP_GPIO_2_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_2_PORT_ID == 0)
#define RTE_ULP_GPIO_2_PORT 0
#define RTE_ULP_GPIO_2_PAD  24
#define RTE_ULP_GPIO_2_PIN  66
#define RTE_ULP_GPIO_2_MODE 0
#elif (RTE_ULP_GPIO_2_PORT_ID == 1)
#define RTE_ULP_GPIO_2_PORT 4
#define RTE_ULP_GPIO_2_PIN  2
#define RTE_ULP_GPIO_2_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_2_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_3_PORT_ID 1
#else
#define RTE_ULP_GPIO_3_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_3_PORT_ID == 0)
#define RTE_ULP_GPIO_3_PORT 0
#define RTE_ULP_GPIO_3_PAD  25
#define RTE_ULP_GPIO_3_PIN  67
#define RTE_ULP_GPIO_3_MODE 0
#elif (RTE_ULP_GPIO_3_PORT_ID == 1)
#define RTE_ULP_GPIO_3_PORT 4
#define RTE_ULP_GPIO_3_PIN  3
#define RTE_ULP_GPIO_3_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_3_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_4_PORT_ID 1
#else
#define RTE_ULP_GPIO_4_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_4_PORT_ID == 0)
#define RTE_ULP_GPIO_4_PORT 0
#define RTE_ULP_GPIO_4_PAD  26
#define RTE_ULP_GPIO_4_PIN  68
#define RTE_ULP_GPIO_4_MODE 0
#elif (RTE_ULP_GPIO_4_PORT_ID == 1)
#define RTE_ULP_GPIO_4_PORT 4
#define RTE_ULP_GPIO_4_PIN  4
#define RTE_ULP_GPIO_4_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_4_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_5_PORT_ID 1
#else
#define RTE_ULP_GPIO_5_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_5_PORT_ID == 0)
#define RTE_ULP_GPIO_5_PORT 4
#define RTE_ULP_GPIO_5_PAD  27
#define RTE_ULP_GPIO_5_PIN  69
#define RTE_ULP_GPIO_5_MODE 0
#elif (RTE_ULP_GPIO_5_PORT_ID == 1)
#define RTE_ULP_GPIO_5_PORT 4
#define RTE_ULP_GPIO_5_PIN  5
#define RTE_ULP_GPIO_5_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_5_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_6_PORT_ID 1
#else
#define RTE_ULP_GPIO_6_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_6_PORT_ID == 0)
#define RTE_ULP_GPIO_6_PORT 4
#define RTE_ULP_GPIO_6_PAD  28
#define RTE_ULP_GPIO_6_PIN  70
#define RTE_ULP_GPIO_6_MODE 0
#elif (RTE_ULP_GPIO_6_PORT_ID == 1)
#define RTE_ULP_GPIO_6_PORT 4
#define RTE_ULP_GPIO_6_PIN  6
#define RTE_ULP_GPIO_6_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_6_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_7_PORT_ID 1
#else
#define RTE_ULP_GPIO_7_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_7_PORT_ID == 0)
#define RTE_ULP_GPIO_7_PORT 4
#define RTE_ULP_GPIO_7_PAD  29
#define RTE_ULP_GPIO_7_PIN  71
#define RTE_ULP_GPIO_7_MODE 0
#elif (RTE_ULP_GPIO_7_PORT_ID == 1)
#define RTE_ULP_GPIO_7_PORT 4
#define RTE_ULP_GPIO_7_PIN  7
#define RTE_ULP_GPIO_7_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_7_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_8_PORT_ID 1
#else
#define RTE_ULP_GPIO_8_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_8_PORT_ID == 0)
#define RTE_ULP_GPIO_8_PORT 4
#define RTE_ULP_GPIO_8_PAD  30
#define RTE_ULP_GPIO_8_PIN  72
#define RTE_ULP_GPIO_8_MODE 0
#elif (RTE_ULP_GPIO_8_PORT_ID == 1)
#define RTE_ULP_GPIO_8_PORT 4
#define RTE_ULP_GPIO_8_PIN  8
#define RTE_ULP_GPIO_8_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_8_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_9_PORT_ID 1
#else
#define RTE_ULP_GPIO_9_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_9_PORT_ID == 0)
#define RTE_ULP_GPIO_9_PORT 4
#define RTE_ULP_GPIO_9_PAD  31
#define RTE_ULP_GPIO_9_PIN  73
#define RTE_ULP_GPIO_9_MODE 0
#elif (RTE_ULP_GPIO_9_PORT_ID == 1)
#define RTE_ULP_GPIO_9_PORT 4
#define RTE_ULP_GPIO_9_PIN  9
#define RTE_ULP_GPIO_9_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_9_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_10_PORT_ID 1
#else
#define RTE_ULP_GPIO_10_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_10_PORT_ID == 0)
#define RTE_ULP_GPIO_10_PORT 4
#define RTE_ULP_GPIO_10_PAD  32
#define RTE_ULP_GPIO_10_PIN  74
#define RTE_ULP_GPIO_10_MODE 0
#elif (RTE_ULP_GPIO_10_PORT_ID == 1)
#define RTE_ULP_GPIO_10_PORT 4
#define RTE_ULP_GPIO_10_PIN  10
#define RTE_ULP_GPIO_10_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_10_PIN Pin Configuration!"
#endif

#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_ULP_GPIO_11_PORT_ID 1
#else
#define RTE_ULP_GPIO_11_PORT_ID 0
#endif

#if (RTE_ULP_GPIO_11_PORT_ID == 0)
#define RTE_ULP_GPIO_11_PORT 4
#define RTE_ULP_GPIO_11_PAD  33
#define RTE_ULP_GPIO_11_PIN  75
#define RTE_ULP_GPIO_11_MODE 0
#elif (RTE_ULP_GPIO_11_PORT_ID == 1)
#define RTE_ULP_GPIO_11_PORT 4
#define RTE_ULP_GPIO_11_PIN  11
#define RTE_ULP_GPIO_11_MODE 0
#else
#error "Invalid  RTE_ULP_GPIO_11_PIN Pin Configuration!"
#endif

#define RTE_UULP_GPIO_0_PIN  0
#define RTE_UULP_GPIO_0_MODE 0

#define RTE_UULP_GPIO_1_PIN  1
#define RTE_UULP_GPIO_1_MODE 0

#define RTE_UULP_GPIO_2_PIN  2
#define RTE_UULP_GPIO_2_MODE 0

#define RTE_UULP_GPIO_3_PIN  3
#define RTE_UULP_GPIO_3_MODE 0

#define RTE_UULP_GPIO_4_PIN  4
#define RTE_UULP_GPIO_4_MODE 0

#define RTE_UULP_GPIO_5_PIN  5
#define RTE_UULP_GPIO_5_MODE 0
