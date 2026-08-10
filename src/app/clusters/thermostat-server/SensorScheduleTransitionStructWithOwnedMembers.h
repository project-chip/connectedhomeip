/**
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

#include <stddef.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <clusters/Thermostat/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kMaxEnabledSensorsPerTransition = 32;
static constexpr size_t kMaxSensorHandleSize            = 16;

struct SensorScheduleTransitionStructWithOwnedMembers : protected Structs::SensorScheduleTransitionStruct::Type
{
public:
    SensorScheduleTransitionStructWithOwnedMembers() = default;
    SensorScheduleTransitionStructWithOwnedMembers(const Structs::SensorScheduleTransitionStruct::Type & other);
    SensorScheduleTransitionStructWithOwnedMembers(const Structs::SensorScheduleTransitionStruct::DecodableType & other);
    SensorScheduleTransitionStructWithOwnedMembers(const SensorScheduleTransitionStructWithOwnedMembers & other) = delete;
    SensorScheduleTransitionStructWithOwnedMembers & operator=(const Structs::SensorScheduleTransitionStruct::Type & other);
    SensorScheduleTransitionStructWithOwnedMembers &
    operator=(const Structs::SensorScheduleTransitionStruct::DecodableType & other);
    SensorScheduleTransitionStructWithOwnedMembers & operator=(const SensorScheduleTransitionStructWithOwnedMembers & other);

    void SetDayOfWeek(BitMask<ScheduleDayOfWeekBitmap> newDayOfWeek);
    void SetTransitionTime(uint16_t newTransitionTime);
    CHIP_ERROR SetEnabledSensors(const DataModel::List<const ByteSpan> & newEnabledSensors);
    CHIP_ERROR SetEnabledSensors(const DataModel::DecodableList<ByteSpan> & newEnabledSensors);
    CHIP_ERROR SetEnabledSensors(Span<const ByteSpan> newEnabledSensors);

    BitMask<ScheduleDayOfWeekBitmap> GetDayOfWeek() const;
    uint16_t GetTransitionTime() const;
    DataModel::List<const ByteSpan> GetEnabledSensors() const;

    using Structs::SensorScheduleTransitionStruct::Type::Encode;
    using Structs::SensorScheduleTransitionStruct::Type::kIsFabricScoped;

private:
    void RefreshEnabledSensorsList();

    uint8_t mSensorHandlesData[kMaxEnabledSensorsPerTransition][kMaxSensorHandleSize] = { { 0 } };
    ByteSpan mSensorHandles[kMaxEnabledSensorsPerTransition];
    size_t mNumEnabledSensors = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
