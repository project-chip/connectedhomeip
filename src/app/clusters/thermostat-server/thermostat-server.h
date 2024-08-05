/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <transport/Session.h>

#include <protocols/interaction_model/StatusCode.h>

struct ThermostatMatterScheduleManager
{
    /**
     * @brief Construct a new ThermostatMatterScheduleManager object *
     */

    // If the endpoint supports both editable presets and editable schedules
    ThermostatMatterScheduleManager() {};
    virtual ~ThermostatMatterScheduleManager() {};

    virtual bool IsEditing() = 0;  // is any endpoint currently being edited?
    virtual bool IsEditing(chip::EndpointId aEndpoint) = 0;
    virtual CHIP_ERROR StartEditing(chip::EndpointId aEndpoint, const chip::Access::SubjectDescriptor &inDescriptor) = 0;
    virtual bool IsActiveSubjectDescriptor(chip::EndpointId aEndpoint, const chip::Access::SubjectDescriptor &inDescriptor) = 0;

    virtual CHIP_ERROR RollbackEdits() = 0; // rollback all edits
    virtual CHIP_ERROR RollbackEdits(chip::EndpointId aEndpoint) = 0;    

    virtual chip::Protocols::InteractionModel::Status CommitEdits(chip::EndpointId aEndpoint) = 0;

    // presets
    virtual CHIP_ERROR GetPresetTypeAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::Thermostat::Structs::PresetTypeStruct::Type & outPresetType) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR GetPresetAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::Thermostat::Structs::PresetStruct::Type & outPreset) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR ClearPresets(chip::EndpointId aEndpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual chip::Protocols::InteractionModel::Status AppendPreset(chip::EndpointId aEndpoint, const chip::app::Clusters::Thermostat::Structs::PresetStruct::DecodableType & preset) { return chip::Protocols::InteractionModel::Status::UnsupportedWrite; }

    // schedules
    virtual CHIP_ERROR GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, chip::app::Clusters::Thermostat::Structs::ScheduleTypeStruct::Type & scheduleType) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type & schedule) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR ClearSchedules(chip::EndpointId aEndpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual chip::Protocols::InteractionModel::Status AppendSchedule(chip::EndpointId aEndpoint, const chip::app::Clusters::Thermostat::Structs::ScheduleStruct::DecodableType & schedule) { return chip::Protocols::InteractionModel::Status::UnsupportedWrite; }

    // Default implmenetation provided that will do attribute writes as required by the spec once tha handle has changed.
    // Override this with your own implementation if these writes are not necessary or will cause issues in your implemenation.
    // Note: these are called by the thermostat server implementation on every write to the ActviePresetHandle and 
    // ActiveScheduleHandle attriutes.
    virtual CHIP_ERROR PresetHandleChanged(chip::EndpointId aEndpoint);
    virtual CHIP_ERROR ScheduleHandleChanged(chip::EndpointId aEndpoint);

    virtual CHIP_ERROR GetPresetStructByHandle(chip::EndpointId aEndpoint, const chip::ByteSpan &inHandle, chip::app::Clusters::Thermostat::Structs::PresetStruct::Type & outPreset) const;
    virtual CHIP_ERROR GetScheduleStructByHandle(chip::EndpointId aEndpoint, const chip::ByteSpan &inHandle, chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type & outSchedule) const;

    static void SetActiveInstance(ThermostatMatterScheduleManager * inManager);
    static ThermostatMatterScheduleManager * GetActiveInstance();


protected:
    chip::Protocols::InteractionModel::Status ValidatePresetsForCommitting(chip::EndpointId aEndpoint, chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & oldList, chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & newList);
    chip::Protocols::InteractionModel::Status ValidateSchedulesForCommitting(chip::EndpointId aEndpoint, chip::Span<chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type> & oldList,
                                   chip::Span<chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type> & newList,
                                   chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & presetList);

    bool areDescriptorsEqualAndValid(const chip::Access::SubjectDescriptor &desc1, const chip::Access::SubjectDescriptor &desc2);
};
