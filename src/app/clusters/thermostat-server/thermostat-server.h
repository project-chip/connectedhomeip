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
#include <app/util/af-enums.h>


using namespace chip::app::Clusters::Thermostat::Structs;

struct ThermostatMatterScheduleManager
{
    enum editType
    {
        Presets,
        Schedules
    };

    /**
     * Callbacks are not thread safe. To access the ThermostatMatterScheduleManager struct please
     * consider using the LockChipStack / UnlockChipStack functions of the PlatformMgr.
     */
    using onEditStartCb  = void (*)(ThermostatMatterScheduleManager *, editType aType);
    using onEditCancelCb = onEditStartCb;
    using onEditCommitCb = EmberAfStatus (*)(ThermostatMatterScheduleManager *, editType);

    using getPresetTypeAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, PresetTypeStruct::Type &presetType);
    using getPresetAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, PresetStruct::Type &preset);
    using clearPresetsCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *);
    using appendPresetCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, const PresetStruct::DecodableType &preset);

    using getScheduleTypeAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, ScheduleTypeStruct::Type &scheduleType);
    using getScheduleAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, ScheduleStruct::Type &schedule);
    using clearSchedulesCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *);
    using appendScheduleCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, const ScheduleStruct::DecodableType &schedule);

    /**
     * @brief Construct a new ThermostatMatterScheduleManager object
     *
     * Please note: The app should create a instance of this object to handle
     * the schedules for each endpoint a thermostat cluster is instantiated on.
     *
     * @param endpoint endpoint of the cluster
     * @param onEditStart callback to indicate to the app to start editing the schedules/presets
     * @param onEditCancel callback to indicate to the app to cancel editing the schedules/presets
     * @param onEditCommit callback to indicate to the app to commit the new schedules/presets
     * 
     * @param getPresetTypeAtIndex callback requesting from the app a preset type at a given index.
     * @param getPresetAtIndex callback requesting from the app a preset at a given index.
     * @param setPresetAtIndex callback requesting the app to set a preset at a given index.
     * 
     * @param getScheduleTypeAtIndex callback requesting from the app a preset type at a given index.
     * @param getScheduleAtIndex callback requesting from the app a preset at a given index.
     * @param setScheduleAtIndex callback requesting the app to set a preset at a given index.
     */

    // If the endpoint supports both editable presets and editable schedules
    ThermostatMatterScheduleManager( chip::EndpointId endpoint, 
                                    onEditStartCb onEditStart, 
                                    onEditCancelCb onEditCancel,
                                    onEditCommitCb onEditCommit,

                                    getPresetTypeAtIndexCB getPresetTypeAtIndex,
                                    getPresetAtIndexCB getPresetAtIndex,
                                    appendPresetCB appendPreset,
                                    clearPresetsCB clearPresets,

                                    getScheduleTypeAtIndexCB getScheduleTypeAtIndexCB,
                                    getScheduleAtIndexCB getScheduleAtIndex,
                                    appendScheduleCB appendSchedule,
                                    clearSchedulesCB clearSchedules);

    // If the endpoint supports editable schedules
    ThermostatMatterScheduleManager( chip::EndpointId endpoint, 
                                    onEditStartCb onEditStart, 
                                    onEditCancelCb onEditCancel,
                                    onEditCommitCb onEditCommit,

                                    getScheduleTypeAtIndexCB getScheduleTypeAtIndexCB,
                                    getScheduleAtIndexCB getScheduleAtIndex,
                                    appendScheduleCB appendSchedule,
                                    clearSchedulesCB clearSchedules);

    // If the endpoint supports editable presets
    ThermostatMatterScheduleManager( chip::EndpointId endpoint, 
                                    onEditStartCb onEditStart, 
                                    onEditCancelCb onEditCancel,
                                    onEditCommitCb onEditCommit,

                                    getPresetTypeAtIndexCB getPresetTypeAtIndex,
                                    getPresetAtIndexCB getPresetAtIndex,
                                    appendPresetCB appendPreset,
                                    clearPresetsCB clearPresets);

    ~ThermostatMatterScheduleManager();

    chip::EndpointId mEndpoint;
    onEditStartCb mOnEditStartCb = nullptr;
    onEditCancelCb mOnEditCancelCb = nullptr;
    onEditCommitCb mOnEditCommitCb = nullptr;

    getPresetTypeAtIndexCB mGetPresetTypeAtIndexCb = nullptr;
    getPresetAtIndexCB mGetPresetAtIndexCb = nullptr;
    appendPresetCB mAppendPresetCb = nullptr;
    clearPresetsCB mClearPresetsCb = nullptr;

    getScheduleTypeAtIndexCB mGetScheduleTypeAtIndexCb = nullptr;
    getScheduleAtIndexCB mGetScheduleAtIndexCb = nullptr;
    appendScheduleCB mAppendScheduleCb = nullptr;
    clearSchedulesCB mClearSchedulesCb = nullptr;

    ThermostatMatterScheduleManager * nextEditor = nullptr;

    bool hasNext() { return this->nextEditor != nullptr; }
    ThermostatMatterScheduleManager * next() { return this->nextEditor; }
    void setNext(ThermostatMatterScheduleManager * inst) { this->nextEditor = inst; }

    EmberAfStatus ValidatePresetsForCommitting(chip::Span<PresetStruct::Type> &oldlist, chip::Span<PresetStruct::Type> &newlist);
    EmberAfStatus ValidateSchedulesForCommitting(chip::Span<ScheduleStruct::Type> &oldlist, chip::Span<ScheduleStruct::Type> &newlist);
//    static CHIP_ERROR ValidateSchedulesForCommitting(oldlist, newlist);
};

