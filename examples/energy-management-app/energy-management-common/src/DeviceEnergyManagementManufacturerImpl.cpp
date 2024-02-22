/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <DeviceEnergyManagementManufacturerImpl.h>
// #include <EnergyEvseManager.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
// #include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
// #include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
// #include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>

using chip::Protocols::InteractionModel::Status;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

static DataModel::Nullable<Structs::ForecastStruct::Type> sForecast;

struct DeviceEnergyManagementTestEventSaveData
{
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast;
};

static DeviceEnergyManagementTestEventSaveData    sDeviceEnergyManagementTestEventSaveData;


CHIP_ERROR DeviceEnergyManagementManufacturer::Init()
{
    /* Manufacturers should modify this to do any custom initialisation */

    /* Register callbacks */
    DeviceEnergyManagementDelegate * dg = GetDeviceEnergyManagementManufacturer()->GetDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "EVSE Delegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

// TODO:    dg->HwRegisterEvseCallbackHandler(ApplicationCallbackHandler, reinterpret_cast<intptr_t>(this));

    /*
     * This is an example implementation for manufacturers to consider
     */

    /* Once the system is initialised then check to see if the state was restored
     * (e.g. after a power outage), and if the Enable timer check needs to be started
     */

    return CHIP_NO_ERROR;
}

/*
 */
CHIP_ERROR DeviceEnergyManagementManufacturer::Shutdown()
{
    return CHIP_NO_ERROR;
}

DeviceEnergyManagementDelegate * GetDeviceEnergyManagementDelegate()
{
    DeviceEnergyManagementManufacturer * mn = GetDeviceEnergyManagementManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "DeviceEnergyManagementManufacturer is null");

    DeviceEnergyManagementDelegate * dg = mn->GetDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "DeviceEnergyManagement Delegate is null");

    return dg;
}

void SetTestEventTrigger_zzzzzzzzzz()
{
    ChipLogProgress(Support, "[zzzzzzzzzz-handle] L-%d", __LINE__ );
}

void SetTestEventTrigger_StartTimeAdjustment()
{
    DeviceEnergyManagementDelegate * dg = GetDeviceEnergyManagementDelegate();
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d", __LINE__ );

    sDeviceEnergyManagementTestEventSaveData.forecast = dg->GetForecast();
    //     virtual DataModel::Nullable<Structs::ForecastStruct::Type> GetForecast() override;
    sForecast = dg->GetForecast();

    //  Status  StartTimeAdjustRequest(const uint32_t requestedStartTime, AdjustmentCauseEnum cause) override;
    Status s = dg->StartTimeAdjustRequest(1, AdjustmentCauseEnum::kLocalOptimization);
}

bool HandleDeviceEnergyManagementTestEventTrigger(uint64_t eventTrigger)
{
    DeviceEnergyManagementTrigger trigger = static_cast<DeviceEnergyManagementTrigger>(eventTrigger);

 ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => zzzzzzzzz-%d", __LINE__);
 ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => zzzzzzzzz-%d - trigger=0x%lux", __LINE__, (uint64_t)trigger);
 
    switch (trigger)
    {
    case DeviceEnergyManagementTrigger::kPowerAdjustment:
        ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => Create PowerAdjustment struct");
        SetTestEventTrigger_zzzzzzzzzz();
        break;
    case DeviceEnergyManagementTrigger::kPowerAdjustmentClear:
        ChipLogProgress(Support, "[PowerAdjustmentClear-Test-Event] => Clear PowerAdjustment struct");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutLocalOptimization:
        ChipLogProgress(Support, "[UserOptOutLocalOptimization-Test-Event] => Set User opt-out Local Optimization");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutGridOptimization:
        ChipLogProgress(Support, "[UserOptOutGrisOptimization-Test-Event] => Set User opt-out Grid Optimization");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutClearAll:
        ChipLogProgress(Support, "[UserOptOutClearAll-Test-Event] => Clear all User opt-outs");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustment:
        ChipLogProgress(Support, "[StartTimeAdjustment-Test-Event] => Create simulated forecast to allow StartTimeAdjustment");
        SetTestEventTrigger_StartTimeAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustmentClear:
        ChipLogProgress(Support, "[StartTimeAdjustmentClear-Test-Event] => Clear StartTimeAdjustment forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausable:
        ChipLogProgress(Support, "[Pausable-Test-Event] => Create Pausable forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausableNextSlot:
        ChipLogProgress(Support, "[PausableNextSlot-Test-Event] => Move to next Pausable slot in forecast");
        // TODO call implementation
        break;
    case DeviceEnergyManagementTrigger::kPausableClear:
        ChipLogProgress(Support, "[PausableClear-Test-Event] => Clear Pausable forecast");
        // TODO call implementation
        break;

    default:
        return false;
    }

    return true;
}
