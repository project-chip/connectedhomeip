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

    /**
     * Notifies the application when fan drive-related attributes change due to cluster processing (for example
     * FanMode, PercentSetting / PercentCurrent, or SpeedSetting / SpeedCurrent after a successful write or
     * FanMode side effects). Not called when the delegate callback is temporarily suppressed to avoid re-entrancy.
     *
     * @param[in] newState Current fan drive snapshot after the update.
     */
    virtual void OnFanDriveStateChanged(const FanDriveState & newState) {}

    /**
     * Notifies the application after RockSetting was successfully updated (Rock feature).
     *
     * @param[in] newValue Committed RockSetting value.
     */
    virtual void OnRockSettingChanged(BitMask<RockBitmap> newValue) {}

    /**
     * Notifies the application after WindSetting was successfully updated (Wind feature).
     *
     * @param[in] newValue Committed WindSetting value.
     */
    virtual void OnWindSettingChanged(BitMask<WindBitmap> newValue) {}

    /**
     * Notifies the application after AirflowDirection was successfully updated (AirflowDirection feature).
     *
     * @param[in] newValue Committed AirflowDirection value.
     */
    virtual void OnAirflowDirectionChanged(AirflowDirectionEnum newValue) {}

    Delegate(EndpointId aEndpoint = kInvalidEndpointId) : mEndpoint(aEndpoint) {}

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
