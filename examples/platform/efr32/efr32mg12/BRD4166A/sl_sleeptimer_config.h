/***************************************************************************//**
 * @file
 * @brief Sleep Timer configuration file.
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_SLEEPTIMER_CONFIG_H
#define SL_SLEEPTIMER_CONFIG_H

#define SL_SLEEPTIMER_PERIPHERAL_DEFAULT 0
#define SL_SLEEPTIMER_PERIPHERAL_RTCC    1
#define SL_SLEEPTIMER_PERIPHERAL_PRORTC  2
#define SL_SLEEPTIMER_PERIPHERAL_RTC     3
#define SL_SLEEPTIMER_PERIPHERAL_SYSRTC  4
#define SL_SLEEPTIMER_PERIPHERAL_BURTC   5

// <o SL_SLEEPTIMER_PERIPHERAL> Timer Peripheral Used by Sleeptimer
//   <SL_SLEEPTIMER_PERIPHERAL_DEFAULT=> Default (auto select)
//   <SL_SLEEPTIMER_PERIPHERAL_RTCC=> RTCC
//   <SL_SLEEPTIMER_PERIPHERAL_PRORTC=> Radio internal RTC (PRORTC)
//   <SL_SLEEPTIMER_PERIPHERAL_RTC=> RTC
//   <SL_SLEEPTIMER_PERIPHERAL_SYSRTC=> SYSRTC
//   <SL_SLEEPTIMER_PERIPHERAL_BURTC=> Back-Up RTC (BURTC)
// <i> Selection of the Timer Peripheral Used by the Sleeptimer
#define SL_SLEEPTIMER_PERIPHERAL  SL_SLEEPTIMER_PERIPHERAL_DEFAULT

// <q SL_SLEEPTIMER_WALLCLOCK_CONFIG> Enable wallclock functionality
// <i> Enable or disable wallclock functionalities (get_time, get_date, etc).
// <i> Default: 0
#define SL_SLEEPTIMER_WALLCLOCK_CONFIG  0

// <o SL_SLEEPTIMER_FREQ_DIVIDER> Timer frequency divider
// <i> Default: 1
#define SL_SLEEPTIMER_FREQ_DIVIDER  1

// <q SL_SLEEPTIMER_PRORTC_HAL_OWNS_IRQ_HANDLER> If Radio internal RTC (PRORTC) HAL is used, determines if it owns the IRQ handler. Enable, if no wireless stack is used.
// <i> Default: 0
#define SL_SLEEPTIMER_PRORTC_HAL_OWNS_IRQ_HANDLER  0

// <q SL_SLEEPTIMER_DEBUGRUN> Enable DEBUGRUN functionality on hardware RTC.
// <i> Default: 0
#define SL_SLEEPTIMER_DEBUGRUN  0

#endif /* SLEEPTIMER_CONFIG_H */

// <<< end of configuration section >>>
