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

/** @file "mainBattery.cpp"
 *
 * Battery functionality for application with battery as source.
 * The Reading will be set in the power configuration cluster
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpBatteryMonitor.h"
#include "gpSched.h"

#include <lib/support/CHIPMem.h>

#include "App_Battery.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters;

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP
#define QPG_BATTERY_ENDPOINT_ID (0)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Extern Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Application function prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Default class overrides
 *****************************************************************************/

/*****************************************************************************
 *                    Stack configuration
 *****************************************************************************/

/*****************************************************************************
 *                    Application function definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional callbacks to application
 *****************************************************************************/

void SetBatteryVoltage(uint32_t voltage)
{
    SystemLayer().ScheduleLambda([voltage] {
        Protocols::InteractionModel::Status status =
            PowerSource::Attributes::BatVoltage::Set(QPG_BATTERY_ENDPOINT_ID, (uint32_t) voltage);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating voltage %x", to_underlying(status));
        }
    });
}

void SetBatteryPercentageRemaining(DoubledPercentage value)
{
    SystemLayer().ScheduleLambda([value] {
        Protocols::InteractionModel::Status status =
            PowerSource::Attributes::BatPercentRemaining::Set(QPG_BATTERY_ENDPOINT_ID, value);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating percentage %x", to_underlying(status));
        }
    });
}

void SetBatChargeLevel(BatChargeLevelEnum level)
{
    SystemLayer().ScheduleLambda([level] {
        Protocols::InteractionModel::Status status = PowerSource::Attributes::BatChargeLevel::Set(QPG_BATTERY_ENDPOINT_ID, level);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating BatChargeLevel %x", to_underlying(status));
        }
    });
}

/** @brief Application specific implementation of the Battery charge level
          Below implementation is an reference example for battery powered devices
*/
BatChargeLevelEnum CalculateBatChargeLevel(uint32_t currentBatVoltage, DoubledPercentage doubledPercentage)
{
    BatChargeLevelEnum value;

    // Assign proper values to the power level dependant on battery'
    if (currentBatVoltage <= APP_BATTERY_CRITICAL_THRESHOLD)
    {
        // battery voltage < critical battery level
        value = BatChargeLevelEnum::kCritical;
    }
    else if (doubledPercentage > (2 * APP_BATTERY_LOW))
    {
        // battery remaining > 33% (2*33)
        value = BatChargeLevelEnum::kOk;
    }
    else
    {
        // battery remaining < 33%
        value = BatChargeLevelEnum::kWarning;
    }

    return value;
}

/*****************************************************************************
 * ---  Battery monitoring
 *****************************************************************************/

/** @brief Function to retrieve Battery voltage value from system
 *
 *   @return voltage                   low voltage threshold in 1mV/unit.
 */
uint32_t Application_GetLowBatteryVoltageThreshold(void)
{
    return (APP_BATTERY_LOW_THRESHOLD);
}

/** @brief Function to retrieve Battery voltage value from system
 *
 *   @param pBatteryVoltage            measured battery voltage in mV
 *   @return success                   True if valid temperature value is returned.
 */
Bool Application_GetBatteryVoltage(uint32_t * pBatteryVoltage)
{
    UQ2_6 voltageReading;
    bool success;

    // Measure Battery Voltage
    if (pBatteryVoltage == NULL)
    {
        return false;
    }

    success = gpBatteryMonitor_Measure(gpBatteryMonitor_MeasurementTypeUnloadedWithRecovery, &voltageReading);
    *pBatteryVoltage =
        BATTERY_MONITOR_GET_INTEGER_PART(voltageReading) * 1000 + BATTERY_MONITOR_GET_FLOATING_PART(voltageReading) * 10;

    return success;
}

/** @brief example function to calculate and update the battery remaining percentage
 *
 *   @param   batteryVoltage             Current battery voltage in 1mV unit
 *   @return  doubledPercentage          The battery remaining percentage in a 0.5% unit
 */
DoubledPercentage Application_UpdateBatteryRemaining(uint32_t batteryVoltage)
{
    /**
     *   Porting note:
     *   Add/Modify the the battery remaining percentage measurement
     *   according to your specific HW/Battery here.
     *   Currently provided is an example by converting battery voltage into remaining percentage.
     */
    uint8_t remainingPercentage; // in 1% unit

    if (batteryVoltage >= APP_BATTERY_FULL_THRESHOLD)
    {
        remainingPercentage = 100;
        ChipLogProgress(NotSpecified, "Battery: FULL");
    }
    // Battery Section 1: Full ~ Medium
    else if (batteryVoltage >= APP_BATTERY_MEDIUM_THRESHOLD)
    {
        remainingPercentage = (100 -
                               (uint8_t) (((APP_BATTERY_FULL_THRESHOLD - batteryVoltage) * (100 - APP_BATTERY_MEDIUM)) /
                                          (APP_BATTERY_FULL_THRESHOLD - APP_BATTERY_MEDIUM_THRESHOLD)));

        ChipLogProgress(NotSpecified, "Battery: above MEDIUM");
    }
    // Battery Section 2: Medium ~ Low
    else if (batteryVoltage >= APP_BATTERY_LOW_THRESHOLD)
    {
        remainingPercentage =
            (APP_BATTERY_MEDIUM -
             (uint8_t) (((APP_BATTERY_MEDIUM_THRESHOLD - batteryVoltage) * (APP_BATTERY_MEDIUM - APP_BATTERY_LOW)) /
                        (APP_BATTERY_MEDIUM_THRESHOLD - APP_BATTERY_LOW_THRESHOLD)));

        ChipLogProgress(NotSpecified, "Battery: above LOW");
    }
    // Battery Section 3: Low ~ Critical
    else if (batteryVoltage >= APP_BATTERY_CRITICAL_THRESHOLD)
    {
        remainingPercentage = (APP_BATTERY_LOW -
                               (uint8_t) ((APP_BATTERY_LOW_THRESHOLD - batteryVoltage) * APP_BATTERY_LOW /
                                          (APP_BATTERY_LOW_THRESHOLD - APP_BATTERY_CRITICAL_THRESHOLD)));
        ChipLogProgress(NotSpecified, "Battery: below LOW");
    }
    else
    {
        remainingPercentage = 0;
        ChipLogProgress(NotSpecified, "Battery: CRITICAL");
    }

    ChipLogProgress(NotSpecified, "Battery: %lu.%03luV, %d(%%)", batteryVoltage / 1000, batteryVoltage % 1000, remainingPercentage);

    return remainingPercentage * 2;
}

/** @brief Function to update Power Configuration cluster attributes
 */
void Application_UpdateBattery(void)
{
    uint32_t voltage;
    Bool status = false;

    // get current battery voltage reading
    status = Application_GetBatteryVoltage(&voltage);

    if (status)
    {
        // Update Power configuration's BatteryPercentageRemaining attribute
        DoubledPercentage doubledPercentage = Application_UpdateBatteryRemaining(voltage);

        // Update Power configuration's BatteryVoltage attribute
        SetBatteryVoltage(voltage);
        SetBatteryPercentageRemaining(doubledPercentage);
        BatChargeLevelEnum level = CalculateBatChargeLevel(voltage, doubledPercentage);
        SetBatChargeLevel(level);
    }
    else
    {
        ChipLogError(NotSpecified, "error reading battery voltage");
    }
}

/** @brief Function to start/restart periodic update battery voltage reading
 */
void Application_StartPeriodicBatteryUpdate(void)
{
    // Update Battery reading
    Application_UpdateBattery();

    // remove previous scheduled event first
    if (gpSched_ExistsEvent(Application_StartPeriodicBatteryUpdate))
    {
        gpSched_UnscheduleEvent(Application_StartPeriodicBatteryUpdate);
    }

    // schedule the periodic battery voltage reading update
    gpSched_ScheduleEvent(APP_BATTERY_UPDATE_PERIOD, Application_StartPeriodicBatteryUpdate);
}

/** @brief Function to stop periodic update battery voltage reading
 */
void Application_StopPeriodicBatteryUpdate(void)
{
    // Update Battery reading
    Application_UpdateBattery();

    // remove periodic battery voltage reading update schedular
    if (gpSched_ExistsEvent(Application_StartPeriodicBatteryUpdate))
    {
        gpSched_UnscheduleEvent(Application_StartPeriodicBatteryUpdate);
    }
}
