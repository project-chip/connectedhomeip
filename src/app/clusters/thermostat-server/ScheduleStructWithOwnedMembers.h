/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *        Structs::ScheduleStruct::Type and manages the storage of the schedule handle
 *        member which it owns.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once
#include <app/util/attribute-storage.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kScheduleHandleSize = 16;

static constexpr size_t kSchedulePresetHandleSize = 16;

static constexpr size_t kScheduleNameSize = 64;

struct ScheduleStructWithOwnedMembers : protected Structs::ScheduleStruct::Type
{
public:
    ScheduleStructWithOwnedMembers() = default;
    ScheduleStructWithOwnedMembers(const Structs::ScheduleStruct::Type & other);
    ScheduleStructWithOwnedMembers & operator=(const Structs::ScheduleStruct::Type & other);
    ScheduleStructWithOwnedMembers & operator=(const ScheduleStructWithOwnedMembers & other);
    void SetTransitionData(Structs::ScheduleTransitionStruct::Type * pBuffer, size_t bufSize);
    void SetSystemMode(SystemModeEnum enumValue);
    CHIP_ERROR SetScheduleHandle(const DataModel::Nullable<ByteSpan> & newScheduleHandle);
    CHIP_ERROR SetName(const Optional<CharSpan> & newName);
    CHIP_ERROR SetPresetHandle(const Optional<ByteSpan> & newPresetHandle);
    CHIP_ERROR SetTransitions(const DataModel::List<const Structs::ScheduleTransitionStruct::Type> & newTransitions);
    void SetBuiltIn(DataModel::Nullable<bool> newBuiltIn);
    Structs::ScheduleTransitionStruct::Type * GetTransitionData() const;
    DataModel::Nullable<ByteSpan> GetScheduleHandle() const;
    SystemModeEnum GetSystemMode() const;
    Optional<CharSpan> GetName() const;
    Optional<ByteSpan> GetPresetHandle() const;
    DataModel::List<const Structs::ScheduleTransitionStruct::Type> GetTransitions() const;
    DataModel::Nullable<bool> GetBuiltIn() const;

    using Structs::ScheduleStruct::Type::Encode;
    using Structs::ScheduleStruct::Type::kIsFabricScoped;

private:
    uint8_t scheduleHandleData[kScheduleHandleSize] = { 0 };
    char scheduleNameData[kScheduleNameSize];
    uint8_t schedulePresetHandleData[kSchedulePresetHandleSize] = { 0 };
    Structs::ScheduleTransitionStruct::Type * scheduleTransitionData;
    size_t scheduleTransitionDataMaxSize = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
