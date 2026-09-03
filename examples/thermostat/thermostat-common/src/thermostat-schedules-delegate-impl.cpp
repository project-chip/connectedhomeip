/*
 *
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

#include "../include/thermostat-schedules-delegate-impl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

ThermostatSchedulesDelegate::ThermostatSchedulesDelegate(EndpointId endpoint) : mEndpointId(endpoint)
{
    // Initialize Schedules state
    mMaxNumberOfSchedulesAllowedPerScheduleType = kMaxNumberOfSchedulesSupported;
    mNumberOfSchedules                          = kMaxNumberOfSchedulesSupported;
    mNextFreeIndexInSchedulesList               = 0;
    mNextFreeIndexInPendingSchedulesList        = 0;

    InitializeScheduleTypes();
    InitializeSchedules();

    memset(mActiveScheduleHandleData, 0, sizeof(mActiveScheduleHandleData));
    mActiveScheduleHandleDataSize = 0;
}

void ThermostatSchedulesDelegate::InitializeScheduleTypes()
{
    static_assert(MATTER_ARRAY_SIZE(mScheduleTypes) == 2);

    mScheduleTypes[0] = { .systemMode           = SystemModeEnum::kHeat,
                          .numberOfSchedules    = mMaxNumberOfSchedulesAllowedPerScheduleType,
                          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) };

    mScheduleTypes[1] = { .systemMode           = SystemModeEnum::kCool,
                          .numberOfSchedules    = mMaxNumberOfSchedulesAllowedPerScheduleType,
                          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) };
}

CHIP_ERROR ThermostatSchedulesDelegate::GetScheduleTypeAtIndex(size_t index, ScheduleTypeStruct::Type & scheduleType)
{
    if (index < MATTER_ARRAY_SIZE(mScheduleTypes))
    {
        scheduleType = mScheduleTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

void ThermostatSchedulesDelegate::InitializeSchedules()
{
    // Initialize a single built-in heating schedule with two transitions - one in the morning and one in the evening,
    // active every day of the week.
    mSchedules[0].SetSystemMode(SystemModeEnum::kHeat);

    const uint8_t handle[] = { static_cast<uint8_t>(SystemModeEnum::kHeat) };
    TEMPORARY_RETURN_IGNORED mSchedules[0].SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle)));
    TEMPORARY_RETURN_IGNORED mSchedules[0].SetName(NullOptional);
    TEMPORARY_RETURN_IGNORED mSchedules[0].SetPresetHandle(NullOptional);

    // Sunday through Saturday, i.e. every day of the week.
    chip::BitMask<ScheduleDayOfWeekBitmap> everyDay(0x7F);

    ScheduleTransitionStruct::Type transitions[2];
    transitions[0].dayOfWeek       = everyDay;
    transitions[0].transitionTime  = 360; // 06:00
    transitions[0].heatingSetpoint = MakeOptional(static_cast<int16_t>(2100));

    transitions[1].dayOfWeek       = everyDay;
    transitions[1].transitionTime  = 1320; // 22:00
    transitions[1].heatingSetpoint = MakeOptional(static_cast<int16_t>(1800));

    TEMPORARY_RETURN_IGNORED mSchedules[0].SetTransitions(DataModel::List<const ScheduleTransitionStruct::Type>(transitions));
    mSchedules[0].SetBuiltIn(DataModel::MakeNullable(true));

    mNextFreeIndexInSchedulesList = 1;
}

std::optional<System::Clock::Milliseconds16> ThermostatSchedulesDelegate::GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
{
    switch (attributeId)
    {
    case Attributes::Schedules::Id:
        return std::chrono::milliseconds(9000);
    default:
        return std::nullopt;
    }
}

uint8_t ThermostatSchedulesDelegate::GetNumberOfSchedules()
{
    return mNumberOfSchedules;
}

uint8_t ThermostatSchedulesDelegate::GetNumberOfScheduleTransitions()
{
    return kMaxNumberOfScheduleTransitionsSupported;
}

DataModel::Nullable<uint8_t> ThermostatSchedulesDelegate::GetNumberOfScheduleTransitionsPerDay()
{
    return DataModel::MakeNullable<uint8_t>(kMaxNumberOfScheduleTransitionsPerDaySupported);
}

CHIP_ERROR ThermostatSchedulesDelegate::GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule)
{
    if (index < mNextFreeIndexInSchedulesList)
    {
        schedule = mSchedules[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatSchedulesDelegate::GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle)
{
    if (!mActiveScheduleHandleIsNull)
    {
        VerifyOrReturnError(!activeScheduleHandle.IsNull() &&
                                activeScheduleHandle.Value().size() >= mActiveScheduleHandleDataSize,
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(mActiveScheduleHandleData, mActiveScheduleHandleDataSize),
                                                   activeScheduleHandle.Value()));
        activeScheduleHandle.Value().reduce_size(mActiveScheduleHandleDataSize);
    }
    else
    {
        activeScheduleHandle.SetNull();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatSchedulesDelegate::SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle)
{
    bool newIsNull = newActiveScheduleHandle.IsNull();
    ByteSpan oldHandle(mActiveScheduleHandleData, mActiveScheduleHandleDataSize);

    if (mActiveScheduleHandleIsNull == newIsNull && (newIsNull || oldHandle.data_equal(newActiveScheduleHandle.Value())))
    {
        return CHIP_NO_ERROR;
    }

    if (!newIsNull)
    {
        size_t newActiveScheduleHandleSize = newActiveScheduleHandle.Value().size();
        if (newActiveScheduleHandleSize > sizeof(mActiveScheduleHandleData))
        {
            ChipLogError(NotSpecified,
                         "Failed to set ActiveScheduleHandle. newActiveScheduleHandle size %u is larger than schedule handle "
                         "size %u",
                         static_cast<uint8_t>(newActiveScheduleHandleSize), static_cast<uint8_t>(kScheduleHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(mActiveScheduleHandleData, newActiveScheduleHandle.Value().data(), newActiveScheduleHandleSize);
        mActiveScheduleHandleDataSize = newActiveScheduleHandleSize;
        ChipLogDetail(NotSpecified, "Set ActiveScheduleHandle to ");
        ChipLogByteSpan(NotSpecified, newActiveScheduleHandle.Value());
    }
    else
    {
        memset(mActiveScheduleHandleData, 0, sizeof(mActiveScheduleHandleData));
        mActiveScheduleHandleDataSize = 0;
        ChipLogDetail(NotSpecified, "Clear ActiveScheduleHandle");
    }

    mActiveScheduleHandleIsNull = newIsNull;
    MatterReportingAttributeChangeCallback(mEndpointId, Thermostat::Id, Attributes::ActiveScheduleHandle::Id);

    return CHIP_NO_ERROR;
}

void ThermostatSchedulesDelegate::InitializePendingSchedules()
{
    mNextFreeIndexInPendingSchedulesList = 0;
    for (uint8_t indexInSchedules = 0; indexInSchedules < mNextFreeIndexInSchedulesList; indexInSchedules++)
    {
        mPendingSchedules[mNextFreeIndexInPendingSchedulesList] = mSchedules[indexInSchedules];
        mNextFreeIndexInPendingSchedulesList++;
    }
}

CHIP_ERROR ThermostatSchedulesDelegate::AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule)
{
    if (mNextFreeIndexInPendingSchedulesList < MATTER_ARRAY_SIZE(mPendingSchedules))
    {
        mPendingSchedules[mNextFreeIndexInPendingSchedulesList] = schedule;
        if (schedule.GetScheduleHandle().IsNull())
        {
            // Since we support only one schedule of each system mode, using the octet string containing the system mode
            // suffices as the unique schedule handle.
            const uint8_t handle[] = { static_cast<uint8_t>(schedule.GetSystemMode()) };
            TEMPORARY_RETURN_IGNORED mPendingSchedules[mNextFreeIndexInPendingSchedulesList].SetScheduleHandle(
                DataModel::MakeNullable(ByteSpan(handle)));
        }
        mNextFreeIndexInPendingSchedulesList++;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatSchedulesDelegate::GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule)
{
    if (index < mNextFreeIndexInPendingSchedulesList)
    {
        schedule = mPendingSchedules[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatSchedulesDelegate::CommitPendingSchedules()
{
    mNextFreeIndexInSchedulesList = 0;
    for (uint8_t indexInPendingSchedules = 0; indexInPendingSchedules < mNextFreeIndexInPendingSchedulesList;
         indexInPendingSchedules++)
    {
        const ScheduleStructWithOwnedMembers & pendingSchedule = mPendingSchedules[indexInPendingSchedules];
        mSchedules[mNextFreeIndexInSchedulesList]              = pendingSchedule;
        mNextFreeIndexInSchedulesList++;
    }
    return CHIP_NO_ERROR;
}

void ThermostatSchedulesDelegate::ClearPendingScheduleList()
{
    mNextFreeIndexInPendingSchedulesList = 0;
}
