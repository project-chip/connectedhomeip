/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

// Default implmenetation details for the THermostatMatterScheduleManager.
// Note: ValidatePresetsForCommitting and ValidateSchedulesForCommitting are implemented 
// in seperate .cpp files due to their complexity and for readability.  All other methods 
// that aren't pure virtual have default implementations either in 
// this file or the header file.

// ----------------------------------------------
// - Schedules and Presets Manager object       -
// ----------------------------------------------

#include "thermostat-server.h"
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPEncoding.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;

using imcode = Protocols::InteractionModel::Status;

// Object Tracking
static ThermostatMatterScheduleManager * gsMatterScheduleEditor = nullptr;

void 
ThermostatMatterScheduleManager::SetActiveInstance(ThermostatMatterScheduleManager * inManager)
{
    if (gsMatterScheduleEditor != nullptr)
    {
        if (gsMatterScheduleEditor->IsEditing())
        {
            ChipLogError(Zcl, "Warning: Active ThermostatMatterScheduleManager was editing when swapped out");
            gsMatterScheduleEditor->RollbackEdits();
        }
    }
    gsMatterScheduleEditor = inManager;
}

ThermostatMatterScheduleManager * ThermostatMatterScheduleManager::GetActiveInstance()
{
    return gsMatterScheduleEditor;
}

bool 
ThermostatMatterScheduleManager::areDescriptorsEqualAndValid(const Access::SubjectDescriptor &desc1, const Access::SubjectDescriptor &desc2)
{
    if (desc1.fabricIndex == kUndefinedFabricIndex || desc1.subject == kUndefinedNodeId)
        return false;
    if (desc2.fabricIndex == kUndefinedFabricIndex || desc2.subject == kUndefinedNodeId)
        return false;
    return ((desc1.fabricIndex == desc2.fabricIndex) && (desc1.subject == desc2.subject));
}

CHIP_ERROR 
ThermostatMatterScheduleManager::PresetHandleChanged(chip::EndpointId aEndpoint)
{
    CHIP_ERROR retVal = CHIP_NO_ERROR;
#if 0
    imcode imStatus = imcode::InvalidCommand;

    chip::ByteSpan presetHandle;
    PresetStruct::Type preset;

    imStatus = ActivePresetHandle::Get(aEndpoint, presetHandle);
    retVal = StatusIB(imStatus).ToChipError();
    SuccessOrExit(retVal);

    retVal = GetPresetStructByHandle(aEndpoint, presetHandle, preset);
    SuccessOrExit(retVal);

    // I think we will need this logic in another method since editing the active preset would also need this:
    switch (preset.presetScenario)
    {
        case PresetScenarioEnum::kOccupied: 
            if (preset.coolingSetpoint.HasValue())
               OccupiedCoolingSetpoint::Set(aEndpoint, preset.coolingSetpoint.GetValue()); 
            if (preset.heatingSetpoint.HasValue())
               OccupiedHeatingSetpoint::Set(aEndpoint, preset.heatingSetpoint.GetValue()); 
            break;
        case PresetScenarioEnum::kUnoccupied: 
            if (preset.coolingSetpoint.HasValue())
               UnoccupiedCoolingSetpoint::Set(aEndpoint, preset.coolingSetpoint.GetValue()); 
            if (preset.heatingSetpoint.HasValue())
               UnoccupiedHeatingSetpoint::Set(aEndpoint, preset.heatingSetpoint.GetValue()); 
            break;
    }

exit:
#endif
    return retVal;
}

CHIP_ERROR 
ThermostatMatterScheduleManager::ScheduleHandleChanged(chip::EndpointId aEndpoint)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR 
ThermostatMatterScheduleManager::GetPresetStructByHandle(chip::EndpointId aEndpoint, const chip::ByteSpan &inHandle, PresetStruct::Type & outPreset) const
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR 
ThermostatMatterScheduleManager::GetScheduleStructByHandle(chip::EndpointId aEndpoint, const chip::ByteSpan &inHandle, ScheduleStruct::Type & outSchedule) const
{
    return CHIP_NO_ERROR;
}
