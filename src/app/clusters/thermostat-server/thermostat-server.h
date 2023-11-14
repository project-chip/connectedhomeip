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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>

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
    using onEditCommitCb = onEditStartCb;

    using getPresetTypeAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, PresetTypeStruct::Type &scheduleType);
    using getPresetAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, PresetStruct::Type &schedule);
    using setPresetAtIndexCB = getPresetAtIndexCB;

    using getScheduleTypeAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, ScheduleTypeStruct::Type &scheduleType);
    using getScheduleAtIndexCB = CHIP_ERROR (*)(ThermostatMatterScheduleManager *, size_t index, ScheduleStruct::Type &schedule);
    using setScheduleAtIndexCB = getScheduleAtIndexCB;

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
    ThermostatMatterScheduleManager( chip::EndpointId endpoint, 
                                    onEditStartCb onEditStart, 
                                    onEditCancelCb onEditCancel,
                                    onEditCommitCb onEditCommit,

                                    getPresetTypeAtIndexCB getPresetTypeAtIndex,
                                    getPresetAtIndexCB getPresetAtIndex,
                                    setPresetAtIndexCB setPresetAtIndex,

                                    getScheduleTypeAtIndexCB getScheduleTypeAtIndexCB,
                                    getScheduleAtIndexCB getScheduleAtIndex,
                                    setScheduleAtIndexCB setScheduleAtIndex);
    ~ThermostatMatterScheduleManager();

    chip::EndpointId mEndpoint;
    onEditStartCb mOnEditStartCb = nullptr;
    onEditCancelCb mOnEditCancelCb = nullptr;
    onEditCommitCb mOnEditCommitCb = nullptr;

    getPresetTypeAtIndexCB mGetPresetTypeAtIndexCb = nullptr;
    getPresetAtIndexCB mGetPresetAtIndexCb = nullptr;
    setPresetAtIndexCB mSetPresetAtIndexCb = nullptr;

    getScheduleTypeAtIndexCB mGetScheduleTypeAtIndexCb = nullptr;
    getScheduleAtIndexCB mGetScheduleAtIndexCb = nullptr;
    setScheduleAtIndexCB mSetScheduleAtIndexCb = nullptr;

    ThermostatMatterScheduleManager * nextEditor = nullptr;

    bool hasNext() { return this->nextEditor != nullptr; }
    ThermostatMatterScheduleManager * next() { return this->nextEditor; }
    void setNext(ThermostatMatterScheduleManager * inst) { this->nextEditor = inst; }
};