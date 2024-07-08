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

#include <WhmInstance.h>
#include <WhmManufacturer.h>

#include <app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h>
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip;
using namespace chip::app::Clusters::WaterHeaterManagement;

using Protocols::InteractionModel::Status;

CHIP_ERROR WhmManufacturer::Init()
{
    WaterHeaterManagementDelegate * dg = GetWhmManufacturer()->GetWhmDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "WhmDelegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

    mBoostActive = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmManufacturer::Shutdown()
{
    return CHIP_NO_ERROR;
}

BitMask<WaterHeaterDemandBitmap> WhmManufacturer::DetermineHeatingSources()
{
    WaterHeaterManagementDelegate * dg = GetWhmManufacturer()->GetWhmDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "WhmDelegate is not initialized");
        return BitMask<WaterHeaterDemandBitmap>(0);
    }

    // A list of valid heaterTypes
    uint8_t waterHeaterTypeValues[] = {
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kImmersionElement1),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kImmersionElement2),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kHeatPump),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kBoiler),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kOther),
    };

    // The corresponding list of valid headerDemands
    uint8_t waterHeaterDemandValues[] = {
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kImmersionElement1),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kImmersionElement2),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kHeatPump),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kBoiler),
        static_cast<uint8_t>(WaterHeaterTypeBitmap::kOther),
    };

    // Iterate across the valid waterHeaterTypes seeing which heating sources are available based on heaterTypes.
    // Set the corresponding bit in the heaterDemand bitmap.
    BitMask<WaterHeaterTypeBitmap> heaterTypes = dg->GetHeaterTypes();

    uint8_t heaterDemandMask = 0;
    for (uint8_t idx = 0; idx < sizeof(waterHeaterTypeValues) / sizeof(waterHeaterTypeValues[0]); idx++)
    {
        // Is this heating source being used?
        if (heaterTypes.Raw() & waterHeaterTypeValues[idx])
        {
            heaterDemandMask |= waterHeaterDemandValues[idx];
        }
    }

    return BitMask<WaterHeaterDemandBitmap>(heaterDemandMask);
}

Status WhmManufacturer::TurnHeatingOn()
{
    Status status = Status::Success;

    ChipLogProgress(AppServer, "WhmManufacturer::TurnHeatingOn");

    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    if (dg->GetBoostState() == BoostStateEnum::kActive)
    {
        mBoostActive = true;
    }

    return status;
}

Status WhmManufacturer::TurnHeatingOff()
{
    Status status = Status::Success;

    ChipLogProgress(AppServer, "WhmManufacturer::TurnHeatingOff");

    if (mBoostActive)
    {
        mBoostActive = false;
    }

    return status;
}

Status WhmManufacturer::BoostCommandStarted(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                            Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage,
                                            Optional<chip::Percent> targetReheat)
{
    return Status::Success;
}

Status WhmManufacturer::BoostCommandCancelled()
{
    return Status::Success;
}

void WhmManufacturer::BoostCommandFinished() {}

WaterHeaterManagementDelegate * GetWhmDelegate()
{
    WhmManufacturer * mn = GetWhmManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "WhmManufacturer is null");

    WaterHeaterManagementDelegate * wg = mn->GetWhmDelegate();
    VerifyOrDieWithMsg(wg != nullptr, AppServer, "WhmDelegate is null");

    return wg;
}

void SetTestEventTrigger_BasicInstallationTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate installation in a 100L tank full of water at 20C, with a target temperature of 60C, in OFF mode
    dg->SetTankVolume(100);
    dg->SetTargetWaterTemperature(6000);
    dg->SetHeaterTypes(BitMask<WaterHeaterTypeBitmap>(WaterHeaterTypeBitmap::kImmersionElement1));
    dg->DrawOffHotWater(100, 2000);
}

void SetTestEventTrigger_BasicInstallationTestEventClear() {}

void SetTestEventTrigger_WaterTemperature20CTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate 100% of the water in the tank being at 20C
    dg->SetWaterTemperature(2000);
}

void SetTestEventTrigger_WaterTemperature61CTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate 100% of the water in the tank being at 61C
    dg->SetWaterTemperature(6100);
}

void SetTestEventTrigger_WaterTemperature66CTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate 100% of the water in the tank being at 66C
    dg->SetWaterTemperature(6600);
}

void SetTestEventTrigger_ManualModeTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate the Water Heater Mode being set to MANUAL
    dg->SetWaterHeaterMode(WaterHeaterManagementDelegate::ModeManual);
}

void SetTestEventTrigger_OffModeTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate the Water Heater Mode being set to OFF
    dg->SetWaterHeaterMode(WaterHeaterManagementDelegate::ModeOff);
}

void SetTestEventTrigger_DrawOffHotWaterTestEvent()
{
    WaterHeaterManagementDelegate * dg = GetWhmDelegate();

    // Simulate drawing off 25% of the tank volume of hot water, replaced with water at 20C
    dg->DrawOffHotWater(25, 2000);
}

bool HandleWaterHeaterManagementTestEventTrigger(uint64_t eventTrigger)
{
    WaterHeaterManagementTrigger trigger = static_cast<WaterHeaterManagementTrigger>(eventTrigger);

    switch (trigger)
    {
    case WaterHeaterManagementTrigger::kBasicInstallationTestEvent:
        ChipLogProgress(Support,
                        "[Whm::kBasicInstallationTestEvent] => Simulate installation in a 100L tank full of water at 20C, with a "
                        "target temperature of 60C, in OFF mode");
        SetTestEventTrigger_BasicInstallationTestEvent();
        break;
    case WaterHeaterManagementTrigger::kBasicInstallationTestEventClear:
        ChipLogProgress(Support, "[Whm::kBasicInstallationTestEventClear] => End simulation of installation");
        SetTestEventTrigger_BasicInstallationTestEventClear();
        break;
    case WaterHeaterManagementTrigger::kWaterTemperature20CTestEvent:
        ChipLogProgress(Support, "[Whm::kWaterTemperature20CTestEvent] => Simulate 100%% of the water in the tank being at 20C");
        SetTestEventTrigger_WaterTemperature20CTestEvent();
        break;
    case WaterHeaterManagementTrigger::kWaterTemperature61CTestEvent:
        ChipLogProgress(Support, "[Whm::kWaterTemperature61CTestEvent] => Simulate 100%% of the water in the tank being at 61C");
        SetTestEventTrigger_WaterTemperature61CTestEvent();
        break;
    case WaterHeaterManagementTrigger::kWaterTemperature66CTestEvent:
        ChipLogProgress(Support, "[Whm::kWaterTemperature66CTestEvent] => Simulate 100%% of the water in the tank being at 66C");
        SetTestEventTrigger_WaterTemperature66CTestEvent();
        break;
    case WaterHeaterManagementTrigger::kManualModeTestEvent:
        ChipLogProgress(Support, "[Whm::kManualModeTestEvent] => Simulate the Water Heater Mode being set to MANUAL");
        SetTestEventTrigger_ManualModeTestEvent();
        break;
    case WaterHeaterManagementTrigger::kOffModeTestEvent:
        ChipLogProgress(Support, "[Whm::kOffModeTestEvent] => Simulate the Water Heater Mode being set to OFF");
        SetTestEventTrigger_OffModeTestEvent();
        break;
    case WaterHeaterManagementTrigger::kDrawOffHotWaterTestEvent:
        ChipLogProgress(Support,
                        "[Whm::kDrawOffHotWaterTestEvent] => Simulate drawing off 25%% of the tank volume of hot water, replaced "
                        "with water at 20C");
        SetTestEventTrigger_DrawOffHotWaterTestEvent();
        break;
    default:
        return false;
    }

    return true;
}
