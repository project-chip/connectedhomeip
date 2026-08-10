/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/thermostat-server/SensorScheduleTransitionStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatClusterSensors.h>
#include <app/clusters/thermostat-server/ThermostatSensorStructWithOwnedMembers.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr uint8_t kMaxNumberOfSensors                    = 10;
static constexpr uint8_t kMaxNumberOfSensorScheduleTransitions = 10;

class ThermostatSensorsDelegate : public ThermostatSensors::Delegate
{
public:
    ThermostatSensorsDelegate(EndpointId endpoint);

    CHIP_ERROR GetSensorAtIndex(size_t index, ThermostatSensorStructWithOwnedMembers & sensor) override;
    CHIP_ERROR GetAvailableSensorAtIndex(size_t index, ByteSpan & sensorHandle) override;
    bool SetAvailableSensors(Span<const ByteSpan> availableSensors) override;
    CHIP_ERROR GetEnabledSensorAtIndex(size_t index, ByteSpan & sensorHandle) override;
    bool SetEnabledSensors(Span<const ByteSpan> enabledSensors) override;
    uint8_t GetNumberOfSensorScheduleTransitions() override;

    CHIP_ERROR GetSensorScheduleTransitionAtIndex(size_t index,
                                                  SensorScheduleTransitionStructWithOwnedMembers & transition) override;
    CHIP_ERROR GetPendingSensorScheduleTransitionAtIndex(size_t index,
                                                         SensorScheduleTransitionStructWithOwnedMembers & transition) override;
    void InitializePendingSensorScheduleTransitions() override;
    void ClearPendingSensorScheduleTransitions() override;
    CHIP_ERROR
    AppendToPendingSensorScheduleTransitions(const SensorScheduleTransitionStructWithOwnedMembers & transition) override;
    CHIP_ERROR CommitPendingSensorScheduleTransitions() override;

private:
    void InitializeSensors();

    uint8_t mNumberOfSensors = 0;
    ThermostatSensorStructWithOwnedMembers mSensors[kMaxNumberOfSensors];
    uint8_t mAvailableSensorsData[kMaxNumberOfSensors][kMaxSensorHandleSize] = { { 0 } };
    size_t mAvailableSensorsLen[kMaxNumberOfSensors]                         = { 0 };
    size_t mNumAvailableSensors                                             = 0;

    uint8_t mEnabledSensorsData[kMaxNumberOfSensors][kMaxSensorHandleSize] = { { 0 } };
    size_t mEnabledSensorsLen[kMaxNumberOfSensors]                         = { 0 };
    size_t mNumEnabledSensors                                             = 0;

    uint8_t mNumberOfSensorScheduleTransitions = kMaxNumberOfSensorScheduleTransitions;
    SensorScheduleTransitionStructWithOwnedMembers mSensorScheduleTransitions[kMaxNumberOfSensorScheduleTransitions];
    uint8_t mNextFreeIndexInSensorScheduleTransitionsList = 0;

    SensorScheduleTransitionStructWithOwnedMembers mPendingSensorScheduleTransitions[kMaxNumberOfSensorScheduleTransitions];
    uint8_t mNextFreeIndexInPendingSensorScheduleTransitionsList = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
