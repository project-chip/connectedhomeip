/***************************************************************************//**
 * @file
 * @brief DEVICE_INIT_HFXO Config
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

#ifndef SL_DEVICE_INIT_HFXO_CONFIG_H
#define SL_DEVICE_INIT_HFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_HFXO_MODE> Mode
// <i>
// <cmuOscMode_Crystal=> Crystal oscillator
// <cmuOscMode_External=> External digital clock
// <i> Default: cmuOscMode_Crystal
#define SL_DEVICE_INIT_HFXO_MODE           cmuOscMode_Crystal

// <o SL_DEVICE_INIT_HFXO_FREQ> Frequency <38000000-40000000>
// <i> Default: 38400000
#define SL_DEVICE_INIT_HFXO_FREQ           38400000

// <o SL_DEVICE_INIT_HFXO_CTUNE> CTUNE <0-511>
// <i> Default: 360
#define SL_DEVICE_INIT_HFXO_CTUNE          331

// <h> Advanced Configurations
// <o SL_DEVICE_INIT_HFXO_AUTOSTART> Auto-start HFXO. This feature is incompatible with Power Manager and can only be enabled in applications that do not use Power Manager or a radio protocol stack. - DEPRECATED
// <true=> True
// <false=> False
// <i> Default: false
#define SL_DEVICE_INIT_HFXO_AUTOSTART      false

// <o SL_DEVICE_INIT_HFXO_AUTOSELECT> Auto-select HFXO. This feature is incompatible with Power Manager and can only be enabled in applications that do not use Power Manager or a radio protocol stack. - DEPRECATED
// <true=> True
// <false=> False
// <i> Default: false
#define SL_DEVICE_INIT_HFXO_AUTOSELECT      false

// </h>

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_HFXO_CONFIG_H
