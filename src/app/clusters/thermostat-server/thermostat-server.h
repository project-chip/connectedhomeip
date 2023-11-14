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
#include <app/util/basic-types.h>

struct ThermostatMatterScheduleEditor
{
    enum editType
    {
        Presets,
        Schedules
    };

    /**
     * Callbacks are not thread safe. To access the ThermostatMatterScheduleEditor struct please
     * consider using the LockChipStack / UnlockChipStack functions of the PlatformMgr.
     */
    using onEditStartCb    = void (*)(ThermostatMatterScheduleEditor *, editType aType);
    using onEditCancelCb     = onEditStartCb;
    using onEditCommitCb = onEditStartCb;

    /**
     * @brief Construct a new ThermostatMatterScheduleEditor object
     *
     * Please note: The app should create a instance of this object to handle
     * the schedules for each endpoint a thermostat cluster is instantiated on.
     *
     * @param endpoint endpoint of the cluster
     * @param onEditStart callback to indicate to the app to start editing the schedule
     * @param onEditCancel callback to indicate to the app to cancel editing the schedule
     * @param onEditCommit callback to indicate to the app to commit the new schedule
     */
    ThermostatMatterScheduleEditor(chip::EndpointId endpoint, onEditStartCb onEditStart, onEditCancelCb onEditCancel,
                                onEditCommitCb onEditCommit);
    ~ThermostatMatterScheduleEditor();

    chip::EndpointId mEndpoint;
    onEditStartCb mOnEditStartCb = nullptr;
    onEditCancelCb mOnEditCancelCb = nullptr;
    onEditCommitCb mOnEditCommitCb = nullptr;

    ThermostatMatterScheduleEditor * nextEditor = nullptr;

    bool hasNext() { return this->nextEditor != nullptr; }
    ThermostatMatterScheduleEditor * next() { return this->nextEditor; }
    void setNext(ThermostatMatterScheduleEditor * inst) { this->nextEditor = inst; }
};