/***************************************************************************//**
 * @file
 * @brief Legacy Host ASH configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef LEGACY_NCP_ASH_CONFIG_H
#define LEGACY_NCP_ASH_CONFIG_H

// <o LEGACY_NCP_ASH_SERIAL_PORT> The USART used for ASH communications in COM_Port_t format, as defined in platform/service/legacy_hal/inc/serial.h (see defined names for USART ports) <-1..3:1>
// <i> Default: (-1)
// <i> The default value of -1 causes the ASH code to try to use the USART assigned to the "VCOM" instance of SL_IOSTREAM_USART. Otherwise the value is passed to Legacy HAL, which attempts to find an instance of SL_IOSTREAM_USART which uses that USART.
#define LEGACY_NCP_ASH_SERIAL_PORT    (-1)

#endif /* LEGACY_NCP_ASH_CONFIG_H */

// <<< end of configuration section >>>
