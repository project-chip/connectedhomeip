/***************************************************************************//**
 * @file
 * @brief Board Control
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BOARD_CONTROL_CONFIG_H
#define SL_BOARD_CONTROL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_BOARD_ENABLE_VCOM> Enable Virtual COM UART
// <i> Default: 0
#define SL_BOARD_ENABLE_VCOM                    1

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_BOARD_ENABLE_VCOM
// $[GPIO_SL_BOARD_ENABLE_VCOM]
#define SL_BOARD_ENABLE_VCOM_PORT               gpioPortC
#define SL_BOARD_ENABLE_VCOM_PIN                1
// [GPIO_SL_BOARD_ENABLE_VCOM]$

// <<< sl:end pin_tool >>>

#endif // SL_BOARD_CONTROL_CONFIG_H
