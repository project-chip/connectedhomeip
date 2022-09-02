/***************************************************************************//**
 * @file
 * @brief Power Amplifier configuration file.
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

#ifndef SL_RAIL_UTIL_PA_CONFIG_H
#define SL_RAIL_UTIL_PA_CONFIG_H

#include "rail_types.h"

// <<< Use Configuration Wizard in Context Menu >>>
// <h>PA configuration

// <o SL_RAIL_UTIL_PA_POWER_DECI_DBM> Initial PA Power (deci-dBm, 100 = 10.0 dBm)
// <i> Default: 100
#define SL_RAIL_UTIL_PA_POWER_DECI_DBM      100

// <o SL_RAIL_UTIL_PA_RAMP_TIME_US> PA Ramp Time (microseconds)
// <0-65535:1>
// <i> Default: 10
#define SL_RAIL_UTIL_PA_RAMP_TIME_US        10

// <o SL_RAIL_UTIL_PA_VOLTAGE_MV> Milli-volts on PA supply pin (PA_VDD)
// <0-65535:1>
// <i> Default: 3300
#define SL_RAIL_UTIL_PA_VOLTAGE_MV          3300

// <o SL_RAIL_UTIL_PA_SELECTION_2P4GHZ> 2.4 GHz PA Selection
// <RAIL_TX_POWER_MODE_2P4GIG_HIGHEST=> Highest Possible
// <RAIL_TX_POWER_MODE_2P4GIG_HP=> High Power (chip-specific)
// <RAIL_TX_POWER_MODE_2P4GIG_LP=> Low Power
// <RAIL_TX_POWER_MODE_NONE=> Disable
// <i> Default: RAIL_TX_POWER_MODE_2P4GIG_HIGHEST
#define SL_RAIL_UTIL_PA_SELECTION_2P4GHZ    RAIL_TX_POWER_MODE_2P4GIG_HIGHEST

// <o SL_RAIL_UTIL_PA_SELECTION_SUBGHZ> Sub-1 GHz PA Selection
// <RAIL_TX_POWER_MODE_NONE=> Disable
// <i> Default: RAIL_TX_POWER_MODE_NONE
#define SL_RAIL_UTIL_PA_SELECTION_SUBGHZ    RAIL_TX_POWER_MODE_NONE

// <s.50 SL_RAIL_UTIL_PA_CURVE_HEADER> Header file containing custom PA curves
// <i> Default: "pa_curves_efr32.h"
#define SL_RAIL_UTIL_PA_CURVE_HEADER        "pa_curves_efr32.h"

// <s.50 SL_RAIL_UTIL_PA_CURVE_TYPES> Header file containing PA curve types
// <i> Default: "pa_curve_types_efr32.h"
#define SL_RAIL_UTIL_PA_CURVE_TYPES         "pa_curve_types_efr32.h"

// <q SL_RAIL_UTIL_PA_CALIBRATION_ENABLE> Enable PA Calibration
// <i> Default: 0
#define SL_RAIL_UTIL_PA_CALIBRATION_ENABLE  0

// </h>
// <<< end of configuration section >>>

#endif // SL_RAIL_UTIL_PA_CONFIG_H
