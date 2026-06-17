/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    Copyright 2026 NXP
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

#include "BatteryApplicationManager.h"
#include "FreeRTOS.h"
#include "timers.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

extern "C" {
#include "sensors.h"
}

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters;

namespace chip::NXP::App {

BatteryApplicationManager BatteryApplicationManager::sInstance;

// Battery monitoring interval: 5 minutes (in milliseconds)
#define BATTERY_UPDATE_INTERVAL_MS 300000

// Battery voltage thresholds (in mV)
#define APP_BATTERY_FULL_THRESHOLD 3000     // 3.0 V = 100%
#define APP_BATTERY_MEDIUM_THRESHOLD 2600   // 2.6 V = 66%
#define APP_BATTERY_LOW_THRESHOLD 2200      // 2.2 V = 33%
#define APP_BATTERY_CRITICAL_THRESHOLD 1800 // 1.8 V = 0%

// Battery percentage levels
#define APP_BATTERY_MEDIUM 66
#define APP_BATTERY_LOW 33

// Endpoint ID where Power Source cluster is implemented
#define BATTERY_ENDPOINT_ID 0

void BatteryApplicationManager::Init()
{
    ChipLogProgress(DeviceLayer, "BatteryApplicationManager::Init()");

    // Initialize the Sensors module (if not already initialized)
    SENSORS_Init();

    // Create FreeRTOS timer for periodic battery updates (5 minutes)
    mBatteryTimer = xTimerCreate("BatteryTimer",                            // Timer name
                                 pdMS_TO_TICKS(BATTERY_UPDATE_INTERVAL_MS), // Timer period (5 minutes)
                                 pdTRUE,                                    // Auto-reload timer
                                 nullptr,                                   // Timer ID (not used)
                                 [](TimerHandle_t xTimer) {                 // Callback function
                                     BatteryAppMgr().UpdateBatteryStatus();
                                 });

    if (mBatteryTimer == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to create battery timer");
        return;
    }

    // Schedule PowerSource::Attributes updates in Matter context
    // CHIP task has not yet started here
    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([] {
        Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;

        // Update Status attribute (battery is active)
        status = PowerSource::Attributes::Status::Set(BATTERY_ENDPOINT_ID, PowerSourceStatusEnum::kActive);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(DeviceLayer, "Failed to update Status: %x", to_underlying(status));
        }

        // Update Order attribute (primary battery = 0)
        status = PowerSource::Attributes::Order::Set(BATTERY_ENDPOINT_ID, 0);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(DeviceLayer, "Failed to update Order: %x", to_underlying(status));
        }

        // Update Description attribute
        status = PowerSource::Attributes::Description::Set(BATTERY_ENDPOINT_ID, CharSpan::fromCharString("3.3V coin cell battery"));
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(DeviceLayer, "Failed to update Description: %x", to_underlying(status));
        }

        // Update BatReplaceability attribute
        status = PowerSource::Attributes::BatReplaceability::Set(BATTERY_ENDPOINT_ID, BatReplaceabilityEnum::kUserReplaceable);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(DeviceLayer, "Failed to update BatReplaceability: %x", to_underlying(status));
        }

        // Update BatPresent attribute - indicate whether the Node detects that the batteries are properly installed
        // DUT can't detect if batteries are installed, always set to true
        status = PowerSource::Attributes::BatPresent::Set(BATTERY_ENDPOINT_ID, true);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(DeviceLayer, "Failed to update BatPresent: %x", to_underlying(status));
        }
    });

    // Perform initial battery measurement
    UpdateBatteryStatus();
}

void BatteryApplicationManager::StartPeriodicUpdate()
{
    ChipLogProgress(DeviceLayer, "Starting periodic battery updates");
    TimerHandle_t timer = static_cast<TimerHandle_t>(mBatteryTimer);

    if (timer != nullptr)
    {
        if (xTimerIsTimerActive(timer))
        {
            ChipLogError(DeviceLayer, "Battery timer already started.");
            return;
        }

        // Start the timer
        if (xTimerStart(timer, 0) != pdPASS)
        {
            ChipLogError(DeviceLayer, "Failed to start battery timer");
        }
    }
}

void BatteryApplicationManager::StopPeriodicUpdate()
{
    ChipLogProgress(DeviceLayer, "Stopping periodic battery updates");
    TimerHandle_t timer = static_cast<TimerHandle_t>(mBatteryTimer);

    if (timer != nullptr)
    {
        // Stop the timer
        if (xTimerStop(timer, 0) != pdPASS)
        {
            ChipLogError(DeviceLayer, "Failed to stop battery timer");
        }
    }
}

void BatteryApplicationManager::UpdateBatteryStatus()
{
    TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            // Trigger battery measurement
            SENSORS_TriggerBatteryMeasurement();
            // Schedule refresh
            BatteryAppMgr().RefreshBatteryLevel();
        },
        0);
}

void BatteryApplicationManager::RefreshBatteryLevel()
{
    // Refresh and get battery level from Sensors module
    uint8_t batteryLevel = SENSORS_RefreshBatteryLevel();

    ChipLogProgress(DeviceLayer, "Battery level: %d%%", batteryLevel);

    // Update Power Source cluster attributes
    UpdatePowerSourceCluster(batteryLevel);
}

void BatteryApplicationManager::UpdatePowerSourceCluster(uint8_t batteryPercentage)
{
    // Convert percentage to doubled percentage (0.5% units, 0-200 range)
    uint8_t doubledPercentage = batteryPercentage * 2;
    // Battery voltage in mV
    uint32_t batteryVoltage =
        APP_BATTERY_CRITICAL_THRESHOLD + ((APP_BATTERY_FULL_THRESHOLD - APP_BATTERY_CRITICAL_THRESHOLD) * batteryPercentage / 100);

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;

    // Update BatVoltage attribute
    status = PowerSource::Attributes::BatVoltage::Set(BATTERY_ENDPOINT_ID, batteryVoltage);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to update BatVoltage: %x", to_underlying(status));
    }

    // Update BatPercentRemaining attribute
    status = PowerSource::Attributes::BatPercentRemaining::Set(BATTERY_ENDPOINT_ID, doubledPercentage);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to update BatPercentRemaining: %x", to_underlying(status));
    }

    // Calculate battery charge level based on percentage
    BatChargeLevelEnum chargeLevel;

    if (batteryPercentage > APP_BATTERY_MEDIUM)
    {
        chargeLevel = BatChargeLevelEnum::kOk;
    }
    else if (batteryPercentage > APP_BATTERY_LOW)
    {
        chargeLevel = BatChargeLevelEnum::kWarning;
    }
    else
    {
        chargeLevel = BatChargeLevelEnum::kCritical;
    }

    // Update BatTimeRemaining attribute
    // DUT can't estimate remaining time, set to null value
    status = PowerSource::Attributes::BatTimeRemaining::Set(BATTERY_ENDPOINT_ID, DataModel::NullNullable);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to update BatTimeRemaining: %x", to_underlying(status));
    }

    // Update BatChargeLevel attribute
    status = PowerSource::Attributes::BatChargeLevel::Set(BATTERY_ENDPOINT_ID, chargeLevel);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to update BatChargeLevel: %x", to_underlying(status));
    }

    // Update BatReplacementNeeded attribute
    bool replacementNeeded = (chargeLevel == BatChargeLevelEnum::kCritical);

    status = PowerSource::Attributes::BatReplacementNeeded::Set(BATTERY_ENDPOINT_ID, replacementNeeded);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to update BatReplacementNeeded: %x", to_underlying(status));
    }
}

} // namespace chip::NXP::App
