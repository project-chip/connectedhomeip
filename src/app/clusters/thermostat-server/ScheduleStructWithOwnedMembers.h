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

/****************************************************************************
 * @file
 * @brief This class has a struct ScheduleStructWithOwnedMembers that inherits from
 *        Structs::ScheduleStruct::Type and manages the storage of the schedule handle,
 *        name, preset handle and transitions members which it owns.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kScheduleHandleSize = 16;

static constexpr size_t kScheduleNameSize = 64;

// Maximum number of transitions a single ScheduleStructWithOwnedMembers can own. This bounds the embedded
// storage used to hold a deep copy of the Transitions list; devices that need denser schedules should raise it.
static constexpr size_t kScheduleTransitionsMax = 16;

struct ScheduleStructWithOwnedMembers : protected Structs::ScheduleStruct::Type
{
public:
    ScheduleStructWithOwnedMembers() = default;
    ScheduleStructWithOwnedMembers(const Structs::ScheduleStruct::Type & other);
    ScheduleStructWithOwnedMembers(const Structs::ScheduleStruct::DecodableType & other);
    // Copy construction deleted: a defaulted copy ctor would shallow-copy the inherited spans into the source's buffers.
    ScheduleStructWithOwnedMembers(const ScheduleStructWithOwnedMembers & other) = delete;
    ScheduleStructWithOwnedMembers & operator=(const Structs::ScheduleStruct::Type & other);
    ScheduleStructWithOwnedMembers & operator=(const Structs::ScheduleStruct::DecodableType & other);
    ScheduleStructWithOwnedMembers & operator=(const ScheduleStructWithOwnedMembers & other);

    void SetSystemMode(SystemModeEnum enumValue);
    CHIP_ERROR SetScheduleHandle(const DataModel::Nullable<ByteSpan> & newScheduleHandle);
    CHIP_ERROR SetName(const Optional<CharSpan> & newName);
    CHIP_ERROR SetPresetHandle(const Optional<ByteSpan> & newPresetHandle);
    CHIP_ERROR SetTransitions(const DataModel::List<const Structs::ScheduleTransitionStruct::Type> & newTransitions);
    CHIP_ERROR SetTransitions(const DataModel::DecodableList<Structs::ScheduleTransitionStruct::DecodableType> & newTransitions);
    void SetBuiltIn(DataModel::Nullable<bool> newBuiltIn);

    SystemModeEnum GetSystemMode() const;
    DataModel::Nullable<ByteSpan> GetScheduleHandle() const;
    Optional<CharSpan> GetName() const;
    Optional<ByteSpan> GetPresetHandle() const;
    DataModel::List<const Structs::ScheduleTransitionStruct::Type> GetTransitions() const;
    DataModel::Nullable<bool> GetBuiltIn() const;

    using Structs::ScheduleStruct::Type::Encode;
    using Structs::ScheduleStruct::Type::kIsFabricScoped;

private:
    CHIP_ERROR SetTransitionAtIndex(size_t index, const Structs::ScheduleTransitionStruct::Type & transition);

    uint8_t scheduleHandleData[kScheduleHandleSize] = { 0 };
    char scheduleNameData[kScheduleNameSize]        = { 0 };
    uint8_t presetHandleData[kScheduleHandleSize]   = { 0 };

    Structs::ScheduleTransitionStruct::Type transitionsData[kScheduleTransitionsMax];
    uint8_t transitionPresetHandleData[kScheduleTransitionsMax][kScheduleHandleSize] = {};
    size_t transitionsCount                                                          = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
