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
// <cmuHfxoOscMode_Crystal=> Crystal oscillator
// <cmuHfxoOscMode_ExternalSine=> External sine wave
// <i> Default: cmuHfxoOscMode_Crystal
#define SL_DEVICE_INIT_HFXO_MODE           cmuHfxoOscMode_Crystal

// <o SL_DEVICE_INIT_HFXO_FREQ> Frequency <38000000-40000000>
// <i> Default: 39000000
#define SL_DEVICE_INIT_HFXO_FREQ           39000000

// <o SL_DEVICE_INIT_HFXO_CTUNE> CTUNE <0-255>
// <i> Default: 140
#define SL_DEVICE_INIT_HFXO_CTUNE          140

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_HFXO_CONFIG_H
