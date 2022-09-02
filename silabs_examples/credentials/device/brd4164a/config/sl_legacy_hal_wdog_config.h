/***************************************************************************//**
 * @file sl_legacy_hal_wdog_config.h
 * @brief Legacy HAL watchdog configuration file.
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_LEGACY_HAL_WDOG_CONFIG_H
#define SL_LEGACY_HAL_WDOG_CONFIG_H

// <h> Legacy HAL WDOG Configurations

// <e SL_LEGACY_HAL_DISABLE_WATCHDOG> Disable calling halInternalEnableWatchDog in base-replacement.c's halInit().
// <i> Default: 0
#define SL_LEGACY_HAL_DISABLE_WATCHDOG 0
// </e>

// <o SL_LEGACY_HAL_WDOGn> WDOG to use for SL_LEGACY_HAL_WDOGn.
// <i> Default: 0
// <0=> WDOG0
// <1=> WDOD1
#define SL_LEGACY_HAL_WDOGn 0

// </h>

#endif /* SL_LEGACY_HAL_WDOG_CONFIG_H */

// <<< end of configuration section >>>
