/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <thermostat-delegate-impl.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

ThermostatDelegate ThermostatDelegate::sInstance;

ThermostatDelegate::ThermostatDelegate()
{
    mNumberOfPresets                   = kMaxNumberOfPresetsSupported;
    mNextFreeIndexInPresetsList        = 0;
    mNextFreeIndexInPendingPresetsList = 0;

    InitializePresets();

    memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
    mActivePresetHandleDataSize = 0;

    mNumberOfSchedules                   = kMaxNumberOfSchedulesSupported;
    mNumberOfScheduleTransitions         = kMaxNumberOfScheduleTransitionsSupported;
    mNumberOfScheduleTransitionPerDay    = DataModel::MakeNullable(kMaxNumberOfScheduleTransitionPerDaySupported);
    mNextFreeIndexInSchedulesList        = 0;
    mNextFreeIndexInPendingSchedulesList = 0;

    for (int i = 0; i < kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType; i++)
    {
        mSchedules[i].SetTransitionData(mScheduleTransitionData[i], kMaxNumberOfScheduleTransitionsSupported);
        mPendingSchedules[i].SetTransitionData(mPendingScheduleTransitionData[i], kMaxNumberOfScheduleTransitionsSupported);
    }

    InitializeSchedules();

    memset(mActiveScheduleHandleData, 0, sizeof(mActiveScheduleHandleData));
    mActiveScheduleHandleDataSize = 0;
}

std::optional<System::Clock::Milliseconds16> ThermostatDelegate::GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
{
    switch (attributeId)
    {
    case Attributes::Presets::Id:
        // If the client expects to edit the presets, then we'll give it 3 seconds to do so
        return std::chrono::milliseconds(3000);
    case Attributes::Schedules::Id:
        // If the client expects to edit the schedules, then we'll give it 9 seconds to do so
        return std::chrono::milliseconds(9000);
    default:
        return std::nullopt;
    }
}

/**
 * Thermostat Delegate codes for Presets feature
 */

CHIP_ERROR ThermostatDelegate::GetUniquePresetHandle(DataModel::Nullable<chip::ByteSpan> & presetHandle, uint8_t seed)
{
    // TODO: generate unique handle in octstr max 16 bytes
    static uint8_t handle[kPresetHandleSize];
    size_t handleSize;

    handle[0]  = seed;
    handleSize = 1;

    presetHandle = DataModel::MakeNullable(ByteSpan(handle, handleSize));

    return CHIP_NO_ERROR;
}

void ThermostatDelegate::InitializePresets()
{
    // Initialize the presets with 2 built in presets - occupied and unoccupied.
    PresetScenarioEnum presetScenarioEnumArray[2] = { PresetScenarioEnum::kOccupied, PresetScenarioEnum::kUnoccupied };
    static_assert(MATTER_ARRAY_SIZE(presetScenarioEnumArray) <= MATTER_ARRAY_SIZE(mPresets));

    uint8_t index = 0;
    for (PresetScenarioEnum presetScenario : presetScenarioEnumArray)
    {
        DataModel::Nullable<chip::ByteSpan> presetHandle;

        GetUniquePresetHandle(presetHandle, static_cast<uint8_t>(presetScenario));

        mPresets[index].SetPresetScenario(presetScenario);
        mPresets[index].SetPresetHandle(presetHandle);
        mPresets[index].SetName(NullOptional);
        int16_t coolingSetpointValue = static_cast<int16_t>(2500 + (index * 100));
        mPresets[index].SetCoolingSetpoint(MakeOptional(coolingSetpointValue));

        int16_t heatingSetpointValue = static_cast<int16_t>(2100 - (index * 100));
        mPresets[index].SetHeatingSetpoint(MakeOptional(heatingSetpointValue));
        mPresets[index].SetBuiltIn(DataModel::MakeNullable(true));
        index++;
    }

    // Set the value of the next free index in the presets list.
    mNextFreeIndexInPresetsList = index;
}

CHIP_ERROR ThermostatDelegate::GetPresetTypeAtIndex(size_t index, PresetTypeStruct::Type & presetType)
{
    static PresetTypeStruct::Type presetTypes[] = {
        { .presetScenario     = PresetScenarioEnum::kOccupied,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kAutomatic) },
        { .presetScenario     = PresetScenarioEnum::kUnoccupied,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kAutomatic) },
        { .presetScenario     = PresetScenarioEnum::kSleep,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kSupportsNames) },
        { .presetScenario     = PresetScenarioEnum::kWake,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kSupportsNames) },
        { .presetScenario     = PresetScenarioEnum::kVacation,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kSupportsNames) },
        { .presetScenario     = PresetScenarioEnum::kUserDefined,
          .numberOfPresets    = kMaxNumberOfPresetsOfEachType,
          .presetTypeFeatures = to_underlying(PresetTypeFeaturesBitmap::kSupportsNames) },
    };
    if (index < MATTER_ARRAY_SIZE(presetTypes))
    {
        presetType = presetTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

uint8_t ThermostatDelegate::GetNumberOfPresets()
{
    return mNumberOfPresets;
}

CHIP_ERROR ThermostatDelegate::GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)
{
    if (index < mNextFreeIndexInPresetsList)
    {
        preset = mPresets[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle)
{
    if (mActivePresetHandleDataSize != 0)
    {
        ReturnErrorOnFailure(
            CopySpanToMutableSpan(ByteSpan(mActivePresetHandleData, mActivePresetHandleDataSize), activePresetHandle.Value()));
        activePresetHandle.Value().reduce_size(mActivePresetHandleDataSize);
    }
    else
    {
        activePresetHandle.SetNull();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatDelegate::SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle)
{
    if (!newActivePresetHandle.IsNull())
    {
        size_t newActivePresetHandleSize = newActivePresetHandle.Value().size();
        if (newActivePresetHandleSize > sizeof(mActivePresetHandleData))
        {
            ChipLogError(NotSpecified,
                         "Failed to set ActivePresetHandle. newActivePresetHandle size %u is larger than preset handle size %u",
                         static_cast<uint8_t>(newActivePresetHandleSize), static_cast<uint8_t>(kPresetHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(mActivePresetHandleData, newActivePresetHandle.Value().data(), newActivePresetHandleSize);
        mActivePresetHandleDataSize = newActivePresetHandleSize;
        ChipLogDetail(NotSpecified, "Set ActivePresetHandle to ");
        ChipLogByteSpan(NotSpecified, newActivePresetHandle.Value());
    }
    else
    {
        memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
        mActivePresetHandleDataSize = 0;
        ChipLogDetail(NotSpecified, "Clear ActivePresetHandle");
    }
    return CHIP_NO_ERROR;
}

void ThermostatDelegate::InitializePendingPresets()
{
    mNextFreeIndexInPendingPresetsList = 0;
    for (uint8_t indexInPresets = 0; indexInPresets < mNextFreeIndexInPresetsList; indexInPresets++)
    {
        mPendingPresets[mNextFreeIndexInPendingPresetsList] = mPresets[indexInPresets];
        mNextFreeIndexInPendingPresetsList++;
    }
}

CHIP_ERROR ThermostatDelegate::AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset)
{
    if (mNextFreeIndexInPendingPresetsList < MATTER_ARRAY_SIZE(mPendingPresets))
    {
        mPendingPresets[mNextFreeIndexInPendingPresetsList] = preset;
        if (preset.GetPresetHandle().IsNull())
        {
            // TODO: #34556 Since we support only one preset of each type, using the octet string containing the preset scenario
            // suffices as the unique preset handle. Need to fix this to actually provide unique handles once multiple presets of
            // each type are supported.
            CHIP_ERROR err;

            DataModel::Nullable<chip::ByteSpan> presetHandle;

            err = GetUniquePresetHandle(presetHandle, static_cast<uint8_t>(preset.GetPresetScenario()));
            if (err != CHIP_NO_ERROR)
            {
                return err;
            }

            mPendingPresets[mNextFreeIndexInPendingPresetsList].SetPresetHandle(presetHandle);
        }
        mNextFreeIndexInPendingPresetsList++;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatDelegate::GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)
{
    if (index < mNextFreeIndexInPendingPresetsList)
    {
        preset = mPendingPresets[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::CommitPendingPresets()
{
    mNextFreeIndexInPresetsList = 0;
    for (uint8_t indexInPendingPresets = 0; indexInPendingPresets < mNextFreeIndexInPendingPresetsList; indexInPendingPresets++)
    {
        const PresetStructWithOwnedMembers & pendingPreset = mPendingPresets[indexInPendingPresets];
        mPresets[mNextFreeIndexInPresetsList]              = pendingPreset;
        mNextFreeIndexInPresetsList++;
    }
    return CHIP_NO_ERROR;
}

void ThermostatDelegate::ClearPendingPresetList()
{
    mNextFreeIndexInPendingPresetsList = 0;
}

CHIP_ERROR ThermostatDelegate::GetUniqueScheduleHandle(DataModel::Nullable<chip::ByteSpan> & scheduleHandle, uint8_t seed)
{
    // TODO: generate unique handle in octstr max 16 bytes
    static uint8_t handle[kScheduleHandleSize];
    size_t handleSize;

    handle[0]  = seed;
    handleSize = 1;

    scheduleHandle = DataModel::MakeNullable(ByteSpan(handle, handleSize));

    return CHIP_NO_ERROR;
}

void ThermostatDelegate::InitializeSchedules()
{
    // Initialize the system mode with 2 built in system modes - cool and heat.
    SystemModeEnum systemModeEnumArray[2] = { SystemModeEnum::kCool, SystemModeEnum::kHeat };

    // Initialize the transition list
    Structs::ScheduleTransitionStruct::Type transitionMonday          = { .dayOfWeek       = ScheduleDayOfWeekBitmap::kMonday,
                                                                          .transitionTime  = 1000,
                                                                          .presetHandle    = NullOptional,
                                                                          .systemMode      = Optional<SystemModeEnum>(SystemModeEnum::kCool),
                                                                          .coolingSetpoint = Optional<int16_t>(2100),
                                                                          .heatingSetpoint = Optional<int16_t>(2500) };
    Structs::ScheduleTransitionStruct::Type transitionSunday          = { .dayOfWeek       = ScheduleDayOfWeekBitmap::kSunday,
                                                                          .transitionTime  = 1000,
                                                                          .presetHandle    = NullOptional,
                                                                          .systemMode      = Optional<SystemModeEnum>(SystemModeEnum::kHeat),
                                                                          .coolingSetpoint = Optional<int16_t>(2100),
                                                                          .heatingSetpoint = Optional<int16_t>(2500) };
    Structs::ScheduleTransitionStruct::Type scheduleTransitions[2][2] = {
        { transitionMonday, transitionSunday },
        { transitionSunday, transitionMonday },
    };
    static_assert(MATTER_ARRAY_SIZE(systemModeEnumArray) <= MATTER_ARRAY_SIZE(mSchedules));

    uint8_t index = 0;
    for (SystemModeEnum systemMode : systemModeEnumArray)
    {
        DataModel::Nullable<chip::ByteSpan> scheduleHandle;

        // Set the schedule handle to the system mode value as a unique id.
        GetUniqueScheduleHandle(scheduleHandle, static_cast<uint8_t>(systemMode));

        mSchedules[index].SetSystemMode(systemMode);
        mSchedules[index].SetScheduleHandle(scheduleHandle);
        mSchedules[index].SetName(NullOptional);
        mSchedules[index].SetPresetHandle(scheduleTransitions[index][0].presetHandle); // use presetHandle from the first transition
        mSchedules[index].SetTransitions(
            DataModel::List<const Structs::ScheduleTransitionStruct::Type>(scheduleTransitions[index]));
        mSchedules[index].SetBuiltIn(DataModel::MakeNullable(true));
        index++;
    }

    // Set the value of the next free index in the schedules list.
    mNextFreeIndexInSchedulesList = index;
}

CHIP_ERROR ThermostatDelegate::GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType)
{
    static ScheduleTypeStruct::Type scheduleTypes[] = {
        { .systemMode           = SystemModeEnum::kOff,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsOff) },
        { .systemMode           = SystemModeEnum::kAuto,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsPresets) },
        { .systemMode           = SystemModeEnum::kCool,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kHeat,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kEmergencyHeat,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kPrecooling,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kFanOnly,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kDry,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
        { .systemMode           = SystemModeEnum::kSleep,
          .numberOfSchedules    = kMaxNumberOfSchedulesOfEachType,
          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) },
    };
    if (index < MATTER_ARRAY_SIZE(scheduleTypes))
    {
        scheduleType = scheduleTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

uint8_t ThermostatDelegate::GetNumberOfSchedules()
{
    return mNumberOfSchedules;
}

uint8_t ThermostatDelegate::GetNumberOfScheduleTransitions()
{
    return mNumberOfScheduleTransitions;
}

DataModel::Nullable<uint8_t> ThermostatDelegate::GetNumberOfScheduleTransitionPerDay()
{
    return mNumberOfScheduleTransitionPerDay;
}

CHIP_ERROR ThermostatDelegate::GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule)
{
    if (index < mNextFreeIndexInSchedulesList)
    {
        schedule = mSchedules[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle)
{
    if (mActiveScheduleHandleDataSize != 0)
    {
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

CHIP_ERROR ThermostatDelegate::SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle)
{
    if (!newActiveScheduleHandle.IsNull())
    {
        size_t newActiveScheduleHandleSize = newActiveScheduleHandle.Value().size();
        if (newActiveScheduleHandleSize > sizeof(mActiveScheduleHandleData))
        {
            ChipLogError(
                NotSpecified,
                "Failed to set ActiveScheduleHandle. newActiveScheduleHandle size %u is larger than schedule handle size %u",
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
    return CHIP_NO_ERROR;
}

void ThermostatDelegate::InitializePendingSchedules()
{
    mNextFreeIndexInPendingSchedulesList = 0;
    for (uint8_t indexInSchedules = 0; indexInSchedules < mNextFreeIndexInSchedulesList; indexInSchedules++)
    {
        mPendingSchedules[mNextFreeIndexInPendingSchedulesList] = mSchedules[indexInSchedules];
        mNextFreeIndexInPendingSchedulesList++;
    }
}

CHIP_ERROR ThermostatDelegate::AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule)
{
    if (mNextFreeIndexInPendingSchedulesList < MATTER_ARRAY_SIZE(mPendingSchedules))
    {
        mPendingSchedules[mNextFreeIndexInPendingSchedulesList] = schedule;
        if (schedule.GetScheduleHandle().IsNull())
        {
            // TODO: #34556 Since we support only one schedule of each type, using the octet string containing the preset scenario
            // suffices as the unique schedule handle. Need to fix this to actually provide unique handles once multiple presets of
            // each type are supported.
            CHIP_ERROR err;
            DataModel::Nullable<chip::ByteSpan> scheduleHandle;

            err = GetUniqueScheduleHandle(scheduleHandle, static_cast<uint8_t>(schedule.GetSystemMode()));
            if (err != CHIP_NO_ERROR)
            {
                return err;
            }

            mPendingSchedules[mNextFreeIndexInPendingSchedulesList].SetScheduleHandle(scheduleHandle);
        }
        mNextFreeIndexInPendingSchedulesList++;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatDelegate::GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule)
{
    if (index < mNextFreeIndexInPendingSchedulesList)
    {
        schedule = mPendingSchedules[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::CommitPendingSchedules()
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

void ThermostatDelegate::ClearPendingScheduleList()
{
    mNextFreeIndexInPendingSchedulesList = 0;
}
