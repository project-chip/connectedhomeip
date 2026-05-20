/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/** @file "App_Battery.h"
 *
 *  Application API
 *
 *  Declarations of the public functions and enumerations of App battery.
 */

#ifndef _APP_BATTERY_H_
#define _APP_BATTERY_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

#include <functional>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <app/data-model/DecodableList.h>
#include <app/util/im-client-callbacks.h>

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @brief Macro Define for period of measurement update */
#define APP_BATTERY_UPDATE_PERIOD MS_TO_US(300000) // 5 mins

/** @brief Macro Define for an example battery voltage range */
#define APP_BATTERY_MEDIUM 66 //  Battery medium(66%)
#define APP_BATTERY_LOW 33    //  Battery low(33%)

/** @brief Macro Define for an example thresholds of battery voltage */
#define APP_BATTERY_FULL_THRESHOLD 3200     // Battery full(100%)   in 1 mV - 3.2V
#define APP_BATTERY_MEDIUM_THRESHOLD 2500   // Battery medium(66%)  in 1 mV - 2.5V
#define APP_BATTERY_LOW_THRESHOLD 2300      // Battery low(33%)     in 1 mV - 2.3V
#define APP_BATTERY_CRITICAL_THRESHOLD 2100 // Battery critical(0%) in 1 mV - 2.1V

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef UInt8 DoubledPercentage;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** @brief callback when device has updated its battery voltage and remaining percentage
 */
void Application_cbBatteryVoltageUpdated(UInt8 status);

/** @brief Function to retrieve Battery voltage value from system
 *
 *   @return voltage                   low voltage threshold in 100mV/unit.
 */
UInt32 Application_GetLowBatteryVoltageThreshold(void);

/** @brief Function to retrieve Battery voltage value from system
 *
 *   @paramp pBatteryVoltage           measured battery voltage in mV unit
 *   @return success                   True if valid temperature value is returned.
 */
Bool Application_GetBatteryVoltage(UInt32 * pBatteryVoltage);

/** @brief example function to calculate and update the battery remaining percentage
 *
 *   @param   batteryVoltage             Current battery voltage in mV
 *   @return  remainingPercentage        The battery remaining percentage in a 0.5% unit
 */
UInt8 Application_UpdateBatteryRemaining(UInt32 voltage);

/** @brief Function to update Power Configuration cluster attributes
 */
void Application_UpdateBattery(void);

/** @brief Function to start/restart periodic update battery voltage reading
 */
void Application_StartPeriodicBatteryUpdate(void);

/** @brief Function to stop periodic update battery voltage reading
 */
void Application_StopPeriodicBatteryUpdate(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_APP_H_
