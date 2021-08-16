/***************************************************************************//**
 * @file
 * @brief Packet Trace Information configuration file.
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

#ifndef SL_RAIL_UTIL_PTI_CONFIG_H
#define SL_RAIL_UTIL_PTI_CONFIG_H

#include "rail_types.h"

// <<< Use Configuration Wizard in Context Menu >>>
// <h> PTI Configuration

// <o SL_RAIL_UTIL_PTI_MODE> PTI mode
// <RAIL_PTI_MODE_UART=> UART
// <RAIL_PTI_MODE_UART_ONEWIRE=> UART onewire
// <RAIL_PTI_MODE_SPI=> SPI
// <RAIL_PTI_MODE_DISABLED=> Disabled
// <i> Default: RAIL_PTI_MODE_UART
#define SL_RAIL_UTIL_PTI_MODE           RAIL_PTI_MODE_UART

// <o SL_RAIL_UTIL_PTI_BAUD_RATE_HZ> PTI Baud Rate (Hertz)
// <147800-20000000:1>
// <i> Default: 1600000
#define SL_RAIL_UTIL_PTI_BAUD_RATE_HZ   1600000

// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <pti signal=DOUT,(DFRAME),(DCLK)> SL_RAIL_UTIL_PTI
// $[PTI_SL_RAIL_UTIL_PTI]
#define SL_RAIL_UTIL_PTI_PERIPHERAL              PTI

// PTI DOUT on PB12
#define SL_RAIL_UTIL_PTI_DOUT_PORT               gpioPortB
#define SL_RAIL_UTIL_PTI_DOUT_PIN                12
#define SL_RAIL_UTIL_PTI_DOUT_LOC                6

// PTI DFRAME on PB13
#define SL_RAIL_UTIL_PTI_DFRAME_PORT             gpioPortB
#define SL_RAIL_UTIL_PTI_DFRAME_PIN              13
#define SL_RAIL_UTIL_PTI_DFRAME_LOC              6


// [PTI_SL_RAIL_UTIL_PTI]$

// <<< sl:end pin_tool >>>

#endif // SL_RAIL_UTIL_PTI_CONFIG_H
