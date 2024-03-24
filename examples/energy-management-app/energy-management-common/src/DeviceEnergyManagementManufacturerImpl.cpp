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

#include "app/clusters/device-energy-management-server/device-energy-management-server.h"
#include <DeviceEnergyManagementManufacturerImpl.h>
#include <DeviceEnergyManagementDelegateImpl.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include "utils.h"


using chip::Protocols::InteractionModel::Status;

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

// TODO: refactor, once the best approach is clear
extern void ForecastTestSetup_TP3b(DataModel::Nullable<Structs::ForecastStruct::Type> & nullableForecast);

static DataModel::Nullable<Structs::ForecastStruct::Type>  sForecast;
static OptOutStateEnum sOptOutState = OptOutStateEnum::kNoOptOut;

struct DeviceEnergyManagementTestEventSaveData
{
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast;
};

static PowerAdjustmentCapability::TypeInfo::Type  sPowerAdjustmentCapability;

struct DeviceEnergyManagementTestEventPowerAdjustRequest
{
    int64_t  power;
    uint32_t duration;
    AdjustmentCauseEnum cause;
};

static DeviceEnergyManagementTestEventSaveData    sDeviceEnergyManagementTestEventSaveData;

DeviceEnergyManagementDelegate * DeviceEnergyManagementManufacturer::sDelegate = nullptr;

CHIP_ERROR DeviceEnergyManagementManufacturer::Init()
{
    /* Manufacturers should modify this to do any custom initialisation */

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

void SetTestEventTrigger_PowerAdjustment()
{
    ChipLogProgress(Support, "[PowerAdjustment-handle] L-%d", __LINE__ );
    // TODO: implement
}

void SetTestEventTrigger_StartTimeAdjustment()
{
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();

    sForecast = dg->GetForecast();
    ForecastTestSetup_TP3b(sForecast);
}

void SetTestEventTrigger_StartTimeAdjustmentClear()
{
    ChipLogProgress(Support, "[StartTimeAdjustmentClear-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();

    if (CHIP_NO_ERROR != dg->SetForecast(sForecast))
    {
        ChipLogProgress(Support, "[StartTimeAdjustmentClear-handle] L-%d Failed to restore forecast!", __LINE__ );
    }
}

void SetTestEventTrigger_UserOptOutOptimization( OptOutStateEnum optOutState)
{
    ChipLogProgress(Support, "[UserOptOutOptimization-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();
    sOptOutState = dg->GetOptOutState();
    dg->SetOptOutState(optOutState);
}

void SetTestEventTrigger_PowerAdjustRequest()
{
    ChipLogProgress(Support, "[PowerAdjustRequest-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();
    sPowerAdjustmentCapability = dg->GetPowerAdjustmentCapability();
    // TODO: implement
}


bool HandleDeviceEnergyManagementTestEventTrigger(uint64_t eventTrigger)
{
    DeviceEnergyManagementTrigger trigger = static_cast<DeviceEnergyManagementTrigger>(eventTrigger);

    switch (trigger)
    {
    case DeviceEnergyManagementTrigger::kPowerAdjustment:
        ChipLogProgress(Support, "[PowerAdjustment-Test-Event] => Create PowerAdjustment struct");
        SetTestEventTrigger_PowerAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kPowerAdjustmentClear:
        ChipLogProgress(Support, "[PowerAdjustmentClear-Test-Event] => Clear PowerAdjustment struct");
	    SetTestEventTrigger_PowerAdjustRequest();
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutLocalOptimization:
        ChipLogProgress(Support, "[UserOptOutLocalOptimization-Test-Event] => Set User opt-out Local Optimization");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kLocalOptOut);
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutGridOptimization:
        ChipLogProgress(Support, "[UserOptOutGrisOptimization-Test-Event] => Set User opt-out Grid Optimization");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kGridOptOut);
        break;
    case DeviceEnergyManagementTrigger::kUserOptOutClearAll:
        ChipLogProgress(Support, "[UserOptOutClearAll-Test-Event] => Clear all User opt-outs");
        SetTestEventTrigger_UserOptOutOptimization(OptOutStateEnum::kNoOptOut);
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustment:
        ChipLogProgress(Support, "[StartTimeAdjustment-Test-Event] => Create simulated forecast to allow StartTimeAdjustment");
        SetTestEventTrigger_StartTimeAdjustment();
        break;
    case DeviceEnergyManagementTrigger::kStartTimeAdjustmentClear:
        ChipLogProgress(Support, "[StartTimeAdjustmentClear-Test-Event] => Clear StartTimeAdjustment forecast");
        SetTestEventTrigger_StartTimeAdjustmentClear();
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
