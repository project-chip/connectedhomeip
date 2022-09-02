/***************************************************************************//**
 * @file
 * @brief IOSTREAM_USART Config.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_IOSTREAM_USART_VCOM_CONFIG_H
#define SL_IOSTREAM_USART_VCOM_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>USART settings

// <o SL_IOSTREAM_USART_VCOM_BAUDRATE> Baud rate
// <i> Default: 115200
#define SL_IOSTREAM_USART_VCOM_BAUDRATE              115200

// <o SL_IOSTREAM_USART_VCOM_PARITY> Parity mode to use
// <usartNoParity=> No Parity
// <usartEvenParity=> Even parity
// <usartOddParity=> Odd parity
// <i> Default: usartNoParity
#define SL_IOSTREAM_USART_VCOM_PARITY                usartNoParity

// <o SL_IOSTREAM_USART_VCOM_STOP_BITS> Number of stop bits to use.
// <usartStopbits0p5=> 0.5 stop bits
// <usartStopbits1=> 1 stop bits
// <usartStopbits1p5=> 1.5 stop bits
// <usartStopbits2=> 2 stop bits
// <i> Default: usartStopbits1
#define SL_IOSTREAM_USART_VCOM_STOP_BITS             usartStopbits1

// <o SL_IOSTREAM_USART_VCOM_FLOW_CONTROL_TYPE> Flow control
// <usartHwFlowControlNone=> None
// <usartHwFlowControlCts=> CTS
// <usartHwFlowControlRts=> RTS
// <usartHwFlowControlCtsAndRts=> CTS/RTS
// <uartFlowControlSoftware=> Software Flow control (XON/XOFF)
// <i> Default: usartHwFlowControlNone
#define SL_IOSTREAM_USART_VCOM_FLOW_CONTROL_TYPE     usartHwFlowControlCtsAndRts

// <o SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE> Receive buffer size
// <i> Default: 32
#define SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE    32

// <q SL_IOSTREAM_USART_VCOM_CONVERT_BY_DEFAULT_LF_TO_CRLF> Convert \n to \r\n
// <i> It can be changed at runtime using the C API.
// <i> Default: 0
#define SL_IOSTREAM_USART_VCOM_CONVERT_BY_DEFAULT_LF_TO_CRLF     0

// <q SL_IOSTREAM_USART_VCOM_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION> Restrict the energy mode to allow the reception.
// <i> Default: 1
// <i> Limits the lowest energy mode the system can sleep to in order to keep the reception on. May cause higher power consumption.
#define SL_IOSTREAM_USART_VCOM_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION    1

// </h>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <usart signal=TX,RX,(CTS),(RTS)> SL_IOSTREAM_USART_VCOM
// $[USART_SL_IOSTREAM_USART_VCOM]
#define SL_IOSTREAM_USART_VCOM_PERIPHERAL        USART0
#define SL_IOSTREAM_USART_VCOM_PERIPHERAL_NO     0

// USART0 TX on PA08
#define SL_IOSTREAM_USART_VCOM_TX_PORT           gpioPortA
#define SL_IOSTREAM_USART_VCOM_TX_PIN            8

// USART0 RX on PA09
#define SL_IOSTREAM_USART_VCOM_RX_PORT           gpioPortA
#define SL_IOSTREAM_USART_VCOM_RX_PIN            9

// USART0 CTS on PB05
#define SL_IOSTREAM_USART_VCOM_CTS_PORT          gpioPortB
#define SL_IOSTREAM_USART_VCOM_CTS_PIN           5

// USART0 RTS on PA00
#define SL_IOSTREAM_USART_VCOM_RTS_PORT          gpioPortA
#define SL_IOSTREAM_USART_VCOM_RTS_PIN           0

// [USART_SL_IOSTREAM_USART_VCOM]$
// <<< sl:end pin_tool >>>

#endif
