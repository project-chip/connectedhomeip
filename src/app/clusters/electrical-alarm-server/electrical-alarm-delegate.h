/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

/** @brief
 *  Defines methods for implementing application-specific logic for the ElectricalAlarm Cluster.
 */
class Delegate
{
public:
    /**
     *   @brief
     *   A notification that the Mask attribute will be changed.
     *   @param[in] mask The new value of the Mask attribute.
     *   @return true if the cluster should apply the change; false to reject.
     */
    virtual bool ModifyEnabledAlarmsCallback(const BitMask<AlarmBitmap> mask) = 0;

    /**
     *   @brief
     *   A notification that the server should reset the given latched alarms.
     *   @param[in] alarms Bitmask of alarms to reset.
     *   @return true if the cluster should apply the reset; false to reject.
     */
    virtual bool ResetAlarmsCallback(const BitMask<AlarmBitmap> alarms) = 0;

    /**
     *   @brief
     *   A notification that the server should apply new threshold values
     *   (ADJUST feature, SetElectricalAlarmThresholds command).
     *   Only fields that are present (HasValue() == true) need to be applied.
     *   @param[in] commandData Decoded command fields; absent optionals retain
     *                          their current attribute value.
     *   @return true if the cluster should persist the new thresholds; false to reject.
     *   Default implementation approves all threshold changes; override to add
     *   device-specific validation or to write values to hardware.
     */
    virtual bool SetElectricalAlarmThresholdsCallback(
        const Commands::SetElectricalAlarmThresholds::DecodableType & commandData) { return true; }

    Delegate(EndpointId endpoint) : mEndpoint(endpoint) {}

    Delegate() = default;

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
