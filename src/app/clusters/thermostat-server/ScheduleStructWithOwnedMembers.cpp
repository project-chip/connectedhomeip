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

#include "ScheduleStructWithOwnedMembers.h"

using namespace chip;
using namespace chip::app;
using namespace DataModel;
using namespace chip::app::Clusters::Thermostat::Structs;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ScheduleStructWithOwnedMembers::ScheduleStructWithOwnedMembers(const ScheduleStruct::Type & other)
{
    *this = other;
}

ScheduleStructWithOwnedMembers & ScheduleStructWithOwnedMembers::operator=(const ScheduleStruct::Type & other)
{
    SetSystemMode(other.systemMode);
    CHIP_ERROR err = SetScheduleHandle(other.scheduleHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule handle with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    err = SetName(other.name);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule name with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    SetPresetHandle(other.presetHandle);
    err = SetTransitions(other.transitions);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule transitions with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    SetBuiltIn(other.builtIn);
    return *this;
}

ScheduleStructWithOwnedMembers & ScheduleStructWithOwnedMembers::operator=(const ScheduleStructWithOwnedMembers & other)
{
    if (this == &other)
    {
        return *this;
    }
    *this = static_cast<const ScheduleStruct::Type &>(other);
    return *this;
}

void ScheduleStructWithOwnedMembers::SetTransitionData(Structs::ScheduleTransitionStruct::Type * pBuffer, size_t bufSize)
{
    if (bufSize == 0)
    {
        scheduleTransitionData = NULL;
    }
    else
    {
        scheduleTransitionData = pBuffer;
    }
    scheduleTransitionDataMaxSize = bufSize;
}

void ScheduleStructWithOwnedMembers::SetSystemMode(SystemModeEnum enumValue)
{
    systemMode = enumValue;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetScheduleHandle(const Nullable<ByteSpan> & newScheduleHandle)
{
    if (!newScheduleHandle.IsNull())
    {
        size_t newScheduleHandleSize = newScheduleHandle.Value().size();
        if (newScheduleHandleSize > kScheduleHandleSize)
        {
            ChipLogError(Zcl, "Failed to set Schedule handle. New Schedule handle size (%u) > allowed schedule handle size (%u)",
                         static_cast<unsigned>(newScheduleHandleSize), static_cast<unsigned>(kScheduleHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableByteSpan targetSpan(scheduleHandleData);
        ReturnErrorOnFailure(CopySpanToMutableSpan(newScheduleHandle.Value(), targetSpan));
        scheduleHandle.SetNonNull(targetSpan);
    }
    else
    {
        scheduleHandle.SetNull();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetName(const Optional<CharSpan> & newName)
{
    if (newName.HasValue())
    {
        size_t newNameSize = newName.Value().size();
        if (newNameSize > kScheduleNameSize)
        {
            ChipLogError(Zcl, "Failed to set Schedule name. New name size (%u) > allowed schedule name size (%u)",
                         static_cast<unsigned>(newNameSize), static_cast<unsigned>(kScheduleNameSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableCharSpan targetSpan(scheduleNameData);
        CharSpan newNameSpan = newName.Value();
        ReturnErrorOnFailure(CopyCharSpanToMutableCharSpan(newNameSpan, targetSpan));

        name.SetValue(targetSpan);
    }
    else
    {
        name.ClearValue();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetPresetHandle(const Optional<ByteSpan> & newPresetHandle)
{
    if (newPresetHandle.HasValue())
    {
        size_t newPresetHandleSize = newPresetHandle.Value().size();
        if (newPresetHandleSize > kSchedulePresetHandleSize)
        {
            ChipLogError(Zcl, "Failed to set Preset handle. New preset handle size (%u) > allowed preset handle size (%u)",
                         static_cast<unsigned>(newPresetHandleSize), static_cast<unsigned>(kSchedulePresetHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableByteSpan targetSpan(schedulePresetHandleData);
        ReturnErrorOnFailure(CopySpanToMutableSpan(newPresetHandle.Value(), targetSpan));

        presetHandle.SetValue(targetSpan);
    }
    else
    {
        presetHandle.ClearValue();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetTransitions(
    const DataModel::List<const Structs::ScheduleTransitionStruct::Type> & newTransitions)
{
    if ((scheduleTransitionData == NULL) || (scheduleTransitionDataMaxSize == 0))
    {
        // temporary local instance, borrow the transitions
        transitions = newTransitions;
    }
    else
    {
        size_t scheduleTransitionSize = newTransitions.size();

        if (scheduleTransitionSize > scheduleTransitionDataMaxSize)
        {
            ChipLogError(Zcl, "Failed to set Schedule transitions. New transitions size (%u) > allowed transitions size (%u)",
                         static_cast<unsigned>(scheduleTransitionSize), static_cast<unsigned>(scheduleTransitionDataMaxSize));
            return CHIP_ERROR_NO_MEMORY;
        }

        for (size_t i = 0; i < scheduleTransitionSize; i++)
        {
            scheduleTransitionData[i] = newTransitions[i];
        }

        transitions =
            DataModel::List<const Structs::ScheduleTransitionStruct::Type>(scheduleTransitionData, scheduleTransitionSize);
    }

    return CHIP_NO_ERROR;
}

void ScheduleStructWithOwnedMembers::SetBuiltIn(DataModel::Nullable<bool> newBuiltIn)
{
    builtIn = newBuiltIn;
}

Structs::ScheduleTransitionStruct::Type * ScheduleStructWithOwnedMembers::GetTransitionData() const
{
    return scheduleTransitionData;
}

SystemModeEnum ScheduleStructWithOwnedMembers::GetSystemMode() const
{
    return systemMode;
}

DataModel::Nullable<ByteSpan> ScheduleStructWithOwnedMembers::GetScheduleHandle() const
{
    return scheduleHandle;
}
Optional<CharSpan> ScheduleStructWithOwnedMembers::GetName() const
{
    return name;
}

Optional<ByteSpan> ScheduleStructWithOwnedMembers::GetPresetHandle() const
{
    return presetHandle;
}

DataModel::List<const Structs::ScheduleTransitionStruct::Type> ScheduleStructWithOwnedMembers::GetTransitions() const
{
    return transitions;
}

DataModel::Nullable<bool> ScheduleStructWithOwnedMembers::GetBuiltIn() const
{
    return builtIn;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
