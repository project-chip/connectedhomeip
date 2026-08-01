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

ScheduleStructWithOwnedMembers::ScheduleStructWithOwnedMembers(const ScheduleStruct::DecodableType & other)
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
    err = SetPresetHandle(other.presetHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule preset handle with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    err = SetTransitions(other.transitions);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule transitions with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    SetBuiltIn(other.builtIn);
    return *this;
}

ScheduleStructWithOwnedMembers & ScheduleStructWithOwnedMembers::operator=(const ScheduleStruct::DecodableType & other)
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
    err = SetPresetHandle(other.presetHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Schedule preset handle with err %" CHIP_ERROR_FORMAT, err.Format());
    }
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
            ChipLogError(Zcl, "Failed to set Schedule handle. New schedule handle size (%u) > allowed schedule handle size (%u)",
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
        ReturnErrorOnFailure(CopyCharSpanToMutableCharSpan(newName.Value(), targetSpan));
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
        if (newPresetHandleSize > kScheduleHandleSize)
        {
            ChipLogError(Zcl, "Failed to set Schedule preset handle. New preset handle size (%u) > allowed size (%u)",
                         static_cast<unsigned>(newPresetHandleSize), static_cast<unsigned>(kScheduleHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableByteSpan targetSpan(presetHandleData);
        ReturnErrorOnFailure(CopySpanToMutableSpan(newPresetHandle.Value(), targetSpan));
        presetHandle.SetValue(targetSpan);
    }
    else
    {
        presetHandle.ClearValue();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetTransitionAtIndex(size_t index, const ScheduleTransitionStruct::Type & transition)
{
    if (index >= kScheduleTransitionsMax)
    {
        ChipLogError(Zcl, "Failed to set Schedule transition at index %u: exceeds max of %u", static_cast<unsigned>(index),
                     static_cast<unsigned>(kScheduleTransitionsMax));
        return CHIP_ERROR_NO_MEMORY;
    }

    auto & storedTransition          = transitionsData[index];
    storedTransition.dayOfWeek       = transition.dayOfWeek;
    storedTransition.transitionTime  = transition.transitionTime;
    storedTransition.systemMode      = transition.systemMode;
    storedTransition.coolingSetpoint = transition.coolingSetpoint;
    storedTransition.heatingSetpoint = transition.heatingSetpoint;

    if (transition.presetHandle.HasValue())
    {
        size_t newPresetHandleSize = transition.presetHandle.Value().size();
        if (newPresetHandleSize > kScheduleHandleSize)
        {
            ChipLogError(Zcl, "Failed to set Schedule transition preset handle. Size (%u) > allowed size (%u)",
                         static_cast<unsigned>(newPresetHandleSize), static_cast<unsigned>(kScheduleHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableByteSpan targetSpan(transitionPresetHandleData[index]);
        ReturnErrorOnFailure(CopySpanToMutableSpan(transition.presetHandle.Value(), targetSpan));
        storedTransition.presetHandle.SetValue(targetSpan);
    }
    else
    {
        storedTransition.presetHandle.ClearValue();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ScheduleStructWithOwnedMembers::SetTransitions(const DataModel::List<const ScheduleTransitionStruct::Type> & newTransitions)
{
    if (newTransitions.size() > kScheduleTransitionsMax)
    {
        ChipLogError(Zcl, "Failed to set Schedule transitions. Count (%u) > allowed count (%u)",
                     static_cast<unsigned>(newTransitions.size()), static_cast<unsigned>(kScheduleTransitionsMax));
        return CHIP_ERROR_NO_MEMORY;
    }

    size_t index = 0;
    for (const auto & transition : newTransitions)
    {
        ReturnErrorOnFailure(SetTransitionAtIndex(index, transition));
        index++;
    }
    transitionsCount = index;
    transitions       = DataModel::List<const ScheduleTransitionStruct::Type>(transitionsData, transitionsCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ScheduleStructWithOwnedMembers::SetTransitions(
    const DataModel::DecodableList<ScheduleTransitionStruct::DecodableType> & newTransitions)
{
    size_t index = 0;
    auto iter    = newTransitions.begin();
    while (iter.Next())
    {
        ReturnErrorOnFailure(SetTransitionAtIndex(index, iter.GetValue()));
        index++;
    }
    ReturnErrorOnFailure(iter.GetStatus());
    transitionsCount = index;
    transitions       = DataModel::List<const ScheduleTransitionStruct::Type>(transitionsData, transitionsCount);
    return CHIP_NO_ERROR;
}

void ScheduleStructWithOwnedMembers::SetBuiltIn(DataModel::Nullable<bool> newBuiltIn)
{
    builtIn = newBuiltIn;
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

DataModel::List<const ScheduleTransitionStruct::Type> ScheduleStructWithOwnedMembers::GetTransitions() const
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
