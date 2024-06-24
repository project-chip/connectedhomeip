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

#include <WhmManufacturer.h>
#include <WhmInstance.h>

#include <app/clusters/water-heater-management-server/water-heater-management-server.h>
#include <app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h>

#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip;
using namespace chip::app::Clusters::WaterHeaterManagement;

CHIP_ERROR WhmManufacturer::Init()
{
    WaterHeaterManagementDelegate * dg = GetWhmManufacturer()->GetWhmDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "WhmDelegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmManufacturer::Shutdown()
{
    return CHIP_NO_ERROR;
}

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

void SetTestEventTrigger_BasicInstallationTestEventClear()
{
}

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
        ChipLogProgress(Support, "[Whm::kBasicInstallationTestEvent] => Simulate installation in a 100L tank full of water at 20C, with a target temperature of 60C, in OFF mode");
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
        ChipLogProgress(Support, "[Whm::kDrawOffHotWaterTestEvent] => Simulate drawing off 25%% of the tank volume of hot water, replaced with water at 20C");
        SetTestEventTrigger_DrawOffHotWaterTestEvent();
        break;
    default:
        return false;
    }

    return true;
}
