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

#include "SensorScheduleTransitionStructWithOwnedMembers.h"
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

SensorScheduleTransitionStructWithOwnedMembers::SensorScheduleTransitionStructWithOwnedMembers(
    const Structs::SensorScheduleTransitionStruct::Type & other)
{
    *this = other;
}

SensorScheduleTransitionStructWithOwnedMembers::SensorScheduleTransitionStructWithOwnedMembers(
    const Structs::SensorScheduleTransitionStruct::DecodableType & other)
{
    *this = other;
}

SensorScheduleTransitionStructWithOwnedMembers &
SensorScheduleTransitionStructWithOwnedMembers::operator=(const Structs::SensorScheduleTransitionStruct::Type & other)
{
    SetDayOfWeek(other.dayOfWeek);
    SetTransitionTime(other.transitionTime);
    TEMPORARY_RETURN_IGNORED SetEnabledSensors(other.enabledSensors);
    return *this;
}

SensorScheduleTransitionStructWithOwnedMembers &
SensorScheduleTransitionStructWithOwnedMembers::operator=(const Structs::SensorScheduleTransitionStruct::DecodableType & other)
{
    SetDayOfWeek(other.dayOfWeek);
    SetTransitionTime(other.transitionTime);
    TEMPORARY_RETURN_IGNORED SetEnabledSensors(other.enabledSensors);
    return *this;
}

SensorScheduleTransitionStructWithOwnedMembers &
SensorScheduleTransitionStructWithOwnedMembers::operator=(const SensorScheduleTransitionStructWithOwnedMembers & other)
{
    if (this == &other)
    {
        return *this;
    }
    *this = static_cast<const Structs::SensorScheduleTransitionStruct::Type &>(other);
    return *this;
}

void SensorScheduleTransitionStructWithOwnedMembers::SetDayOfWeek(BitMask<ScheduleDayOfWeekBitmap> newDayOfWeek)
{
    dayOfWeek = newDayOfWeek;
}

void SensorScheduleTransitionStructWithOwnedMembers::SetTransitionTime(uint16_t newTransitionTime)
{
    transitionTime = newTransitionTime;
}

void SensorScheduleTransitionStructWithOwnedMembers::RefreshEnabledSensorsList()
{
    for (size_t i = 0; i < mNumEnabledSensors; i++)
    {
        mSensorHandles[i] = ByteSpan(mSensorHandlesData[i], mSensorHandles[i].size());
    }
    enabledSensors = DataModel::List<const ByteSpan>(mSensorHandles, mNumEnabledSensors);
}

CHIP_ERROR
SensorScheduleTransitionStructWithOwnedMembers::SetEnabledSensors(const DataModel::List<const ByteSpan> & newEnabledSensors)
{
    if (newEnabledSensors.size() > kMaxEnabledSensorsPerTransition)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }
    mNumEnabledSensors = 0;
    for (size_t i = 0; i < newEnabledSensors.size(); i++)
    {
        const auto & handle = newEnabledSensors[i];
        if (handle.size() > kMaxSensorHandleSize)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }
        memcpy(mSensorHandlesData[i], handle.data(), handle.size());
        mSensorHandles[i] = ByteSpan(mSensorHandlesData[i], handle.size());
        mNumEnabledSensors++;
    }
    RefreshEnabledSensorsList();
    return CHIP_NO_ERROR;
}

CHIP_ERROR
SensorScheduleTransitionStructWithOwnedMembers::SetEnabledSensors(const DataModel::DecodableList<ByteSpan> & newEnabledSensors)
{
    mNumEnabledSensors = 0;
    auto iter          = newEnabledSensors.begin();
    while (iter.Next())
    {
        if (mNumEnabledSensors >= kMaxEnabledSensorsPerTransition)
        {
            return CHIP_ERROR_INVALID_LIST_LENGTH;
        }
        const auto & handle = iter.GetValue();
        if (handle.size() > kMaxSensorHandleSize)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }
        memcpy(mSensorHandlesData[mNumEnabledSensors], handle.data(), handle.size());
        mSensorHandles[mNumEnabledSensors] = ByteSpan(mSensorHandlesData[mNumEnabledSensors], handle.size());
        mNumEnabledSensors++;
    }
    ReturnErrorOnFailure(iter.GetStatus());
    RefreshEnabledSensorsList();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SensorScheduleTransitionStructWithOwnedMembers::SetEnabledSensors(Span<const ByteSpan> newEnabledSensors)
{
    if (newEnabledSensors.size() > kMaxEnabledSensorsPerTransition)
    {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }
    mNumEnabledSensors = 0;
    for (size_t i = 0; i < newEnabledSensors.size(); i++)
    {
        const auto & handle = newEnabledSensors[i];
        if (handle.size() > kMaxSensorHandleSize)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }
        memcpy(mSensorHandlesData[i], handle.data(), handle.size());
        mSensorHandles[i] = ByteSpan(mSensorHandlesData[i], handle.size());
        mNumEnabledSensors++;
    }
    RefreshEnabledSensorsList();
    return CHIP_NO_ERROR;
}

BitMask<ScheduleDayOfWeekBitmap> SensorScheduleTransitionStructWithOwnedMembers::GetDayOfWeek() const
{
    return dayOfWeek;
}

uint16_t SensorScheduleTransitionStructWithOwnedMembers::GetTransitionTime() const
{
    return transitionTime;
}

DataModel::List<const ByteSpan> SensorScheduleTransitionStructWithOwnedMembers::GetEnabledSensors() const
{
    return enabledSensors;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
