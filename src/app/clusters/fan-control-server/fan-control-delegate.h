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

#pragma once

#include <app/CommandResponseHelper.h>
#include <app/data-model/Nullable.h>
#include <clusters/FanControl/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

struct FanDriveState
{
    FanModeEnum mode;
    DataModel::Nullable<chip::Percent> percentSetting;
    chip::Percent percentCurrent;
    DataModel::Nullable<uint8_t> speedSetting;
    uint8_t speedCurrent;
};

/** @brief
 *    Defines methods for implementing application-specific logic for the FanControl Cluster.
 */
class Delegate
{
public:
    /**
     * @brief
     *   This method handles the step command. This will happen as fast as possible.
     *
     *   @param[in]  aDirection     the direction in which the speed should step
     *   @param[in]  aWrap          whether the speed should wrap or not
     *   @param[in]  aLowestOff     whether the device should consider the lowest setting as off
     *
     *   @return Success On success.
     *   @return Other Value indicating it failed to execute the command.
     */
    virtual Protocols::InteractionModel::Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) = 0;

    virtual void OnPersistenceRestored() {}

    /**
     * @brief Called when the fan transitions between on and off states.
     *        Implementations should update the OnOff cluster attribute accordingly.
     *        This is separate from OnFanDriveStateChanged to make the OnOff sync responsibility explicit.
     */
    virtual void OnFanStateChanged(bool isOn) {}

    virtual void OnFanDriveStateChanged(const FanDriveState & newState) {}
    virtual void OnRockSettingChanged(BitMask<RockBitmap> newValue) {}
    virtual void OnWindSettingChanged(BitMask<WindBitmap> newValue) {}
    virtual void OnAirflowDirectionChanged(AirflowDirectionEnum newValue) {}

    Delegate(EndpointId aEndpoint) : mEndpoint(aEndpoint) {}

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
