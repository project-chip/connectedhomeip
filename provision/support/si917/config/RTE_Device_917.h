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
#include "pin_config.h"

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
#ifndef USART0_CLK_LOC
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
#else
//Pintool data
#define RTE_USART0_CLK_PORT USART0_CLK_PORT
#define RTE_USART0_CLK_PIN  USART0_CLK_PIN
#if (USART0_CLK_LOC == 0)
#define RTE_USART0_CLK_MUX 2
#define RTE_USART0_CLK_PAD 3
#endif
#if (USART0_CLK_LOC == 1)
#define RTE_USART0_CLK_MUX 2
#define RTE_USART0_CLK_PAD 0 //NO PAD
#endif
#if (USART0_CLK_LOC == 2)
#define RTE_USART0_CLK_MUX 2
#define RTE_USART0_CLK_PAD 16
#endif
//Pintool data
#endif

//     <o> USART0_TX <0=>P0_15 <1=>P0_30  <2=>P0_54  <3=>P0_68 <4=>P0_71
//     <i> TX for USART0
#ifndef USART0_TX_LOC
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
#else
//Pintool data
#define RTE_USART0_TX_PORT USART0_TX_PORT
#if (USART0_TX_LOC == 0)
#define RTE_USART0_TX_PIN USART0_TX_PIN
#define RTE_USART0_TX_MUX 2
#define RTE_USART0_TX_PAD 8
#endif
#if (USART0_TX_LOC == 1)
#define RTE_USART0_TX_PIN USART0_TX_PIN
#define RTE_USART0_TX_MUX 2
#define RTE_USART0_TX_PAD 0 //NO PAD
#endif
#if (USART0_TX_LOC == 2)
#define RTE_USART0_TX_PIN USART0_TX_PIN
#define RTE_USART0_TX_MUX 2
#define RTE_USART0_TX_PAD 18
#endif
#if (USART0_TX_LOC == 3)
#define RTE_USART0_TX_PIN (USART0_TX_PIN + GPIO_MAX_PIN)
#define RTE_USART0_TX_MUX 4
#define RTE_USART0_TX_PAD 29
#endif
//Pintool data
#endif

//     <o> USART0_RX <0=>P0_10 <1=>P0_29 <2=>P0_55 <3=>P0_65 <4=>P0_70
//     <i> RX for USART0
#ifndef USART0_RX_LOC
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
#else
//Pintool data
#define RTE_USART0_RX_PORT USART0_RX_PORT
#if (USART0_RX_LOC == 0)
#define RTE_USART0_RX_PIN USART0_RX_PIN
#define RTE_USART0_RX_MUX 2
#define RTE_USART0_RX_PAD 5
#endif
#if (USART0_RX_LOC == 1)
#define RTE_USART0_RX_PIN USART0_RX_PIN
#define RTE_USART0_RX_MUX 2
#define RTE_USART0_RX_PAD 0 //no pad
#endif
#if (USART0_RX_LOC == 2)
#define RTE_USART0_RX_PIN USART0_RX_PIN
#define RTE_USART0_RX_MUX 2
#define RTE_USART0_RX_PAD 19
#endif
#if (USART0_RX_LOC == 3)
#define RTE_USART0_RX_PIN (USART0_RX_PIN + GPIO_MAX_PIN)
#define RTE_USART0_RX_MUX 2
#define RTE_USART0_RX_PAD 24
#endif
#if (USART0_RX_LOC == 4)
#define RTE_USART0_RX_PIN (USART0_RX_PIN + GPIO_MAX_PIN)
#define RTE_USART0_RX_MUX 4
#define RTE_USART0_RX_PAD 28
#endif
//Pintool data
#endif

//     <o> USART0_CTS <0=>P0_6 <1=>P0_26 <2=>P0_56 <3=>P0_70
//     <i> CTS for USART0
#ifndef USART0_CTS_LOC
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
#else
//Pintool data
#define RTE_USART0_CTS_PORT USART0_CTS_PORT
#if (USART0_CTS_LOC == 0)
#define RTE_USART0_CTS_PIN USART0_CTS_PIN
#define RTE_USART0_CTS_MUX 2
#define RTE_USART0_CTS_PAD 1
#endif
#if (USART0_CTS_LOC == 1)
#define RTE_USART0_CTS_PIN USART0_CTS_PIN
#define RTE_USART0_CTS_MUX 2
#define RTE_USART0_CTS_PAD 0 //NO PAD
#endif
#if (USART0_CTS_LOC == 2)
#define RTE_USART0_CTS_PIN USART0_CTS_PIN
#define RTE_USART0_CTS_MUX 2
#define RTE_USART0_CTS_PAD 20
#endif
#if (USART0_CTS_LOC == 3)
#define RTE_USART0_CTS_PIN (USART0_CTS_PIN + GPIO_MAX_PIN)
#define RTE_USART0_CTS_MUX 2
#define RTE_USART0_CTS_PAD 28
#endif
//Pintool data
#endif

//     <o> USART0_RTS <0=>P0_9  <1=>P0_28 <2=>P0_53 <3=>P0_69
//     <i> RTS for USART0
#ifndef USART0_RTS_LOC
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
#else
//Pintool data
#define RTE_USART0_RTS_PORT USART0_RTS_PORT
#define RTE_USART0_RTS_PIN  USART0_RTS_PIN
#if (USART0_RTS_LOC == 0)
#define RTE_USART0_RTS_MUX 2
#define RTE_USART0_RTS_PAD 4
#endif
#if (USART0_RTS_LOC == 1)
#define RTE_USART0_RTS_MUX 2
#define RTE_USART0_RTS_PAD 0 //NO PAD
#endif
#if (USART0_RTS_LOC == 2)
#define RTE_USART0_RTS_MUX 2
#define RTE_USART0_RTS_PAD 17
#endif
//Pintool data
#endif

//     <o> USART0_IR_TX <0=>P0_48 <1=>P0_72
//     <i> IR TX for USART0
#ifndef USART0_IRTX_LOC
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
#else
//Pintool data
#define RTE_USART0_IR_TX_PORT USART0_IRTX_PORT
#if (USART0_IRTX_LOC == 0)
#define RTE_USART0_IR_TX_PIN USART0_IRTX_PIN
#define RTE_USART0_IR_TX_MUX 2
#define RTE_USART0_IR_TX_PAD 12
#endif
#if (USART0_IRTX_LOC == 1)
#define RTE_USART0_IR_TX_PIN (USART0_IRTX_PIN + GPIO_MAX_PIN)
#define RTE_USART0_IR_TX_MUX 2
#define RTE_USART0_IR_TX_PAD 30
#endif
//Pintool data
#endif

//     <o> USART0_IR_RX <0=>P0_47 <1=>P0_71 <2=>P0_64 <3=>P0_25
//     <i> IR RX for USART0
#ifndef USART0_IRRX_LOC
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
#else
//Pintool data
#define RTE_USART0_IR_RX_PORT USART0_IRRX_PORT
#if (USART0_IRRX_LOC == 0)
#define RTE_USART0_IR_RX_PIN USART0_IRRX_PIN
#define RTE_USART0_IR_RX_MUX 2
#define RTE_USART0_IR_RX_PAD 11
#endif
#if (USART0_IRRX_LOC == 1)
#define RTE_USART0_IR_RX_PIN (USART0_IRRX_PIN + GPIO_MAX_PIN)
#define RTE_USART0_IR_RX_MUX 2
#define RTE_USART0_IR_RX_PAD 29
#endif
//Pintool data
#endif

//     <o> USART0_RI <0=>P0_27  <1=>P0_46 <2=>P0_68
//     <i> RI for USART0
#ifndef USART0_RI_LOC
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
#else
//Pintool data
#define RTE_USART0_RI_PORT USART0_RI_PORT
#define RTE_USART0_RI_PIN  USART0_RI_PIN
#if (USART0_RI_LOC == 0)
#define RTE_USART0_RI_MUX 2
#define RTE_USART0_RI_PAD 0 //no pad
#endif
#if (USART0_RI_LOC == 1)
#define RTE_USART0_RI_MUX 2
#define RTE_USART0_RI_PAD 10
#endif
//Pintool data
#endif

//     <o> USART0_DSR <0=>P0_11 <1=>P0_57
//     <i> DSR for USART0
#ifndef USART0_DSR_LOC
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
#else
//Pintool data
#define RTE_USART0_DSR_PORT USART0_DSR_PORT
#define RTE_USART0_DSR_PIN  USART0_DSR_PIN
#if (USART0_DSR_LOC == 0)
#define RTE_USART0_DSR_MUX 2
#define RTE_USART0_DSR_PAD 6
#endif
#if (USART0_DSR_LOC == 1)
#define RTE_USART0_DSR_MUX 2
#define RTE_USART0_DSR_PAD 21
#endif
//Pintool data
#endif

//     <o> USART0_DCD <0=>P0_12 <1=>P0_29
//     <i> DCD for USART0
#ifndef USART0_DCD_LOC
#define RTE_USART0_DCD_PORT 0
#define RTE_USART0_DCD_PIN  12
#else
#define RTE_USART0_DCD_PORT USART0_DCD_PORT
#define RTE_USART0_DCD_PIN  USART0_DCD_PIN
#endif
#define RTE_USART0_DCD_MUX 2
#define RTE_USART0_DCD_PAD 7

//     <o> USART0_DTR <0=>P0_7
//     <i> DTR for USART0
#ifndef USART0_DTR_LOC
#define RTE_USART0_DTR_PORT 0
#define RTE_USART0_DTR_PIN  7
#else
#define RTE_USART0_DTR_PORT USART0_DTR_PORT
#define RTE_USART0_DTR_PIN  USART0_DTR_PIN
#endif
#define RTE_USART0_DTR_MUX 2
#define RTE_USART0_DTR_PAD 2
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
#ifndef UART1_TX_LOC
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
#else
//Pintool data
#define RTE_UART1_TX_PORT UART1_TX_PORT
#if (UART1_TX_LOC == 0)
#define RTE_UART1_TX_PIN UART1_TX_PIN
#define RTE_UART1_TX_MUX 6
#define RTE_UART1_TX_PAD 2
#endif
#if (UART1_TX_LOC == 1)
#define RTE_UART1_TX_PIN UART1_TX_PIN
#define RTE_UART1_TX_MUX 6
#define RTE_UART1_TX_PAD 0 //no pad
#endif
#if (UART1_TX_LOC == 2)
#define RTE_UART1_TX_PIN (UART1_TX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_TX_MUX 9
#define RTE_UART1_TX_PAD 25
#endif
#if (UART1_TX_LOC == 3)
#define RTE_UART1_TX_PIN (UART1_TX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_TX_MUX 6
#define RTE_UART1_TX_PAD 31
#endif
#if (UART1_TX_LOC == 4)
#define RTE_UART1_TX_PIN (UART1_TX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_TX_MUX 9
#define RTE_UART1_TX_PAD 33
#endif
//Pintool data
#endif

//     <o> UART1_RX <0=>P0_6 <1=>P0_29 <2=>P0_66 <3=>P0_68 <4=>P0_72 <5=>P0_74  <6=>P0_33
//     <i> RX of UART1
#ifndef UART1_RX_LOC
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
#else
//Pintool data
#define RTE_UART1_RX_PORT UART1_RX_PORT
#if (UART1_RX_LOC == 0)
#define RTE_UART1_RX_PIN UART1_RX_PIN
#define RTE_UART1_RX_MUX 6
#define RTE_UART1_RX_PAD 1
#endif
#if (UART1_RX_LOC == 1)
#define RTE_UART1_RX_PIN UART1_RX_PIN
#define RTE_UART1_RX_MUX 6
#define RTE_UART1_RX_PAD 0 //no pad
#endif
#if (UART1_RX_LOC == 2)
#define RTE_UART1_RX_PIN (UART1_RX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_RX_MUX 9
#define RTE_UART1_RX_PAD 24
#endif
#if (UART1_RX_LOC == 3)
#define RTE_UART1_RX_PIN (UART1_RX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_RX_MUX 6
#define RTE_UART1_RX_PAD 30
#endif
#if (UART1_RX_LOC == 4)
#define RTE_UART1_RX_PIN (UART1_RX_PIN + GPIO_MAX_PIN)
#define RTE_UART1_RX_MUX 9
#define RTE_UART1_RX_PAD 32
#endif
//Pintool data
#endif

//     <o> UART1_CTS <0=>P0_11 <1=>P0_28 <2=>P0_51 <3=>P0_65 <4=>P0_71 <5=>P0_73 <6=>P0_32
//     <i> CTS of UART1
#ifndef UART1_CTS_LOC
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
#else
//Pintool data
#define RTE_UART1_CTS_PORT UART1_CTS_PORT
#if (UART1_CTS_LOC == 0)
#define RTE_UART1_CTS_PIN UART1_CTS_PIN
#define RTE_UART1_CTS_MUX 6
#define RTE_UART1_CTS_PAD 6
#endif
#if (UART1_CTS_LOC == 1)
#define RTE_UART1_CTS_PIN UART1_CTS_PIN
#define RTE_UART1_CTS_MUX 6
#define RTE_UART1_CTS_PAD 0 //no pad
#endif
#if (UART1_CTS_LOC == 2)
#define RTE_UART1_CTS_PIN UART1_CTS_PIN
#define RTE_UART1_CTS_MUX 9
#define RTE_UART1_CTS_PAD 15
#endif
#if (UART1_CTS_LOC == 3)
#define RTE_UART1_CTS_PIN (UART1_CTS_PIN + GPIO_MAX_PIN)
#define RTE_UART1_CTS_MUX 9
#define RTE_UART1_CTS_PAD 23
#endif
#if (UART1_CTS_LOC == 4)
#define RTE_UART1_CTS_PIN (UART1_CTS_PIN + GPIO_MAX_PIN)
#define RTE_UART1_CTS_MUX 6
#define RTE_UART1_CTS_PAD 29
#endif
//Pintool data
#endif

//     <o> UART1_RTS  <0=>P0_10 <1=>P0_27 <2=>P0_50 <3=>P0_64  <4=>P0_70 <5=>P0_72 <6=>P0_31
//     <i> RTS of UART1
#ifndef UART1_RTS_LOC
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
#else
//Pintool data
#define RTE_UART1_RTS_PORT UART1_RTS_PORT
#if (UART1_RTS_LOC == 0)
#define RTE_UART1_RTS_PIN UART1_RTS_PIN
#define RTE_UART1_RTS_MUX 6
#define RTE_UART1_RTS_PAD 5
#endif
#if (UART1_RTS_LOC == 1)
#define RTE_UART1_RTS_PIN UART1_RTS_PIN
#define RTE_UART1_RTS_MUX 6
#define RTE_UART1_RTS_PAD 0 //no pad
#endif
#if (UART1_RTS_LOC == 2)
#define RTE_UART1_RTS_PIN UART1_RTS_PIN
#define RTE_UART1_RTS_MUX 9
#define RTE_UART1_RTS_PAD 14
#endif
#if (UART1_RTS_LOC == 3)
#define RTE_UART1_RTS_PIN (UART1_RTS_PIN + GPIO_MAX_PIN)
#define RTE_UART1_RTS_MUX 6
#define RTE_UART1_RTS_PAD 28
#endif
#if (UART1_RTS_LOC == 4)
#define RTE_UART1_RTS_PIN (UART1_RTS_PIN + GPIO_MAX_PIN)
#define RTE_UART1_RTS_MUX 9
#define RTE_UART1_RTS_PAD 30
#endif
//Pintool data
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
#ifndef ULP_UART_TX_LOC
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
#else
//Pintool data
#define RTE_ULP_UART_TX_PORT ULP_UART_TX_PORT
#define RTE_ULP_UART_TX_PIN  ULP_UART_TX_PIN
#define RTE_ULP_UART_TX_MUX  3
//Pintool data
#endif

//     <o> UART1_RX <0=>P0_2 <1=>P0_6 <2=>P0_9
//     <i> RX of ULPSS UART
#ifndef ULP_UART_RX_LOC
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
#else
//Pintool data
#define RTE_ULP_UART_RX_PORT ULP_UART_RX_PORT
#define RTE_ULP_UART_RX_PIN  ULP_UART_RX_PIN
#define RTE_ULP_UART_RX_MUX  3
//Pintool data
#endif

//     <o> UART1_CTS <0=>P0_1 <1=>P0_5 <2=>P0_8
//     <i> CTS of ULPSS UART
#ifndef ULP_UART_CTS_LOC
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
#else
//Pintool data
#define RTE_ULP_UART_CTS_PORT ULP_UART_CTS_PORT
#define RTE_ULP_UART_CTS_PIN  ULP_UART_CTS_PIN
#define RTE_ULP_UART_CTS_MUX  3
//Pintool data
#endif

//     <o> UART1_RTS <0=>P0_0 <1=>P0_4 <2=>P0_10
//     <i> RTS of ULPSS UART
#ifndef ULP_UART_RTS_LOC
#define RTE_ULP_UART_RTS_PORT_ID 0
#if (RTE_ULP_UART_RTS_PORT_ID == 0)
#define RTE_ULP_UART_RTS_PORT 0
#define RTE_ULP_UART_RTS_PIN  10
#else
#error "Invalid ULPSS UART RTE_ULP_UART_RTS_PIN Pin Configuration!"
#endif
#else
#define RTE_ULP_UART_RTS_PORT ULP_UART_RTS_PORT
#define RTE_ULP_UART_RTS_PIN  ULP_UART_RTS_PIN
#endif
#define RTE_ULP_UART_RTS_MUX 8

//   </e>

// <e> SSI_MASTER (Serial Peripheral Interface 1) [Driver_SSI_MASTER]
// <i> Configuration settings for Driver_SSI_MASTER in component ::CMSIS Driver:SPI
#define RTE_SSI_MASTER 1

#define RTE_SSI_MASTER_INPUT_CLOCK SSI_SOCPLLCLK

// <o> SSI_MASTER_MISO Pin <0=>GPIO_12 <1=>GPIO_27 <2=>GPIO_57
#ifndef SSI_MASTER_MISO_LOC
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
#else
//Pintool data
#define RTE_SSI_MASTER_MISO      1
#define RTE_SSI_MASTER_MISO_PORT SSI_MASTER_MISO_PT_PORT
#define RTE_SSI_MASTER_MISO_PIN  SSI_MASTER_MISO_PT_PIN
#define RTE_SSI_MASTER_MISO_MODE EGPIO_PIN_MUX_MODE3
#if (SSI_MASTER_MISO_LOC == 0)
#define RTE_SSI_MASTER_MISO_PADSEL 7
#endif
#if (SSI_MASTER_MISO_LOC == 1)
#define RTE_SSI_MASTER_MISO_PADSEL 0 //NO PAD
#endif
#if (SSI_MASTER_MISO_LOC == 2)
#define RTE_SSI_MASTER_MISO_PADSEL 21
#endif
//Pintool data
#endif

// <o> SSI_MASTER_MOSI Pin <0=>GPIO_11 <1=>GPIO_26 <2=>GPIO_56
#ifndef SSI_MASTER_MOSI_LOC
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
#else
//Pintool data
#define RTE_SSI_MASTER_MOSI      1
#define RTE_SSI_MASTER_MOSI_PORT SSI_MASTER_MOSI_PT_PORT
#define RTE_SSI_MASTER_MOSI_PIN  SSI_MASTER_MOSI_PT_PIN
#define RTE_SSI_MASTER_MOSI_MODE EGPIO_PIN_MUX_MODE3
#if (SSI_MASTER_MOSI_LOC == 0)
#define RTE_SSI_MASTER_MOSI_PADSEL 6
#endif
#if (SSI_MASTER_MOSI_LOC == 1)
#define RTE_SSI_MASTER_MOSI_PADSEL 0 //NO PAD
#endif
#if (SSI_MASTER_MOSI_LOC == 2)
#define RTE_SSI_MASTER_MOSI_PADSEL 20
#endif
//Pintool data
#endif

// <o> SSI_MASTER_SCK Pin <0=>GPIO_8 <1=>GPIO_25 <2=>GPIO_52
#ifndef SSI_MASTER_SCK_LOC
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
#else
//Pintool data
#define RTE_SSI_MASTER_SCK      1
#define RTE_SSI_MASTER_SCK_PORT SSI_MASTER_SCK_PT_PORT
#define RTE_SSI_MASTER_SCK_PIN  SSI_MASTER_SCK_PT_PIN
#define RTE_SSI_MASTER_SCK_MODE EGPIO_PIN_MUX_MODE3
#if (SSI_MASTER_SCK_LOC == 0)
#define RTE_SSI_MASTER_SCK_PADSEL 3
#endif
#if (SSI_MASTER_SCK_LOC == 1)
#define RTE_SSI_MASTER_SCK_PADSEL 0 //NO PAD
#endif
#if (SSI_MASTER_SCK_LOC == 2)
#define RTE_SSI_MASTER_SCK_PADSEL 16
#endif
//Pintool data
#endif

#define M4_SSI_CS0 1
#define M4_SSI_CS1 0
#define M4_SSI_CS2 0
#define M4_SSI_CS3 0

// <o> SSI_MASTER_CS Pin <0=>GPIO_9 <1=>GPIO_28 <2=>GPIO_53 <3=>GPIO_10 <4=>GPIO_15 <5=>GPIO_50 <6=>GPIO_51
#ifndef SSI_MASTER_CS0_LOC
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
#else
//Pintool data
#define RTE_SSI_MASTER_CS0      M4_SSI_CS0
#define RTE_SSI_MASTER_CS0_PORT SSI_MASTER_CS0_PT_PORT
#define RTE_SSI_MASTER_CS0_PIN  SSI_MASTER_CS0_PT_PIN
#define RTE_SSI_MASTER_CS0_MODE EGPIO_PIN_MUX_MODE3
#if (SSI_MASTER_CS0_LOC == 0)
#define RTE_SSI_MASTER_CS0_PADSEL 4
#endif
#if (SSI_MASTER_CS0_LOC == 1)
#define RTE_SSI_MASTER_CS0_PADSEL 0 //NO PAD
#endif
#if (SSI_MASTER_CS0_LOC == 2)
#define RTE_SSI_MASTER_CS0_PADSEL 17
#endif
//Pintool data
#endif

//CS1
#ifndef SSI_MASTER_CS1_LOC
#define RTE_SSI_MASTER_CS1_PORT_ID 0
#if (RTE_SSI_MASTER_CS1_PORT_ID == 0)
#define RTE_SSI_MASTER_CS1_PORT 0
#define RTE_SSI_MASTER_CS1_PIN  10
#else
#error "Invalid SSI_MASTER_CS1 Pin Configuration!"
#endif
#else
#define RTE_SSI_MASTER_CS1_PORT SSI_MASTER_CS1_PT_PORT
#define RTE_SSI_MASTER_CS1_PIN  SSI_MASTER_CS1_PT_PIN
#endif
#define RTE_SSI_MASTER_CS1        M4_SSI_CS1
#define RTE_SSI_MASTER_CS1_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS1_PADSEL 5

//CS2
#ifndef SSI_MASTER_CS2_LOC
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
#else
//Pintool data
#define RTE_SSI_MASTER_CS2      M4_SSI_CS2
#define RTE_SSI_MASTER_CS2_PORT SSI_MASTER_CS2_PT_PORT
#define RTE_SSI_MASTER_CS2_PIN  SSI_MASTER_CS2_PT_PIN
#define RTE_SSI_MASTER_CS2_MODE EGPIO_PIN_MUX_MODE3
#if (SSI_MASTER_CS2_LOC == 0)
#define RTE_SSI_MASTER_CS2_PADSEL 8
#endif
#if (SSI_MASTER_CS2_LOC == 1)
#define RTE_SSI_MASTER_CS2_PADSEL 14
#endif
//Pintool data
#endif

//CS3
#ifndef SSI_MASTER_CS3_LOC
#define RTE_SSI_MASTER_CS3_PORT_ID 0
#if (RTE_SSI_MASTER_CS3_PORT_ID == 0)
#define RTE_SSI_MASTER_CS3_PORT 0
#define RTE_SSI_MASTER_CS3_PIN  51
#else
#error "Invalid SSI_MASTER_CS3 Pin Configuration!"
#endif
#else
#define RTE_SSI_MASTER_CS3_PORT SSI_MASTER_CS3_PT_PORT
#define RTE_SSI_MASTER_CS3_PIN  SSI_MASTER_CS3_PT_PIN
#endif
#define RTE_SSI_MASTER_CS3        M4_SSI_CS3
#define RTE_SSI_MASTER_CS3_MODE   EGPIO_PIN_MUX_MODE3
#define RTE_SSI_MASTER_CS3_PADSEL 15

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
#ifndef SSI_SLAVE_MISO_LOC
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
#else
//Pintool data
#define RTE_SSI_SLAVE_MISO      1
#define RTE_SSI_SLAVE_MISO_PORT SSI_SLAVE_MISO_PT_PORT
#define RTE_SSI_SLAVE_MISO_PIN  SSI_SLAVE_MISO_PT_PIN
#define RTE_SSI_SLAVE_MISO_MODE EGPIO_PIN_MUX_MODE8
#if (SSI_SLAVE_MISO_LOC == 1)
#define RTE_SSI_SLAVE_MISO_PADSEL 6
#endif
#if (SSI_SLAVE_MISO_LOC == 2)
#define RTE_SSI_SLAVE_MISO_PADSEL 0 //no pad
#endif
#if (SSI_SLAVE_MISO_LOC == 3)
#define RTE_SSI_SLAVE_MISO_PADSEL 13
#endif
#if (SSI_SLAVE_MISO_LOC == 4)
#define RTE_SSI_SLAVE_MISO_PADSEL 21
#endif
//Pintool data
#endif

// <o> SSI_SLAVE_MOSI Pin <0=>Not Used <1=>GPIO_10 <2=>GPIO_27 <3=>GPIO_48 <4=>GPIO_56
#ifndef SSI_SLAVE_MOSI_LOC
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
#else
//Pintool data
#define RTE_SSI_SLAVE_MOSI      1
#define RTE_SSI_SLAVE_MOSI_PORT SSI_SLAVE_MOSI_PT_PORT
#define RTE_SSI_SLAVE_MOSI_PIN  SSI_SLAVE_MOSI_PT_PIN
#define RTE_SSI_SLAVE_MOSI_MODE EGPIO_PIN_MUX_MODE8
#if (SSI_SLAVE_MOSI_LOC == 1)
#define RTE_SSI_SLAVE_MOSI_PADSEL 5
#endif
#if (SSI_SLAVE_MOSI_LOC == 2)
#define RTE_SSI_SLAVE_MOSI_PADSEL 0 //no pad
#endif
#if (SSI_SLAVE_MOSI_LOC == 3)
#define RTE_SSI_SLAVE_MOSI_PADSEL 12
#endif
#if (SSI_SLAVE_MOSI_LOC == 4)
#define RTE_SSI_SLAVE_MOSI_PADSEL 20
#endif
//Pintool data
#endif

// <o> SSI_SLAVE_SCK Pin <0=>Not Used <1=>GPIO_8 <2=>GPIO_26 <3=>GPIO_47 <4=>GPIO_52
#ifndef SSI_SLAVE_SCK_LOC
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
#else
//Pintool data
#define RTE_SSI_SLAVE_SCK      1
#define RTE_SSI_SLAVE_SCK_PORT SSI_SLAVE_SCK_PT_PORT
#define RTE_SSI_SLAVE_SCK_PIN  SSI_SLAVE_SCK_PT_PIN
#define RTE_SSI_SLAVE_SCK_MODE EGPIO_PIN_MUX_MODE8
#if (SSI_SLAVE_SCK_LOC == 1)
#define RTE_SSI_SLAVE_SCK_PADSEL 3
#endif
#if (SSI_SLAVE_SCK_LOC == 2)
#define RTE_SSI_SLAVE_SCK_PADSEL 0 //no pad
#endif
#if (SSI_SLAVE_SCK_LOC == 3)
#define RTE_SSI_SLAVE_SCK_PADSEL 11
#endif
#if (SSI_SLAVE_SCK_LOC == 4)
#define RTE_SSI_SLAVE_SCK_PADSEL 16
#endif
//Pintool data
#endif

// <o> SSI_SLAVE_CS Pin <0=>Not Used <1=>GPIO_9 <2=>GPIO_25 <3=>GPIO_46 <4=>GPIO_53
#ifndef SSI_SLAVE_CS_LOC
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
#else
//Pintool data
#define RTE_SSI_SLAVE_CS      1
#define RTE_SSI_SLAVE_CS_PORT SSI_SLAVE_CS_PT_PORT
#define RTE_SSI_SLAVE_CS_PIN  SSI_SLAVE_CS_PT_PIN
#define RTE_SSI_SLAVE_CS_MODE EGPIO_PIN_MUX_MODE8
#if (SSI_SLAVE_CS_LOC == 1)
#define RTE_SSI_SLAVE_CS_PADSEL 4
#endif
#if (SSI_SLAVE_CS_LOC == 2)
#define RTE_SSI_SLAVE_CS_PADSEL 0 //no pad
#endif
#if (SSI_SLAVE_CS_LOC == 3)
#define RTE_SSI_SLAVE_CS_PADSEL 10
#endif
#if (SSI_SLAVE_CS_LOC == 4)
#define RTE_SSI_SLAVE_CS_PADSEL 17
#endif
//Pintool data
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
#ifndef SSI_ULP_MASTER_MISO_LOC
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
#else
//Pintool data
#define RTE_SSI_ULP_MASTER_MISO      1
#define RTE_SSI_ULP_MASTER_MISO_PORT SSI_ULP_MASTER_MISO_PT_PORT
#define RTE_SSI_ULP_MASTER_MISO_PIN  SSI_ULP_MASTER_MISO_PT_PIN
#define RTE_SSI_ULP_MASTER_MISO_MODE 1
//Pintool data
#endif

// <o> SSI_ULP_MASTER_MOSI Pin <0=>Not Used <1=>ULP_GPIO_1 <2=>ULP_GPIO_11
#ifndef SSI_ULP_MASTER_MOSI_LOC
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
#else
//Pintool data
#define RTE_SSI_ULP_MASTER_MOSI      1
#define RTE_SSI_ULP_MASTER_MOSI_PORT SSI_ULP_MASTER_MOSI_PT_PORT
#define RTE_SSI_ULP_MASTER_MOSI_PIN  SSI_ULP_MASTER_MOSI_PT_PIN
#define RTE_SSI_ULP_MASTER_MOSI_MODE 1
//Pintool data
#endif

// <o> SSI_ULP_MASTER_SCK Pin <0=>Not Used <1=>ULP_GPIO_0 <2=>ULP_GPIO_8
#ifndef SSI_ULP_MASTER_SCK_LOC
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
#else
//Pintool data
#define RTE_SSI_ULP_MASTER_SCK      1
#define RTE_SSI_ULP_MASTER_SCK_PORT SSI_ULP_MASTER_SCK_PT_PORT
#define RTE_SSI_ULP_MASTER_SCK_PIN  SSI_ULP_MASTER_SCK_PT_PIN
#define RTE_SSI_ULP_MASTER_SCK_MODE 1
//Pintool data
#endif

// CS0
#ifndef SSI_ULP_MASTER_CS0_LOC
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
#else
//Pintool data
#define RTE_SSI_ULP_MASTER_CS0      ULP_SSI_CS0
#define RTE_SSI_ULP_MASTER_CS0_PORT SSI_ULP_MASTER_CS0_PT_PORT
#define RTE_SSI_ULP_MASTER_CS0_PIN  SSI_ULP_MASTER_CS0_PT_PIN
#define RTE_SSI_ULP_MASTER_CS0_MODE 1
//Pintool data
#endif

// CS1
#ifndef SSI_ULP_MASTER_CS1_LOC
#define RTE_SSI_ULP_MASTER_CS1_PORT 0
#define RTE_SSI_ULP_MASTER_CS1_PIN  4
#else
#define RTE_SSI_ULP_MASTER_CS1_PORT SSI_ULP_MASTER_CS1_PT_PORT
#define RTE_SSI_ULP_MASTER_CS1_PIN  SSI_ULP_MASTER_CS1_PT_PIN
#endif
#define RTE_SSI_ULP_MASTER_CS1      ULP_SSI_CS1
#define RTE_SSI_ULP_MASTER_CS1_MODE 1

// CS2
#ifndef SSI_ULP_MASTER_CS2_LOC
#define RTE_SSI_ULP_MASTER_CS2_PORT 0
#define RTE_SSI_ULP_MASTER_CS2_PIN  6
#else
#define RTE_SSI_ULP_MASTER_CS2_PORT SSI_ULP_MASTER_CS2_PT_PORT
#define RTE_SSI_ULP_MASTER_CS2_PIN  SSI_ULP_MASTER_CS2_PT_PIN
#endif
#define RTE_SSI_ULP_MASTER_CS2      ULP_SSI_CS2
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
#ifndef I2S0_SCLK_LOC
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
#else
//Pintool data
#define RTE_I2S0_SCLK_PORT I2S0_SCLK_PORT
#define RTE_I2S0_SCLK_PIN  I2S0_SCLK_PIN
#define RTE_I2S0_SCLK_MUX  7
#if (I2S0_SCLK_LOC == 0)
#define RTE_I2S0_SCLK_PAD 3
#endif
#if (I2S0_SCLK_LOC == 1)
#define RTE_I2S0_SCLK_PAD 0 //no pad
#endif
#if (I2S0_SCLK_LOC == 2)
#define RTE_I2S0_SCLK_PAD 10
#endif
#if (I2S0_SCLK_LOC == 3)
#define RTE_I2S0_SCLK_PAD 16
#endif
//Pintool data
#endif

//     <o> I2S0_WSCLK <0=>P0_9 <1=>P0_26  <2=>P0_47  <3=>P0_53
//     <i> WSCLK for I2S0
#ifndef I2S0_WSCLK_LOC
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
#else
//Pintool data
#define RTE_I2S0_WSCLK_PORT I2S0_WSCLK_PORT
#define RTE_I2S0_WSCLK_PIN  I2S0_WSCLK_PIN
#define RTE_I2S0_WSCLK_MUX  7
#if (I2S0_WSCLK_LOC == 0)
#define RTE_I2S0_WSCLK_PAD 4
#endif
#if (I2S0_WSCLK_LOC == 1)
#define RTE_I2S0_WSCLK_PAD 0
#endif
#if (I2S0_WSCLK_LOC == 2)
#define RTE_I2S0_WSCLK_PAD 11
#endif
#if (I2S0_WSCLK_LOC == 3)
#define RTE_I2S0_WSCLK_PAD 17
#endif
//Pintool data
#endif

//     <o> I2S0_DOUT0 <0=>P0_11 <1=>P0_28  <2=>P0_49  <3=>P0_57
//     <i> DOUT0 for I2S0
#ifndef I2S0_DOUT0_LOC
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
#else
//Pintool data
#define RTE_I2S0_DOUT0_PORT I2S0_DOUT0_PORT
#define RTE_I2S0_DOUT0_PIN  I2S0_DOUT0_PIN
#define RTE_I2S0_DOUT0_MUX  7
#if (I2S0_DOUT0_LOC == 0)
#define RTE_I2S0_DOUT0_PAD 6
#endif
#if (I2S0_DOUT0_LOC == 1)
#define RTE_I2S0_DOUT0_PAD 0
#endif
#if (I2S0_DOUT0_LOC == 2)
#define RTE_I2S0_DOUT0_PAD 13
#endif
#if (I2S0_DOUT0_LOC == 3)
#define RTE_I2S0_DOUT0_PAD 21
#endif
//Pintool data
#endif

//     <o> I2S0_DIN0 <0=>P0_10 <1=>P0_27  <2=>P0_48  <3=>P0_56
//     <i> DIN0 for I2S0
#ifndef I2S0_DIN0_LOC
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
#else
//Pintool data
#define RTE_I2S0_DIN0_PORT I2S0_DIN0_PORT
#define RTE_I2S0_DIN0_PIN  I2S0_DIN0_PIN
#define RTE_I2S0_DIN0_MUX  7
#if (I2S0_DIN0_LOC == 0)
#define RTE_I2S0_DIN0_PAD 5
#endif
#if (I2S0_DIN0_LOC == 1)
#define RTE_I2S0_DIN0_PAD 0
#endif
#if (I2S0_DIN0_LOC == 2)
#define RTE_I2S0_DIN0_PAD 12
#endif
#if (I2S0_DIN0_LOC == 3)
#define RTE_I2S0_DIN0_PAD 20
#endif
//Pintool data
#endif

//     <o> I2S0_DOUT1 <0=>P0_7 <1=>P0_30 <2=>P0_51 <3=>P0_55
//     <i> DOUT1 for I2S0
#ifndef I2S0_DOUT1_LOC
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
#else
//Pintool data
#define RTE_I2S0_DOUT1_PORT I2S0_DOUT1_PORT
#define RTE_I2S0_DOUT1_PIN  I2S0_DOUT1_PIN
#define RTE_I2S0_DOUT1_MUX  7
#if (I2S0_DOUT1_LOC == 0)
#define RTE_I2S0_DOUT1_PAD 2
#endif
#if (I2S0_DOUT1_LOC == 1)
#define RTE_I2S0_DOUT1_PAD 0
#endif
#if (I2S0_DOUT1_LOC == 2)
#define RTE_I2S0_DOUT1_PAD 15
#endif
#if (I2S0_DOUT1_LOC == 3)
#define RTE_I2S0_DOUT1_PAD 19
#endif
//Pintool data
#endif

//     <o> I2S0_DIN1 <0=>P0_6 <1=>P0_29 <2=>P0_50 <3=>P0_54
//     <i> DIN1 for I2S0
#ifndef I2S0_DIN1_LOC
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
#else
//Pintool data
#define RTE_I2S0_DIN1_PORT I2S0_DIN1_PORT
#define RTE_I2S0_DIN1_PIN  I2S0_DIN1_PIN
#define RTE_I2S0_DIN1_MUX  7
#if (I2S0_DIN1_LOC == 0)
#define RTE_I2S0_DIN1_PAD 1
#endif
#if (I2S0_DIN1_LOC == 1)
#define RTE_I2S0_DIN1_PAD 0
#endif
#if (I2S0_DIN1_LOC == 2)
#define RTE_I2S0_DIN1_PAD 14
#endif
#if (I2S0_DIN1_LOC == 3)
#define RTE_I2S0_DIN1_PAD 18
#endif
//Pintool data
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
#ifndef I2S1_SCLK_LOC
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
#else
//Pintool data
#define RTE_I2S1_SCLK_PORT I2S1_SCLK_PORT
#define RTE_I2S1_SCLK_PIN  I2S1_SCLK_PIN
#define RTE_I2S1_SCLK_MUX  2
//Pintool data
#endif

//   <o> I2S1_WSCLK Pin <0=>P0_2 <1=>P0_4 <2=>P0_10
#ifndef I2S1_WSCLK_LOC
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
#else
//Pintool data
#define RTE_I2S1_WSCLK_PORT I2S1_WSCLK_PORT
#define RTE_I2S1_WSCLK_PIN  I2S1_WSCLK_PIN
#define RTE_I2S1_WSCLK_MUX  2
//Pintool data
#endif

//   <o> I2S1_DOUT0 Pin <0=>P0_1 <1=>P0_5 <2=>P0_11
#ifndef I2S1_DOUT0_LOC
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
#else
//Pintool data
#define RTE_I2S1_DOUT0_PORT I2S1_DOUT0_PORT
#define RTE_I2S1_DOUT0_PIN  I2S1_DOUT0_PIN
#define RTE_I2S1_DOUT0_MUX  2
//Pintool data
#endif

//   <o> I2S1_DIN0 Pin <0=>P0_0 <1=>P0_6 <2=>P0_9 <3=>P0_13
#ifndef I2S1_DIN0_LOC
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
#else
//Pintool data
#define RTE_I2S1_DIN0_PORT I2S1_DIN0_PORT
#define RTE_I2S1_DIN0_PIN  I2S1_DIN0_PIN
#define RTE_I2S1_DIN0_MUX  2
//Pintool data
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
#ifndef I2C0_SCL_LOC
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
#else
//Pintool data
#define RTE_I2C0_SCL_PORT I2C0_SCL_PORT
#if (I2C0_SCL_LOC == 0)
#define RTE_I2C0_SCL_PIN     I2C0_SCL_PIN
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     2
#define RTE_I2C0_SCL_I2C_REN 7
#endif
#if (I2C0_SCL_LOC == 1)
#define RTE_I2C0_SCL_PIN     (I2C0_SCL_PIN + GPIO_MAX_PIN)
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     23
#define RTE_I2C0_SCL_I2C_REN 1
#endif
#if (I2C0_SCL_LOC == 2)
#define RTE_I2C0_SCL_PIN     (I2C0_SCL_PIN + GPIO_MAX_PIN)
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     24
#define RTE_I2C0_SCL_I2C_REN 2
#endif
#if (I2C0_SCL_LOC == 3)
#define RTE_I2C0_SCL_PIN     (I2C0_SCL_PIN + GPIO_MAX_PIN)
#define RTE_I2C0_SCL_MUX     4
#define RTE_I2C0_SCL_PAD     33
#define RTE_I2C0_SCL_I2C_REN 11
#endif
//Pintool data
#endif

//   <o> I2C0_SCL Pin <0=>P0_6 <1=>P0_64 <2=>P0_67 <3=>P0_74 <4=>P0_31
#ifndef I2C0_SDA_LOC
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
#else
//Pintool data
#define RTE_I2C0_SDA_PORT I2C0_SDA_PORT
#if (I2C0_SDA_LOC == 0)
#define RTE_I2C0_SDA_PIN     I2C0_SDA_PIN
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     1
#define RTE_I2C0_SDA_I2C_REN 6
#endif
#if (I2C0_SDA_LOC == 1)
#define RTE_I2C0_SDA_PIN     (I2C0_SDA_PIN + GPIO_MAX_PIN)
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     25
#define RTE_I2C0_SDA_I2C_REN 3
#endif
#if (I2C0_SDA_LOC == 2)
#define RTE_I2C0_SDA_PIN     (I2C0_SDA_PIN + GPIO_MAX_PIN)
#define RTE_I2C0_SDA_MUX     4
#define RTE_I2C0_SDA_PAD     32
#define RTE_I2C0_SDA_I2C_REN 10
#endif
//Pintool data
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
#ifndef I2C1_SCL_LOC
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
#else
//Pintool data
#define RTE_I2C1_SCL_PORT I2C1_SCL_PORT
#if (I2C1_SCL_LOC == 0)
#define RTE_I2C1_SCL_PIN I2C1_SCL_PIN
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 1
#define RTE_I2C1_SCL_REN 6
#endif
#if (I2C1_SCL_LOC == 1)
#define RTE_I2C1_SCL_PIN I2C1_SCL_PIN
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 0 //no pad
#define RTE_I2C1_SCL_REN 29
#endif
#if (I2C1_SCL_LOC == 2)
#define RTE_I2C1_SCL_PIN I2C1_SCL_PIN
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 14
#define RTE_I2C1_SCL_REN 50
#endif
#if (I2C1_SCL_LOC == 3)
#define RTE_I2C1_SCL_PIN I2C1_SCL_PIN
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 18
#define RTE_I2C1_SCL_REN 54
#endif
#if (I2C1_SCL_LOC == 5)
#define RTE_I2C1_SCL_PIN (I2C1_SCL_PIN + GPIO_MAX_PIN)
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 24
#define RTE_I2C1_SCL_REN 2
#endif
#if (I2C1_SCL_LOC == 6)
#define RTE_I2C1_SCL_PIN (I2C1_SCL_PIN + GPIO_MAX_PIN)
#define RTE_I2C1_SCL_MUX 5
#define RTE_I2C1_SCL_PAD 29
#define RTE_I2C1_SCL_REN 6
#endif
//Pintool data
#endif

// <o> I2C1_SCL Pin <0=>P0_7 <1=>P0_30 <2=>P0_51 <3=>P0_55  <4=>P0_65 <4=>P0_67 <4=>P0_71 <7=>P0_34
#ifndef I2C1_SDA_LOC
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
#else
//Pintool data
#define RTE_I2C1_SDA_PORT I2C1_SDA_PORT
#if (I2C1_SDA_LOC == 0)
#define RTE_I2C1_SDA_PIN I2C1_SDA_PIN
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 2
#define RTE_I2C1_SDA_REN 7
#endif
#if (I2C1_SDA_LOC == 1)
#define RTE_I2C1_SDA_PIN I2C1_SDA_PIN
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 0 //no pad
#define RTE_I2C1_SDA_REN 30
#endif
#if (I2C1_SDA_LOC == 2)
#define RTE_I2C1_SDA_PIN I2C1_SDA_PIN
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 15
#define RTE_I2C1_SDA_REN 51
#endif
#if (I2C1_SDA_LOC == 3)
#define RTE_I2C1_SDA_PIN I2C1_SDA_PIN
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 19
#define RTE_I2C1_SDA_REN 55
#endif
#if (I2C1_SDA_LOC == 4)
#define RTE_I2C1_SDA_PIN (I2C1_SDA_PIN + GPIO_MAX_PIN)
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 23
#define RTE_I2C1_SDA_REN 1
#endif
#if (I2C1_SDA_LOC == 5)
#define RTE_I2C1_SDA_PIN (I2C1_SDA_PIN + GPIO_MAX_PIN)
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 25
#define RTE_I2C1_SDA_REN 3
#endif
#if (I2C1_SDA_LOC == 6)
#define RTE_I2C1_SDA_PIN (I2C1_SDA_PIN + GPIO_MAX_PIN)
#define RTE_I2C1_SDA_MUX 5
#define RTE_I2C1_SDA_PAD 29
#define RTE_I2C1_SDA_REN 7
#endif
//Pintool data
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
#ifndef I2C2_SCL_LOC
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
#else
//Pintool data
#define RTE_I2C2_SCL_PORT I2C2_SCL_PORT
#define RTE_I2C2_SCL_PIN  I2C2_SCL_PIN
#define RTE_I2C2_SCL_MUX  4
#if (I2C2_SCL_LOC == 0)
#define RTE_I2C2_SCL_REN 7
#endif
#if (I2C2_SCL_LOC == 1)
#define RTE_I2C2_SCL_REN 8
#endif
//Pintool data
#endif

// <o> I2C2_SDA Pin <0=>P0_0 <1=>P0_4 <2=>P0_6 <3=>P0_9 <4=>P0_11 <5=>P0_12
#ifndef I2C2_SDA_LOC
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
#else
//Pintool data
#define RTE_I2C2_SDA_PORT I2C2_SDA_PORT
#define RTE_I2C2_SDA_PIN  I2C2_SDA_PIN
#define RTE_I2C2_SDA_MUX  4
#if (I2C2_SDA_LOC == 0)
#define RTE_I2C2_SDA_REN 6
#endif
#if (I2C2_SDA_LOC == 1)
#define RTE_I2C2_SDA_MUX 4
#endif
#if (I2C2_SDA_LOC == 2)
#define RTE_I2C2_SDA_MUX 4
#endif
//Pintool data
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
#ifndef GSPI_MASTER_CLK_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_CLK_PORT GSPI_MASTER_CLK_PORT
#define RTE_GSPI_MASTER_CLK_PIN  GSPI_MASTER_CLK_PIN
#define RTE_GSPI_MASTER_CLK_MUX  4
#if (GSPI_MASTER_CLK_LOC == 0)
#define RTE_GSPI_MASTER_CLK_PAD 3
#endif
#if (GSPI_MASTER_CLK_LOC == 1)
#define RTE_GSPI_MASTER_CLK_PAD 0
#endif
#if (GSPI_MASTER_CLK_LOC == 2)
#define RTE_GSPI_MASTER_CLK_PAD 10
#endif
#if (GSPI_MASTER_CLK_LOC == 3)
#define RTE_GSPI_MASTER_CLK_PAD 16
#endif
//Pintool data
#endif

//   <e> GSPI_MASTER_CS0
//   <o>  <0=>P0_9 <1=>P0_28 <2=>P0_49 <3=>P0_53
//   <i> CS0 of GSPI0
//   </e>
#ifndef GSPI_MASTER_CS0_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_CS0      1
#define RTE_GSPI_MASTER_CS0_PORT GSPI_MASTER_CS0_PORT
#define RTE_GSPI_MASTER_CS0_PIN  GSPI_MASTER_CS0_PIN
#define RTE_GSPI_MASTER_CS0_MUX  4
#if (GSPI_MASTER_CS0_LOC == 0)
#define RTE_GSPI_MASTER_CS0_PAD 4
#endif
#if (GSPI_MASTER_CS0_LOC == 1)
#define RTE_GSPI_MASTER_CS0_PAD 0
#endif
#if (GSPI_MASTER_CS0_LOC == 2)
#define RTE_GSPI_MASTER_CS0_PAD 13
#endif
#if (GSPI_MASTER_CS0_LOC == 3)
#define RTE_GSPI_MASTER_CS0_PAD 17
#endif
//Pintool data
#endif

//   <e> GSPI_MASTER_CS1
//   <o>  <0=>P0_10 <1=>P0_29 <2=>P0_50 <3=>P0_54
//   <i> CS1 of GSPI0
//   </e>
#ifndef GSPI_MASTER_CS1_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_CS1      1
#define RTE_GSPI_MASTER_CS1_PORT GSPI_MASTER_CS1_PORT
#define RTE_GSPI_MASTER_CS1_PIN  GSPI_MASTER_CS1_PIN
#define RTE_GSPI_MASTER_CS1_MUX  4
#if (GSPI_MASTER_CS1_LOC == 0)
#define RTE_GSPI_MASTER_CS1_PAD 5
#endif
#if (GSPI_MASTER_CS1_LOC == 1)
#define RTE_GSPI_MASTER_CS1_PAD 0
#endif
#if (GSPI_MASTER_CS1_LOC == 2)
#define RTE_GSPI_MASTER_CS1_PAD 14
#endif
#if (GSPI_MASTER_CS1_LOC == 3)
#define RTE_GSPI_MASTER_CS1_PAD 18
#endif
//Pintool data
#endif

//   <e> GSPI_MASTER_CS2
//   <o>  <0=>P0_15 <1=>P0_30 <2=>P0_51 <3=>P0_55
//   <i> CS2 of GSPI0
//   </e>
#ifndef GSPI_MASTER_CS2_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_CS2      1
#define RTE_GSPI_MASTER_CS2_PORT GSPI_MASTER_CS2_PORT
#define RTE_GSPI_MASTER_CS2_PIN  GSPI_MASTER_CS2_PIN
#define RTE_GSPI_MASTER_CS2_MUX  4
#if (GSPI_MASTER_CS2_LOC == 0)
#define RTE_GSPI_MASTER_CS2_PAD 8
#endif
#if (GSPI_MASTER_CS2_LOC == 1)
#define RTE_GSPI_MASTER_CS2_PAD 0
#endif
#if (GSPI_MASTER_CS2_LOC == 2)
#define RTE_GSPI_MASTER_CS2_PAD 15
#endif
#if (GSPI_MASTER_CS2_LOC == 3)
#define RTE_GSPI_MASTER_CS2_PAD 19
#endif
//Pintool data
#endif

//     <o> GSPI_MASTER_MOSI <0=>P0_12 <1=>P0_27 <2=>P0_48 <3=>P0_57 <4=>P0_6
//     <i> MOSI of GSPI0
#ifndef GSPI_MASTER_MOSI_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_MOSI_PORT GSPI_MASTER_MOSI_PORT
#define RTE_GSPI_MASTER_MOSI_PIN  GSPI_MASTER_MOSI_PIN
#define RTE_GSPI_MASTER_MOSI_MUX  4
#if (GSPI_MASTER_MOSI_LOC == 0)
#define RTE_GSPI_MASTER_MOSI_PAD 1
#endif
#if (GSPI_MASTER_MOSI_LOC == 1)
#define RTE_GSPI_MASTER_MOSI_PAD 0
#endif
#if (GSPI_MASTER_MOSI_LOC == 2)
#define RTE_GSPI_MASTER_MOSI_PAD 12
#endif
#if (GSPI_MASTER_MOSI_LOC == 3)
#define RTE_GSPI_MASTER_MOSI_PAD 21
#endif
//Pintool data
#endif

//     <o> GSPI_MASTER_MISO <0=>P0_11 <1=>P0_26 <2=>P0_47 <3=>P0_56
//     <i> MISO of GSPI0
#ifndef GSPI_MASTER_MISO_LOC
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
#else
//Pintool data
#define RTE_GSPI_MASTER_MISO_PORT GSPI_MASTER_MISO_PORT
#define RTE_GSPI_MASTER_MISO_PIN  GSPI_MASTER_MISO_PIN
#define RTE_GSPI_MASTER_MISO_MUX  4
#if (GSPI_MASTER_MISO_LOC == 0)
#define RTE_GSPI_MASTER_MISO_PAD 6
#endif
#if (GSPI_MASTER_MISO_LOC == 1)
#define RTE_GSPI_MASTER_MISO_PAD 0
#endif
#if (GSPI_MASTER_MISO_LOC == 2)
#define RTE_GSPI_MASTER_MISO_PAD 11
#endif
#if (GSPI_MASTER_MISO_LOC == 3)
#define RTE_GSPI_MASTER_MISO_PAD 20
#endif
//Pintool data
#endif

#if defined(HIGH_THROUGHPUT_EN) && (HIGH_THROUGHPUT_EN == ENABLE)
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

#ifndef SCT_IN0_LOC
#define RTE_SCT_IN_0_PORT_ID 0

#if (RTE_SCT_IN_0_PORT_ID == 0)
#define RTE_SCT_IN_0_PORT 0
#define RTE_SCT_IN_0_PIN  25
#else
#error "Invalid  RTE_SCT_IN_0_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_IN_0_PORT SCT_IN0_PORT
#define RTE_SCT_IN_0_PIN  SCT_IN0_PIN
//Pintool data
#endif
#define RTE_SCT_IN_0_MUX 9
#define RTE_SCT_IN_0_PAD 0 //no pad

//SCT_IN_1  <0=>GPIO_26  <1=>GPIO_65 <2=>GPIO_69
#ifndef SCT_IN1_LOC
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
#else
//Pintool data
#define RTE_SCT_IN_1_PORT SCT_IN1_PORT
#if (SCT_IN1_LOC == 0)
#define RTE_SCT_IN_1_PIN SCT_IN1_PIN
#define RTE_SCT_IN_1_MUX 9
#define RTE_SCT_IN_1_PAD 0 //no pad
#endif
#if (SCT_IN1_LOC == 1)
#define RTE_SCT_IN_1_PIN (SCT_IN1_PIN + GPIO_MAX_PIN)
#define RTE_SCT_IN_1_MUX 7
#define RTE_SCT_IN_1_PAD 23
#endif
//Pintool data
#endif

//SCT_IN_2  <0=>GPIO_27  <1=>GPIO_66 <2=>GPIO_70
#ifndef SCT_IN2_LOC
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
#else
//Pintool data
#define RTE_SCT_IN_2_PORT SCT_IN2_PORT
#if (SCT_IN2_LOC == 0)
#define RTE_SCT_IN_2_PIN SCT_IN2_PIN
#define RTE_SCT_IN_2_MUX 9
#define RTE_SCT_IN_2_PAD 0 //no pad
#endif
#if (SCT_IN2_LOC == 1)
#define RTE_SCT_IN_2_PIN (SCT_IN2_PIN + GPIO_MAX_PIN)
#define RTE_SCT_IN_2_MUX 7
#define RTE_SCT_IN_2_PAD 24
#endif
#if (SCT_IN2_LOC == 2)
#define RTE_SCT_IN_2_PIN (SCT_IN2_PIN + GPIO_MAX_PIN)
#define RTE_SCT_IN_2_MUX 9
#define RTE_SCT_IN_2_PAD 28
#endif
//Pintool data
#endif

//SCT_IN_3  <0=>GPIO_28  <1=>GPIO_67 <2=>GPIO_71
#ifndef SCT_IN3_LOC
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
#else
//Pintool data
#define RTE_SCT_IN_3_PORT SCT_IN3_PORT
#if (SCT_IN3_LOC == 0)
#define RTE_SCT_IN_3_PIN SCT_IN3_PIN
#define RTE_SCT_IN_3_MUX 9
#define RTE_SCT_IN_3_PAD 0 //no pad
#endif
#if (SCT_IN3_LOC == 1)
#define RTE_SCT_IN_3_PIN (SCT_IN3_PIN + GPIO_MAX_PIN)
#define RTE_SCT_IN_3_MUX 7
#define RTE_SCT_IN_3_PAD 25
#endif
#if (SCT_IN3_LOC == 2)
#define RTE_SCT_IN_3_PIN (SCT_IN3_PIN + GPIO_MAX_PIN)
#define RTE_SCT_IN_3_MUX 9
#define RTE_SCT_IN_3_PAD 29
#endif
//Pintool data
#endif

//     SCT_OUT_0 <0=>GPIO_29 <1=>GPIO_68
#ifndef SCT_OUT0_LOC
#define RTE_SCT_OUT_0_PORT_ID 0
#if (RTE_SCT_OUT_0_PORT_ID == 0)
#define RTE_SCT_OUT_0_PORT 0
#define RTE_SCT_OUT_0_PIN  29
#else
#error "Invalid  RTE_SCT_OUT_0_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_0_PORT SCT_OUT0_PORT
#define RTE_SCT_OUT_0_PIN  SCT_OUT0_PIN
//Pintool data
#endif
#define RTE_SCT_OUT_0_MUX 9
#define RTE_SCT_OUT_0_PAD 0 //no pad

//  SCT_OUT_1 <0=>GPIO_30  <1=>GPIO_69
#ifndef SCT_OUT1_LOC
#define RTE_SCT_OUT_1_PORT_ID 0
#if (RTE_SCT_OUT_1_PORT_ID == 0)
#define RTE_SCT_OUT_1_PORT 0
#define RTE_SCT_OUT_1_PIN  30
#else
#error "Invalid  RTE_SCT_OUT_1_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_1_PORT SCT_OUT1_PORT
#define RTE_SCT_OUT_1_PIN  SCT_OUT1_PIN
//Pintool data
#endif
#define RTE_SCT_OUT_1_MUX 9
#define RTE_SCT_OUT_1_PAD 0 //no pad

/// SCT_OUT_2 <0=>GPIO_70 <1=>GPIO_8
#ifndef SCT_OUT2_LOC
#define RTE_SCT_OUT_2_PORT_ID 0
#if ((RTE_SCT_OUT_2_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_2_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_2_PORT_ID == 0)
#define RTE_SCT_OUT_2_PORT 0
#define RTE_SCT_OUT_2_PIN  70
#elif (RTE_SCT_OUT_2_PORT_ID == 1)
#define RTE_SCT_OUT_2_PORT 0
#define RTE_SCT_OUT_2_PIN  8
#define RTE_SCT_OUT_2_MUX  12
#define RTE_SCT_OUT_2_PAD  3
#else
#error "Invalid  RTE_SCT_OUT_2_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_2_PORT SCT_OUT2_PORT
#define RTE_SCT_OUT_2_PIN  (SCT_OUT2_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_2_MUX 7
#define RTE_SCT_OUT_2_PAD 28

/**/
//SCT_OUT_3 <0=>GPIO_71 <1=>GPIO_9
#ifndef SCT_OUT3_LOC
#define RTE_SCT_OUT_3_PORT_ID 0
#if ((RTE_SCT_OUT_3_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_3_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_3_PORT_ID == 0)
#define RTE_SCT_OUT_3_PORT 0
#define RTE_SCT_OUT_3_PIN  71
#elif (RTE_SCT_OUT_3_PORT_ID == 1)
#define RTE_SCT_OUT_3_PORT 0
#define RTE_SCT_OUT_3_PIN  9
#define RTE_SCT_OUT_3_MUX  12
#define RTE_SCT_OUT_3_PAD  4
#else
#error "Invalid  RTE_SCT_OUT_3_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_3_PORT SCT_OUT3_PORT
#define RTE_SCT_OUT_3_PIN  (SCT_OUT3_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_3_MUX 7
#define RTE_SCT_OUT_3_PAD 29

//SCT_OUT_4 <0=>GPIO_72 <1=>GPIO_68
#ifndef SCT_OUT4_LOC
#define RTE_SCT_OUT_4_PORT_ID 0
#if ((RTE_SCT_OUT_4_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_4_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_4_PORT_ID == 0)
/**/
#define RTE_SCT_OUT_4_PORT 0
#define RTE_SCT_OUT_4_PIN  72
#else
#error "Invalid  RTE_SCT_OUT_4_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_4_PORT SCT_OUT4_PORT
#define RTE_SCT_OUT_4_PIN  (SCT_OUT4_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_4_MUX 7
#define RTE_SCT_OUT_4_PAD 30

//SCT_OUT_5 <0=>GPIO_73 <1=>GPIO_69
#ifndef SCT_OUT5_LOC
#define RTE_SCT_OUT_5_PORT_ID 0
#if ((RTE_SCT_OUT_5_PORT_ID == 1))
#error "Invalid RTE_SCT_OUT_5_PIN pin Configuration!"
#endif

#if (RTE_SCT_OUT_5_PORT_ID == 0)
#define RTE_SCT_OUT_5_PORT 2
#define RTE_SCT_OUT_5_PIN  73
#else
#error "Invalid  RTE_SCT_OUT_5_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_5_PORT SCT_OUT5_PORT
#define RTE_SCT_OUT_5_PIN  (SCT_OUT5_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_5_MUX 7
#define RTE_SCT_OUT_5_PAD 31

//SCT_OUT_6 <0=>GPIO_74 <1=>GPIO_70
#ifndef SCT_OUT6_LOC
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
#else
//Pintool data
#define RTE_SCT_OUT_6_PORT SCT_OUT6_PORT
#define RTE_SCT_OUT_6_PIN  (SCT_OUT6_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_6_MUX 7
#define RTE_SCT_OUT_6_PAD 32

// SCT_OUT_7  <0=>GPIO_75 <1=>GPIO_71
#ifndef SCT_OUT7_LOC
#define RTE_SCT_OUT_7_PORT_ID 0

#if (RTE_SCT_OUT_7_PORT_ID == 0)
#define RTE_SCT_OUT_7_PORT 0
#define RTE_SCT_OUT_7_PIN  75
#else
#error "Invalid  RTE_SCT_OUT_7_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_SCT_OUT_7_PORT SCT_OUT7_PORT
#define RTE_SCT_OUT_7_PIN  (SCT_OUT7_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_SCT_OUT_7_MUX 7
#define RTE_SCT_OUT_7_PAD 33

//  SIO //
//<>  Serial Input Output
//SIO_0 <0=>GPIO_6 <1=>GPIO_25  <2=>GPIO_64 <3=>GPIO_72
#ifndef SIO_0_LOC
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
#else
//Pintool data
#define RTE_SIO_0_PORT SIO_0_PORT
#define RTE_SIO_0_MUX  1
#if (SIO_0_LOC == 0)
#define RTE_SIO_0_PIN SIO_0_PIN
#define RTE_SIO_0_PAD 1
#endif
#if (SIO_0_LOC == 1)
#define RTE_SIO_0_PIN SIO_0_PIN
#define RTE_SIO_0_PAD 0
#endif
#if (SIO_0_LOC == 2)
#define RTE_SIO_0_PIN (SIO_0_PIN + GPIO_MAX_PIN)
#define RTE_SIO_0_PAD 30
#endif
//Pintool data
#endif

//SIO_1 <0=>GPIO_7 <1=>GPIO_26  <2=>GPIO_65 <3=>GPIO_73
#ifndef SIO_1_LOC
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
#else
//Pintool data
#define RTE_SIO_1_PORT SIO_1_PORT
#define RTE_SIO_1_MUX  1
#if (SIO_1_LOC == 0)
#define RTE_SIO_1_PIN SIO_1_PIN
#define RTE_SIO_1_PAD 2
#endif
#if (SIO_1_LOC == 1)
#define RTE_SIO_1_PIN SIO_1_PIN
#define RTE_SIO_1_PAD 0
#endif
#if (SIO_1_LOC == 2)
#define RTE_SIO_1_PIN (SIO_1_PIN + GPIO_MAX_PIN)
#define RTE_SIO_1_PAD 23
#endif
#if (SIO_1_LOC == 3)
#define RTE_SIO_1_PIN (SIO_1_PIN + GPIO_MAX_PIN)
#define RTE_SIO_1_PAD 31
#endif
//Pintool data
#endif

// SIO_2  <0=>GPIO_8 <1=>GPIO_27  <2=>GPIO_66 <3=>GPIO_74
#ifndef SIO_2_LOC
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
#else
//Pintool data
#define RTE_SIO_2_PORT SIO_2_PORT
#define RTE_SIO_2_MUX  1
#if (SIO_2_LOC == 0)
#define RTE_SIO_2_PIN SIO_2_PIN
#define RTE_SIO_2_PAD 3
#endif
#if (SIO_2_LOC == 1)
#define RTE_SIO_2_PIN SIO_2_PIN
#define RTE_SIO_2_PAD 0
#endif
#if (SIO_2_LOC == 2)
#define RTE_SIO_2_PIN (SIO_2_PIN + GPIO_MAX_PIN)
#define RTE_SIO_2_PAD 24
#endif
#if (SIO_2_LOC == 3)
#define RTE_SIO_2_PIN (SIO_2_PIN + GPIO_MAX_PIN)
#define RTE_SIO_2_PAD 32
#endif
//Pintool data
#endif

//SIO_3 <0=>GPIO_9 <1=>GPIO_28  <2=>GPIO_67 <3=>GPIO_75
#ifndef SIO_3_LOC
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
#else
//Pintool data
#define RTE_SIO_3_PORT SIO_3_PORT
#define RTE_SIO_3_MUX  1
#if (SIO_3_LOC == 0)
#define RTE_SIO_3_PIN SIO_3_PIN
#define RTE_SIO_3_PAD 4
#endif
#if (SIO_3_LOC == 1)
#define RTE_SIO_3_PIN SIO_3_PIN
#define RTE_SIO_3_PAD 0
#endif
#if (SIO_3_LOC == 2)
#define RTE_SIO_3_PIN (SIO_3_PIN + GPIO_MAX_PIN)
#define RTE_SIO_3_PAD 25
#endif
#if (SIO_3_LOC == 3)
#define RTE_SIO_3_PIN (SIO_3_PIN + GPIO_MAX_PIN)
#define RTE_SIO_3_PAD 33
#endif
//Pintool data
#endif

//SIO_4 <0=>GPIO_10 <1=>GPIO_29 <2=>GPIO_68
#ifndef SIO_4_LOC
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
#else
//Pintool data
#define RTE_SIO_4_PORT SIO_4_PORT
#define RTE_SIO_4_PIN  SIO_4_PIN
#define RTE_SIO_4_MUX  1
#if (SIO_4_LOC == 0)
#define RTE_SIO_4_PAD 5
#endif
#if (SIO_4_LOC == 1)
#define RTE_SIO_4_PAD 0
#endif
//Pintool data
#endif

// SIO_5  <0=>GPIO_11 <1=>GPIO_30 <2=>GPIO_69
#ifndef SIO_5_LOC
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
#else
//Pintool data
#define RTE_SIO_5_PORT SIO_5_PORT
#define RTE_SIO_5_PIN  SIO_5_PIN
#define RTE_SIO_5_MUX  1
#if (SIO_5_LOC == 0)
#define RTE_SIO_5_PAD 6
#endif
#if (SIO_5_LOC == 1)
#define RTE_SIO_5_PAD 0
#endif
//Pintool data
#endif

// SIO_6  GPIO_70
#ifndef SIO_6_LOC
#define RTE_SIO_6_PORT 0
#define RTE_SIO_6_PIN  70
#else
#define RTE_SIO_6_PORT SIO_6_PORT
#define RTE_SIO_6_PIN  (SIO_6_PIN + GPIO_MAX_PIN)
#endif
#define RTE_SIO_6_MUX 1
#define RTE_SIO_6_PAD 28

// SIO_7  <0=>GPIO_15 <1=>GPIO_71
#ifndef SIO_7_LOC
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
#else
//Pintool data
#define RTE_SIO_7_PORT SIO_6_PORT
#define RTE_SIO_7_MUX  1
#if (SIO_7_LOC == 0)
#define RTE_SIO_7_PIN SIO_7_PIN
#define RTE_SIO_7_PAD 8
#endif
#if (SIO_7_LOC == 1)
#define RTE_SIO_7_PIN (SIO_7_PIN + GPIO_MAX_PIN)
#define RTE_SIO_7_PAD 29
#endif
//Pintool data
#endif

//<> Pulse Width Modulation
//PWM_1H  <0=>GPIO_7  <1=>GPIO_64 <2=>GPIO_65
#ifndef PWM_H1_LOC
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
#else
//Pintool data
#define RTE_PWM_1H_PORT PWM_H1_PORT
#if (PWM_H1_LOC == 0)
#define RTE_PWM_1H_PIN PWM_H1_PIN
#define RTE_PWM_1H_MUX 10
#define RTE_PWM_1H_PAD 2
#endif
#if (PWM_H1_LOC == 1)
#define RTE_PWM_1H_PIN (PWM_H1_PIN + GPIO_MAX_PIN)
#define RTE_PWM_1H_MUX 8
#define RTE_PWM_1H_PAD 22
#endif
//Pintool data
#endif

// PWM_1L <0=>GPIO_6  <1=>GPIO_64 <2=>GPIO_64
#ifndef PWM_L1_LOC
#ifdef SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER
#define RTE_PWM_1L_PORT_ID 0
#else
#define RTE_PWM_1L_PORT_ID 1
#endif

#if (RTE_PWM_1L_PORT_ID == 0)
#define RTE_PWM_1L_PORT 0
#define RTE_PWM_1L_PIN  6
#else
#error "Invalid  RTE_PWM_1L_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_PWM_1L_PORT PWM_L1_PORT
#define RTE_PWM_1L_PIN  PWM_L1_PIN
//Pintool data
#endif
#define RTE_PWM_1L_MUX 10
#define RTE_PWM_1L_PAD 1

//PWM_2H  <0=>GPIO_9  <1=>GPIO_67 <2=>GPIO_69
#ifndef PWM_H2_LOC
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
#else
//Pintool data
#define RTE_PWM_2H_PORT PWM_H2_PORT
#if (PWM_H2_LOC == 0)
#define RTE_PWM_2H_PIN PWM_H2_PIN
#define RTE_PWM_2H_MUX 10
#define RTE_PWM_2H_PAD 4
#endif
#if (PWM_H2_LOC == 1)
#define RTE_PWM_2H_PIN (PWM_H2_PIN + GPIO_MAX_PIN)
#define RTE_PWM_2H_MUX 8
#define RTE_PWM_2H_PAD 25
#endif
//Pintool data
#endif

// PWM_2L <0=>GPIO_8  <1=>GPIO_66 <2=>GPIO_68
#ifndef PWM_L2_LOC
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
#else
//Pintool data
#define RTE_PWM_2L_PORT PWM_L2_PORT
#if (PWM_L2_LOC == 0)
#define RTE_PWM_2L_PIN PWM_L2_PIN
#define RTE_PWM_2L_MUX 10
#define RTE_PWM_2L_PAD 3
#endif
#if (PWM_L2_LOC == 1)
#define RTE_PWM_2L_PIN (PWM_L2_PIN + GPIO_MAX_PIN)
#define RTE_PWM_2L_MUX 8
#define RTE_PWM_2L_PAD 24
#endif
//Pintool data
#endif

// PWM_3H <0=>GPIO_11   <1=>GPIO_69
#ifndef PWM_H3_LOC
#define RTE_PWM_3H_PORT_ID 0
#if (RTE_PWM_3H_PORT_ID == 0)
#define RTE_PWM_3H_PORT 0
#define RTE_PWM_3H_PIN  11
#else
#error "Invalid  RTE_PWM_3H_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_PWM_3H_PORT PWM_H3_PORT
#define RTE_PWM_3H_PIN  PWM_H3_PIN
//Pintool data
#endif
#define RTE_PWM_3H_MUX 10
#define RTE_PWM_3H_PAD 6

// PWM_3L <0=>GPIO_10 <1=>GPIO_68
#ifndef PWM_L3_LOC
#define RTE_PWM_3L_PORT_ID 0

#if (RTE_PWM_3L_PORT_ID == 0)
#define RTE_PWM_3L_PORT 0
#define RTE_PWM_3L_PIN  10
#else
#error "Invalid  RTE_PWM_3L_PIN Pin Configuration!"
#endif
#else
//Pintool data
#define RTE_PWM_3L_PORT PWM_L3_PORT
#define RTE_PWM_3L_PIN  PWM_L3_PIN
//Pintool data
#endif
#define RTE_PWM_3L_MUX 10
#define RTE_PWM_3L_PAD 5

// PWM_4H <0=>GPIO_15 <1=>GPIO_71
#ifndef PWM_H4_LOC
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
#else
//Pintool data
#define RTE_PWM_4H_PORT PWM_H4_PORT
#if (PWM_H4_LOC == 0)
#define RTE_PWM_4H_PIN PWM_H4_PIN
#define RTE_PWM_4H_MUX 10
#define RTE_PWM_4H_PAD 8
#endif
#if (PWM_H4_LOC == 1)
#define RTE_PWM_4H_PIN (PWM_H4_PIN + GPIO_MAX_PIN)
#define RTE_PWM_4H_MUX 8
#define RTE_PWM_4H_PAD 29
#endif
//Pintool data
#endif

// PWM_4H <0=>GPIO_12 <1=>GPIO_70
#ifndef PWM_L4_LOC
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
#else
//Pintool data
#define RTE_PWM_4L_PORT PWM_L4_PORT
#if (PWM_L4_LOC == 0)
#define RTE_PWM_4L_PIN PWM_L4_PIN
#define RTE_PWM_4L_MUX 10
#define RTE_PWM_4L_PAD 7
#endif
#if (PWM_L4_LOC == 1)
#define RTE_PWM_4L_PIN (PWM_L4_PIN + GPIO_MAX_PIN)
#define RTE_PWM_4L_MUX 8
#define RTE_PWM_4L_PAD 28
#endif
//Pintool data
#endif

// PWM_FAULTA <0=>GPIO_25 <1=>GPIO_68 <1=>GPIO_73
#ifndef PWM_FAULTA_LOC
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
#else
//Pintool data
#define RTE_PWM_FAULTA_PORT PWM_FAULTA_PORT
#if (PWM_FAULTA_LOC == 0)
#define RTE_PWM_FAULTA_PIN PWM_FAULTA_PIN
#define RTE_PWM_FAULTA_MUX 10
#define RTE_PWM_FAULTA_PAD 0 //no pad
#endif
#if (PWM_FAULTA_LOC == 2)
#define RTE_PWM_FAULTA_PIN (PWM_FAULTA_PIN + GPIO_MAX_PIN)
#define RTE_PWM_FAULTA_MUX 8
#define RTE_PWM_FAULTA_PAD 31
#endif
//Pintool data
#endif

// PWM_FAULTB <0=>GPIO_26 <1=>GPIO_69 <1=>GPIO_74
#ifndef PWM_FAULTB_LOC
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
#else
//Pintool data
#define RTE_PWM_FAULTB_PORT PWM_FAULTB_PORT
#if (PWM_FAULTB_LOC == 0)
#define RTE_PWM_FAULTB_PIN PWM_FAULTB_PIN
#define RTE_PWM_FAULTB_MUX 10
#define RTE_PWM_FAULTB_PAD 0 //no pad
#endif
#if (PWM_FAULTB_LOC == 2)
#define RTE_PWM_FAULTB_PIN (PWM_FAULTB_PIN + GPIO_MAX_PIN)
#define RTE_PWM_FAULTB_MUX 8
#define RTE_PWM_FAULTB_PAD 32
#endif
//Pintool data
#endif

//PWM_SLP_EVENT_TRIG   GPIO_72
#ifndef PWM_SLEEPTRIG_LOC
#define RTE_PWM_SLP_EVENT_TRIG_PORT 0
#define RTE_PWM_SLP_EVENT_TRIG_PIN  72
#else
//Pintool data
#define RTE_PWM_SLP_EVENT_TRIG_PORT PWM_SLEEP_EVENT_TRIG_PORT
#define RTE_PWM_SLP_EVENT_TRIG_PIN  (PWM_SLEEP_EVENT_TRIG_PIN + GPIO_MAX_PIN)
//Pintool data
#endif
#define RTE_PWM_SLP_EVENT_TRIG_MUX 8
#define RTE_PWM_SLP_EVENT_TRIG_PAD 30

//PWM_TMR_EXT_TRIG_1 <0=>GPIO_27 <1=>GPIO_51 <2=>GPIO_70 <3=>GPIO_75
#ifndef PWM_EXTTRIG1_LOC
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
#else
//Pintool data
#define RTE_PWM_TMR_EXT_TRIG_1_PORT PWM_TMR_EXT_TRIG_1_PORT
#if (PWM_EXTTRIG1_LOC == 0)
#define RTE_PWM_TMR_EXT_TRIG_1_PIN PWM_TMR_EXT_TRIG_1_PIN
#define RTE_PWM_TMR_EXT_TRIG_1_MUX 10
#define RTE_PWM_TMR_EXT_TRIG_1_PAD 0 //no pad
#endif
#if (PWM_EXTTRIG1_LOC == 1)
#define RTE_PWM_TMR_EXT_TRIG_1_PIN PWM_TMR_EXT_TRIG_1_PIN
#define RTE_PWM_TMR_EXT_TRIG_1_MUX 8
#define RTE_PWM_TMR_EXT_TRIG_1_PAD 15
#endif
#if (PWM_EXTTRIG1_LOC == 2)
#define RTE_PWM_TMR_EXT_TRIG_1_PIN (PWM_TMR_EXT_TRIG_1_PIN + GPIO_MAX_PIN)
#define RTE_PWM_TMR_EXT_TRIG_1_MUX 10
#define RTE_PWM_TMR_EXT_TRIG_1_PAD 28
#endif
#if (PWM_EXTTRIG1_LOC == 3)
#define RTE_PWM_TMR_EXT_TRIG_1_PIN (PWM_TMR_EXT_TRIG_1_PIN + GPIO_MAX_PIN)
#define RTE_PWM_TMR_EXT_TRIG_1_MUX 8
#define RTE_PWM_TMR_EXT_TRIG_1_PAD 33
#endif
//Pintool data
#endif

//PWM_TMR_EXT_TRIG_2 <0=>GPIO_28 <1=>GPIO_54 <2=>GPIO_71
#ifndef PWM_EXTTRIG2_LOC
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
#else
//Pintool data
#define RTE_PWM_TMR_EXT_TRIG_2_PORT PWM_TMR_EXT_TRIG_2_PORT
#if (PWM_EXTTRIG2_LOC == 0)
#define RTE_PWM_TMR_EXT_TRIG_2_PIN PWM_TMR_EXT_TRIG_2_PIN
#define RTE_PWM_TMR_EXT_TRIG_2_MUX 10
#define RTE_PWM_TMR_EXT_TRIG_2_PAD 0 //no pad
#endif
#if (PWM_EXTTRIG2_LOC == 1)
#define RTE_PWM_TMR_EXT_TRIG_2_PIN PWM_TMR_EXT_TRIG_2_PIN
#define RTE_PWM_TMR_EXT_TRIG_2_MUX 8
#define RTE_PWM_TMR_EXT_TRIG_2_PAD 18
#endif
#if (PWM_EXTTRIG2_LOC == 2)
#define RTE_PWM_TMR_EXT_TRIG_2_PIN (PWM_TMR_EXT_TRIG_2_PIN + GPIO_MAX_PIN)
#define RTE_PWM_TMR_EXT_TRIG_2_MUX 10
#define RTE_PWM_TMR_EXT_TRIG_2_PAD 29
#endif
//Pintool data
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

//ADC START

#ifndef ADC_P0_LOC
#define RTE_ADC_P0_PORT 0
#define RTE_ADC_P0_PIN  0
#else
#define RTE_ADC_P0_PORT ADC_P0_PORT
#define RTE_ADC_P0_PIN  ADC_P0_PIN
#endif
#define RTE_ADC_P0_MUX 1

#ifndef ADC_N0_LOC
#define RTE_ADC_N0_PORT 0
#define RTE_ADC_N0_PIN  1
#else
#define RTE_ADC_N0_PORT ADC_N0_PORT
#define RTE_ADC_N0_PIN  ADC_N0_PIN
#endif
#define RTE_ADC_N0_MUX 1

#ifndef ADC_P1_LOC
#define RTE_ADC_P1_PORT 0
#define RTE_ADC_P1_PIN  2
#else
#define RTE_ADC_P1_PORT ADC_P1_PORT
#define RTE_ADC_P1_PIN  ADC_P1_PIN
#endif
#define RTE_ADC_P1_MUX 1

#ifndef ADC_N1_LOC
#define RTE_ADC_N1_PORT 0
#define RTE_ADC_N1_PIN  3
#else
#define RTE_ADC_N1_PORT ADC_N1_PORT
#define RTE_ADC_N1_PIN  ADC_N1_PIN
#endif
#define RTE_ADC_N1_MUX 1

#ifndef ADC_P2_LOC
#define RTE_ADC_P2_PORT 0
#define RTE_ADC_P2_PIN  4
#else
#define RTE_ADC_P2_PORT ADC_P2_PORT
#define RTE_ADC_P2_PIN  ADC_P2_PIN
#endif
#define RTE_ADC_P2_MUX 1

#ifndef ADC_N2_LOC
#define RTE_ADC_N2_PORT 0
#define RTE_ADC_N2_PIN  5
#else
#define RTE_ADC_N2_PORT ADC_N2_PORT
#define RTE_ADC_N2_PIN  ADC_N2_PIN
#endif
#define RTE_ADC_N2_MUX 1

#ifndef ADC_P3_LOC
#define RTE_ADC_P3_PORT 0
#define RTE_ADC_P3_PIN  6
#else
#define RTE_ADC_P3_PORT ADC_P3_PORT
#define RTE_ADC_P3_PIN  ADC_P3_PIN
#endif
#define RTE_ADC_P3_MUX 1

#ifndef ADC_N3_LOC
#define RTE_ADC_N3_PORT 0
#define RTE_ADC_N3_PIN  11
#else
#define RTE_ADC_N3_PORT ADC_N3_PORT
#define RTE_ADC_N3_PIN  ADC_N3_PIN
#endif
#define RTE_ADC_N3_MUX 1

#ifndef ADC_P4_LOC
#define RTE_ADC_P4_PORT 0
#define RTE_ADC_P4_PIN  8
#else
#define RTE_ADC_P4_PORT ADC_P4_PORT
#define RTE_ADC_P4_PIN  ADC_P4_PIN
#endif
#define RTE_ADC_P4_MUX 1

#ifndef ADC_N4_LOC
#define RTE_ADC_N4_PORT 0
#define RTE_ADC_N4_PIN  9
#else
#define RTE_ADC_N4_PORT ADC_N4_PORT
#define RTE_ADC_N4_PIN  ADC_N4_PIN
#endif
#define RTE_ADC_N4_MUX 1

#ifndef ADC_P5_LOC
#define RTE_ADC_P5_PORT 0
#define RTE_ADC_P5_PIN  10
#else
#define RTE_ADC_P5_PORT ADC_P5_PORT
#define RTE_ADC_P5_PIN  ADC_P5_PIN
#endif
#define RTE_ADC_P5_MUX 1

#ifndef ADC_N5_LOC
#define RTE_ADC_N5_PORT 0
#define RTE_ADC_N5_PIN  7
#else
#define RTE_ADC_N5_PORT ADC_N5_PORT
#define RTE_ADC_N5_PIN  ADC_N5_PIN
#endif
#define RTE_ADC_N5_MUX 1

#ifndef ADC_P6_LOC
#define RTE_ADC_P6_PORT 0
#define RTE_ADC_P6_PIN  25
#else
#define RTE_ADC_P6_PORT ADC_P6_PORT
#define RTE_ADC_P6_PIN  ADC_P6_PIN
#endif
#define RTE_ADC_P6_MUX 1
#define RTE_ADC_P6_PAD 0

#ifndef ADC_N6_LOC
#define RTE_ADC_N6_PORT 0
#define RTE_ADC_N6_PIN  26
#else
#define RTE_ADC_N6_PORT ADC_N6_PORT
#define RTE_ADC_N6_PIN  ADC_N6_PIN
#endif
#define RTE_ADC_N6_MUX 1
#define RTE_ADC_N6_PAD 0

#ifndef ADC_P7_LOC
#define RTE_ADC_P7_PORT 0
#define RTE_ADC_P7_PIN  27
#else
#define RTE_ADC_P7_PORT ADC_P7_PORT
#define RTE_ADC_P7_PIN  ADC_P7_PIN
#endif
#define RTE_ADC_P7_MUX 1
#define RTE_ADC_P7_PAD 0

#ifndef ADC_N7_LOC
#define RTE_ADC_N7_PORT 0
#define RTE_ADC_N7_PIN  28
#else
#define RTE_ADC_N7_PORT ADC_N7_PORT
#define RTE_ADC_N7_PIN  ADC_N7_PIN
#endif
#define RTE_ADC_N7_MUX 1
#define RTE_ADC_N7_PAD 0

#ifndef ADC_P8_LOC
#define RTE_ADC_P8_PORT 0
#define RTE_ADC_P8_PIN  29
#else
#define RTE_ADC_P8_PORT ADC_P8_PORT
#define RTE_ADC_P8_PIN  ADC_P8_PIN
#endif
#define RTE_ADC_P8_MUX 1
#define RTE_ADC_P8_PAD 0

#ifndef ADC_N8_LOC
#define RTE_ADC_N8_PORT 0
#define RTE_ADC_N8_PIN  30
#else
#define RTE_ADC_N8_PORT ADC_N8_PORT
#define RTE_ADC_N8_PIN  ADC_N8_PIN
#endif
#define RTE_ADC_N8_MUX 1
#define RTE_ADC_N8_PAD 0

#ifndef ADC_P10_LOC
#define RTE_ADC_P10_PORT 0
#define RTE_ADC_P10_PIN  1
#else
#define RTE_ADC_P10_PORT ADC_P10_PORT
#define RTE_ADC_P10_PIN  ADC_P10_PIN
#endif
#define RTE_ADC_P10_MUX 1

#ifndef ADC_P11_LOC
#define RTE_ADC_P11_PORT 0
#define RTE_ADC_P11_PIN  3
#else
#define RTE_ADC_P11_PORT ADC_P11_PORT
#define RTE_ADC_P11_PIN  ADC_P11_PIN
#endif
#define RTE_ADC_P11_MUX 1

#ifndef ADC_P12_LOC
#define RTE_ADC_P12_PORT 0
#define RTE_ADC_P12_PIN  5
#else
#define RTE_ADC_P12_PORT ADC_P12_PORT
#define RTE_ADC_P12_PIN  ADC_P12_PIN
#endif
#define RTE_ADC_P12_MUX 1

#ifndef ADC_P13_LOC
#define RTE_ADC_P13_PORT 0
#define RTE_ADC_P13_PIN  11
#else
#define RTE_ADC_P13_PORT ADC_P13_PORT
#define RTE_ADC_P13_PIN  ADC_P13_PIN
#endif
#define RTE_ADC_P13_MUX 1

#ifndef ADC_P14_LOC
#define RTE_ADC_P14_PORT 0
#define RTE_ADC_P14_PIN  9
#else
#define RTE_ADC_P14_PORT ADC_P14_PORT
#define RTE_ADC_P14_PIN  ADC_P14_PIN
#endif
#define RTE_ADC_P14_MUX 1

#ifndef ADC_P15_LOC
#define RTE_ADC_P15_PORT 0
#define RTE_ADC_P15_PIN  7
#else
#define RTE_ADC_P15_PORT ADC_P15_PORT
#define RTE_ADC_P15_PIN  ADC_P15_PIN
#endif
#define RTE_ADC_P15_MUX 1

#ifndef ADC_P16_LOC
#define RTE_ADC_P16_PORT 0
#define RTE_ADC_P16_PIN  26
#else
#define RTE_ADC_P16_PORT ADC_P16_PORT
#define RTE_ADC_P16_PIN  ADC_P16_PIN
#endif
#define RTE_ADC_P16_MUX 1
#define RTE_ADC_P16_PAD 0

#ifndef ADC_P17_LOC
#define RTE_ADC_P17_PORT 0
#define RTE_ADC_P17_PIN  28
#else
#define RTE_ADC_P17_PORT ADC_P17_PORT
#define RTE_ADC_P17_PIN  ADC_P17_PIN
#endif
#define RTE_ADC_P17_MUX 1
#define RTE_ADC_P17_PAD 0

#ifndef ADC_P18_LOC
#define RTE_ADC_P18_PORT 0
#define RTE_ADC_P18_PIN  30
#else
#define RTE_ADC_P18_PORT ADC_P18_PORT
#define RTE_ADC_P18_PIN  ADC_P18_PIN
#endif
#define RTE_ADC_P18_MUX 1
#define RTE_ADC_P18_PAD 0

//ADC END

//COMPARATOR START

#ifndef COMP1_P0_LOC
#define RTE_COMP1_P0_PORT 0
#define RTE_COMP1_P0_PIN  0
#else
#define RTE_COMP1_P0_PORT COMP1_P0_PORT
#define RTE_COMP1_P0_PIN  COMP1_P0_PIN
#endif
#define RTE_COMP1_P0_MUX 0

#ifndef COMP1_N0_LOC
#define RTE_COMP1_N0_PORT 0
#define RTE_COMP1_N0_PIN  1
#else
#define RTE_COMP1_N0_PORT COMP1_N0_PORT
#define RTE_COMP1_N0_PIN  COMP1_N0_PIN
#endif
#define RTE_COMP1_N0_MUX 0

#ifndef COMP1_P1_LOC
#define RTE_COMP1_P1_PORT 0
#define RTE_COMP1_P1_PIN  5
#else
#define RTE_COMP1_P1_PORT COMP1_P1_PORT
#define RTE_COMP1_P1_PIN  COMP1_P1_PIN
#endif
#define RTE_COMP1_P1_MUX 0

#ifndef COMP1_N1_LOC
#define RTE_COMP1_N1_PORT 0
#define RTE_COMP1_N1_PIN  4
#else
#define RTE_COMP1_N1_PORT COMP1_N1_PORT
#define RTE_COMP1_N1_PIN  COMP1_N1_PIN
#endif
#define RTE_COMP1_N1_MUX 0

#ifndef COMP2_P0_LOC
#define RTE_COMP2_P0_PORT 0
#define RTE_COMP2_P0_PIN  2
#else
#define RTE_COMP2_P0_PORT COMP2_P0_PORT
#define RTE_COMP2_P0_PIN  COMP2_P0_PIN
#endif
#define RTE_COMP2_P0_MUX 0

#ifndef COMP2_N0_LOC
#define RTE_COMP2_N0_PORT 0
#define RTE_COMP2_N0_PIN  3
#else
#define RTE_COMP2_N0_PORT COMP2_N0_PORT
#define RTE_COMP2_N0_PIN  COMP2_N0_PIN
#endif
#define RTE_COMP2_N0_MUX 0

#ifndef COMP2_P1_LOC
#define RTE_COMP2_P1_PORT 0
#define RTE_COMP2_P1_PIN  27
#else
#define RTE_COMP2_P1_PORT COMP2_P1_PORT
#define RTE_COMP2_P1_PIN  COMP2_P1_PIN
#endif
#define RTE_COMP2_P1_MUX 0
#define RTE_COMP2_P1_PAD 0

#ifndef COMP2_N1_LOC
#define RTE_COMP2_N1_PORT 0
#define RTE_COMP2_N1_PIN  28
#else
#define RTE_COMP2_N1_PORT COMP2_N1_PORT
#define RTE_COMP2_N1_PIN  COMP2_N1_PIN
#endif
#define RTE_COMP2_N1_MUX 0

//COMPARATOR END

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
