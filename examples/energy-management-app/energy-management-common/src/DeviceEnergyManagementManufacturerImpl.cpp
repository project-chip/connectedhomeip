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

// TODO:  not need for such a thing in DEM?     dg->HwRegisterEvseCallbackHandler(ApplicationCallbackHandler, reinterpret_cast<intptr_t>(this));

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

void SetTestEventTrigger_zzzzzzzzzz()
{
    ChipLogProgress(Support, "[zzzzzzzzzz-handle] L-%d", __LINE__ );
}

void SetTestEventTrigger_StartTimeAdjustment()
{
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();

    //     virtual DataModel::Nullable<Structs::ForecastStruct::Type> GetForecast() override;
    sForecast = dg->GetForecast();
    // sDeviceEnergyManagementTestEventSaveData.forecast = dg->GetForecast();

#if 0 // TODO:
    if (!sForecast.IsNull())
    {
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d sForecast.forecastId = %d", __LINE__,  sForecast.Value().forecastId);
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d sForecast.startTime = %d", __LINE__ , sForecast.Value().startTime);
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d sForecast.endTime = %d", __LINE__, sForecast.Value().endTime );
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d sForecast.isPauseable = %s", __LINE__, sForecast.Value().isPauseable? "T":"F" );
#if 0 // TODO:
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d [optional/nullable] sForecast.earliestStartTime = %d", __LINE__,
                                     sForecast.Value().earliestStartTime.HasValue()?
                                        (!sForecast.Value().earliestStartTime.IsNull()?  sForecast.Value().earliestStartTime.Value().Value() : 555555)
                                        : 666666);
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d [optional] sForecast.latestEndTime = %d", __LINE__,
                                     sForecast.Value().latestEndTime.HasValue()?  sForecast.Value().latestEndTime.Value() : 666666);
#endif
    }
#endif
    ForecastTestSetup_TP3b(sForecast);

    uint32_t chipEpoch = 0;
    CHIP_ERROR ce = UtilsGetEpochTS(chipEpoch);
    ChipLogProgress(Support, "ce=%s  epoch = %d", (ce != CHIP_NO_ERROR)? "Err":"Good", chipEpoch);


    Status s = dg->StartTimeAdjustRequest(chipEpoch + 100000, AdjustmentCauseEnum::kLocalOptimization);
    if (s != Status::Success)
    {
        ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d StartTimeAdjustRequest() Failed", __LINE__ );
    }

    // DataModel::Nullable<Structs::ForecastStruct::Type>  newForecast = dg->GetForecast();
    // TODO: compare new/old forcasts? No that's done at the python level.

}

void SetTestEventTrigger_UserOptOutOptimization( OptOutStateEnum optOutState )
{
    ChipLogProgress(Support, "[UserOptOutOptimization-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();
    sOptOutState = dg->GetOptOutState();

    // ChipLogProgress(Support, "[UserOptOutOptimization-Test-Event] ");

    
// TODO:     
}

void SetTestEventTrigger_PowerAdjustRequest()
{
    ChipLogProgress(Support, "[PowerAdjustRequest-handle] L-%d", __LINE__ );

    DeviceEnergyManagementDelegate * dg = DeviceEnergyManagementManufacturer::GetDelegate();
    sPowerAdjustmentCapability = dg->GetPowerAdjustmentCapability();


    
// TODO:     const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause
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
