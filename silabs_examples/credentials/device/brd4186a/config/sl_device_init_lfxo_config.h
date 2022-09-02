/***************************************************************************//**
 * @file
 * @brief DEVICE_INIT_LFXO Config
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

#ifndef SL_DEVICE_INIT_LFXO_CONFIG_H
#define SL_DEVICE_INIT_LFXO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_DEVICE_INIT_LFXO_MODE> Mode
// <i>
// <cmuLfxoOscMode_Crystal=> Crystal oscillator
// <cmuLfxoOscMode_AcCoupledSine=> AC-coupled buffer
// <cmuLfxoOscMode_External=> External digital clock
// <i> Default: cmuLfxoOscMode_Crystal
#define SL_DEVICE_INIT_LFXO_MODE           cmuLfxoOscMode_Crystal

// <o SL_DEVICE_INIT_LFXO_CTUNE> CTUNE <0-127>
// <i> Default: 63
#define SL_DEVICE_INIT_LFXO_CTUNE          36

// <o SL_DEVICE_INIT_LFXO_PRECISION> LFXO precision in PPM <0-65535>
// <i> Default: 500
#define SL_DEVICE_INIT_LFXO_PRECISION      100

// <o SL_DEVICE_INIT_LFXO_TIMEOUT> Startup Timeout Delay
// <i>
// <cmuLfxoStartupDelay_2Cycles=> 2 cycles
// <cmuLfxoStartupDelay_256Cycles=> 256 cycles
// <cmuLfxoStartupDelay_1KCycles=> 1K cycles
// <cmuLfxoStartupDelay_2KCycles=> 2K cycles
// <cmuLfxoStartupDelay_4KCycles=> 4K cycles
// <cmuLfxoStartupDelay_8KCycles=> 8K cycles
// <cmuLfxoStartupDelay_16KCycles=> 16K cycles
// <cmuLfxoStartupDelay_32KCycles=> 32K cycles
// <i> Default: cmuLfxoStartupDelay_4KCycles
#define SL_DEVICE_INIT_LFXO_TIMEOUT           cmuLfxoStartupDelay_4KCycles
// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_LFXO_CONFIG_H
