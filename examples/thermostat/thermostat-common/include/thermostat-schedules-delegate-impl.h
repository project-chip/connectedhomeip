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

#include <app/clusters/thermostat-server/ScheduleStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatClusterSchedules.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr uint8_t kMaxNumberOfScheduleTypes = 2;

// TODO: #34556 Support multiple presets/schedules of each type.
// We will support only one schedule of each schedule type.
static constexpr uint8_t kMaxNumberOfSchedulesOfEachType = 1;

// For testing the use case where number of schedules added exceeds the number of schedules supported, we will have the value
// of kMaxNumberOfSchedulesSupported < kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType
static constexpr uint8_t kMaxNumberOfSchedulesSupported = kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType - 1;

static constexpr uint8_t kMaxNumberOfScheduleTransitionsSupported = 10;

static constexpr uint8_t kMaxNumberOfScheduleTransitionsPerDaySupported = 4;

/*
 * A simple implementation of ThermostatSchedules::Delegate.
 * It reports and persists the state of the thermostat's attributes tracking setpoint schedules.
 */
class ThermostatSchedulesDelegate : public ThermostatSchedules::Delegate
{
public:
    ThermostatSchedulesDelegate(EndpointId endpoint);

    // ThermostatSchedules::Delegate methods
    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;
    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override;
    uint8_t GetNumberOfSchedules() override;
    uint8_t GetNumberOfScheduleTransitions() override;
    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionsPerDay() override;
    CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override;
    CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) override;
    CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle) override;
    void InitializePendingSchedules() override;
    CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) override;
    CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override;
    CHIP_ERROR CommitPendingSchedules() override;
    void ClearPendingScheduleList() override;

private:
    EndpointId mEndpointId;

    /**
     * @brief Initializes the schedule types array with example schedule types.
     */
    void InitializeScheduleTypes();

    /**
     * @brief Initializes the schedules array with a sample built-in schedule for testing.
     */
    void InitializeSchedules();

    uint8_t mMaxNumberOfSchedulesAllowedPerScheduleType;
    uint8_t mNumberOfSchedules;
    Structs::ScheduleTypeStruct::Type mScheduleTypes[kMaxNumberOfScheduleTypes];
    ScheduleStructWithOwnedMembers mSchedules[kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType];
    ScheduleStructWithOwnedMembers mPendingSchedules[kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType];

    uint8_t mNextFreeIndexInPendingSchedulesList;
    uint8_t mNextFreeIndexInSchedulesList;

    uint8_t mActiveScheduleHandleData[kScheduleHandleSize];
    size_t mActiveScheduleHandleDataSize;
    bool mActiveScheduleHandleIsNull = true;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
