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

    // TODO: Verify this is the right object to be tracking to ensure that the client 
    // that started editing is the only one that can edit until it sends a cancel or commit
    chip::SessionHolder mSession;

    virtual bool IsEditing() = 0;  // is any endpoint currently being edited?
    virtual bool IsEditing(chip::EndpointId aEndpoint) = 0;
    virtual CHIP_ERROR StartEditing(chip::EndpointId aEndpoint) = 0;

    virtual CHIP_ERROR RollbackEdits() = 0; // rollback all edits
    virtual CHIP_ERROR RollbackEdits(chip::EndpointId aEndpoint) = 0;    

//    virtual CHIP_ERROR ValidateEdits(chip::EndpointId aEndpoint) = 0;
    virtual chip::Protocols::InteractionModel::Status CommitEdits(chip::EndpointId aEndpoint) = 0;

    // presets
    virtual CHIP_ERROR GetPresetTypeAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::Thermostat::Structs::PresetTypeStruct::Type & outPresetType) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR GetPresetAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::Thermostat::Structs::PresetStruct::Type & outPreset) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR ClearPresets(chip::EndpointId aEndpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR AppendPreset(chip::EndpointId aEndpoint, const chip::app::Clusters::Thermostat::Structs::PresetStruct::DecodableType & preset) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // schedules
    virtual CHIP_ERROR GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, chip::app::Clusters::Thermostat::Structs::ScheduleTypeStruct::Type & scheduleType) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type & schedule) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR ClearSchedules(chip::EndpointId aEndpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR AppendSchedule(chip::EndpointId aEndpoint, const chip::app::Clusters::Thermostat::Structs::ScheduleStruct::DecodableType & schedule) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    static void SetActiveInstance(ThermostatMatterScheduleManager * inManager);
    static ThermostatMatterScheduleManager * GetActiveInstance();

protected:
    chip::Protocols::InteractionModel::Status ValidatePresetsForCommitting(chip::EndpointId aEndpoint, chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & oldList, chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & newList);
    chip::Protocols::InteractionModel::Status ValidateSchedulesForCommitting(chip::EndpointId aEndpoint, chip::Span<chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type> & oldList,
                                   chip::Span<chip::app::Clusters::Thermostat::Structs::ScheduleStruct::Type> & newList,
                                   chip::Span<chip::app::Clusters::Thermostat::Structs::PresetStruct::Type> & presetList);

};
