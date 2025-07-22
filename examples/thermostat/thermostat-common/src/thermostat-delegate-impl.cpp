/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace System::Clock;

ThermostatDelegate ThermostatDelegate::sInstance;

ThermostatDelegate::ThermostatDelegate()
{
    mNumberOfPresets                          = kMaxNumberOfPresetsSupported;
    mNextFreeIndexInPresetsList               = 0;
    mNextFreeIndexInPendingPresetsList        = 0;
    mMaxThermostatSuggestions                 = kMaxNumberOfThermostatSuggestions;
    mIndexOfCurrentSuggestion                 = mMaxThermostatSuggestions;
    mNextFreeIndexInThermostatSuggestionsList = 0;

    // Start the unique ID from 0 and it increases montonically.
    mUniqueID = 0;

    InitializePresets();

    memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
    mActivePresetHandleDataSize = 0;
}

ThermostatDelegate::~ThermostatDelegate()
{
    CancelExpirationTimer();
}

void ThermostatDelegate::InitializePresets()
{
    // Initialize the presets with 2 built in presets - occupied and unoccupied.
    PresetScenarioEnum presetScenarioEnumArray[2] = { PresetScenarioEnum::kOccupied, PresetScenarioEnum::kUnoccupied };
    static_assert(MATTER_ARRAY_SIZE(presetScenarioEnumArray) <= MATTER_ARRAY_SIZE(mPresets));

    uint8_t index = 0;
    for (PresetScenarioEnum presetScenario : presetScenarioEnumArray)
    {
        mPresets[index].SetPresetScenario(presetScenario);

        // Set the preset handle to the preset scenario value as a unique id.
        const uint8_t handle[] = { static_cast<uint8_t>(presetScenario) };
        mPresets[index].SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle)));
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
            const uint8_t handle[] = { static_cast<uint8_t>(preset.GetPresetScenario()) };
            mPendingPresets[mNextFreeIndexInPendingPresetsList].SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle)));
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

uint8_t ThermostatDelegate::GetMaxThermostatSuggestions()
{
    return mMaxThermostatSuggestions;
}

uint8_t ThermostatDelegate::GetNumberOfThermostatSuggestions()
{
    return mNextFreeIndexInThermostatSuggestionsList;
}

CHIP_ERROR ThermostatDelegate::GetThermostatSuggestionAtIndex(size_t index,
                                                              ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion)
{
    if (index < mNextFreeIndexInThermostatSuggestionsList)
    {
        thermostatSuggestion = mThermostatSuggestions[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

void ThermostatDelegate::GetCurrentThermostatSuggestion(
    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion)
{
    if (mIndexOfCurrentSuggestion < mNextFreeIndexInThermostatSuggestionsList)
    {
        currentThermostatSuggestion.SetNonNull(mThermostatSuggestions[mIndexOfCurrentSuggestion]);
    }
    else
    {
        currentThermostatSuggestion.SetNull();
    }
}

DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> ThermostatDelegate::GetThermostatSuggestionNotFollowingReason()
{
    return mThermostatSuggestionNotFollowingReason;
}

CHIP_ERROR ThermostatDelegate::SetThermostatSuggestionNotFollowingReason(
    const DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> & thermostatSuggestionNotFollowingReason)
{
    bool hasChanged = (mThermostatSuggestionNotFollowingReason != thermostatSuggestionNotFollowingReason);

    if (hasChanged)
    {
        mThermostatSuggestionNotFollowingReason = thermostatSuggestionNotFollowingReason;
        MatterReportingAttributeChangeCallback(mEndpointId, Thermostat::Id, Attributes::ThermostatSuggestionNotFollowingReason::Id);
    }

    return CHIP_NO_ERROR;
}

void ThermostatDelegate::SetCurrentThermostatSuggestion(size_t index)
{
    // The MaxThermostatSuggestions attribute value is used as an index to set the current thermostat suggestion to null. Hence the
    // <= check below.
    if (index <= GetMaxThermostatSuggestions())
    {
        bool hasChanged = (mIndexOfCurrentSuggestion != index);
        if (hasChanged)
        {
            mIndexOfCurrentSuggestion = index;
            MatterReportingAttributeChangeCallback(mEndpointId, Thermostat::Id, Attributes::CurrentThermostatSuggestion::Id);
        }
    }
}

CHIP_ERROR
ThermostatDelegate::AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion)
{
    if (mNextFreeIndexInThermostatSuggestionsList < MATTER_ARRAY_SIZE(mThermostatSuggestions))
    {
        mThermostatSuggestions[mNextFreeIndexInThermostatSuggestionsList++] = thermostatSuggestion;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::RemoveFromThermostatSuggestionsList(uint8_t uniqueID)
{
    size_t uniqueIDMatchedIndex = mNextFreeIndexInThermostatSuggestionsList;
    for (size_t index = 0; index < static_cast<size_t>(mNextFreeIndexInThermostatSuggestionsList); index++)
    {
        if (mThermostatSuggestions[index].GetUniqueID() == uniqueID)
        {
            uniqueIDMatchedIndex = index;
            break;
        }
    }
    if (uniqueIDMatchedIndex == mNextFreeIndexInThermostatSuggestionsList)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    // Shift elements to the left to fill the gap.
    for (size_t index = uniqueIDMatchedIndex; index < static_cast<size_t>(mNextFreeIndexInThermostatSuggestionsList - 1); index++)
    {
        mThermostatSuggestions[index] = mThermostatSuggestions[index + 1];
    }
    if (uniqueIDMatchedIndex == mIndexOfCurrentSuggestion)
    {
        CancelExpirationTimer();
        SetCurrentThermostatSuggestion(GetMaxThermostatSuggestions());
    }
    mNextFreeIndexInThermostatSuggestionsList--;
    return CHIP_NO_ERROR;
}

uint8_t ThermostatDelegate::GetUniqueID()
{
    uint8_t maxUniqueId = 0;

    for (size_t index = 0; index < static_cast<size_t>(mNextFreeIndexInThermostatSuggestionsList); index++)
    {
        uint8_t uniqueID = mThermostatSuggestions[index].GetUniqueID();
        if (uniqueID > maxUniqueId)
        {
            maxUniqueId = uniqueID;
        }
    }

    // TODO: Fix this to return an id that does not truly exist in the list. e.g. If the list has uniqueID 0 and 255 this will
    // incorrectly return 0 which exists in the list and is not unique.
    return maxUniqueId + 1;
}
/**
 * @brief Starts a timer to wait for the expiration of the current thermostat suggestion.
 *
 * @param[in] timeoutInSecs The timeout in seconds.
 */
CHIP_ERROR ThermostatDelegate::StartExpirationTimer(uint32_t timeoutInSecs)
{
    ChipLogProgress(Zcl, "Starting timer to wait for %" PRIu32 "seconds for the current thermostat suggestion to expire",
                    timeoutInSecs);
    IsExpirationTimerRunning = true;
    return DeviceLayer::SystemLayer().StartTimer(std::chrono::duration_cast<Milliseconds32>(Seconds32(timeoutInSecs)),
                                                 TimerExpiredCallback, static_cast<void *>(this));
}

CHIP_ERROR ThermostatDelegate::RemoveExpiredSuggestions(uint32_t currentMatterEpochTimestampInSeconds)
{
    for (int i = static_cast<int>(GetNumberOfThermostatSuggestions()) - 1; i >= 0; i--)
    {
        ThermostatSuggestionStructWithOwnedMembers suggestion;
        CHIP_ERROR err = GetThermostatSuggestionAtIndex(static_cast<size_t>(i), suggestion);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        if (suggestion.GetExpirationTime() < currentMatterEpochTimestampInSeconds)
        {
            err = RemoveFromThermostatSuggestionsList(suggestion.GetUniqueID());
            VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        }
    }
    return CHIP_NO_ERROR;
}

void ThermostatDelegate::TimerExpiredCallback(System::Layer * systemLayer, void * appState)
{
    auto ctx = static_cast<ThermostatDelegate *>(appState);
    if (ctx == nullptr)
    {
        ChipLogError(Zcl, "TimerExpiredCallback: Failed to ReEvaluateCurrentSuggestion since context is null");
        return;
    }
    ctx->ReEvaluateCurrentSuggestion();
}

void ThermostatDelegate::CancelExpirationTimer()
{
    if (IsExpirationTimerRunning)
    {
        ChipLogProgress(Zcl, "Cancelling expiration timer for the current thermostat suggestion");
        DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, static_cast<void *>(this));
        IsExpirationTimerRunning = false;
    }
}

CHIP_ERROR ThermostatDelegate::ReEvaluateCurrentSuggestion()
{
    CancelExpirationTimer();

    uint32_t currentMatterEpochTimestampInSeconds = 0;
    CHIP_ERROR err                                = System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get the current time stamp with error: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    // Remove all expired suggestions.
    RemoveExpiredSuggestions(currentMatterEpochTimestampInSeconds);

    // For the reference thermostat app, we will always choose a suggestion with the earliest effective time.
    mIndexOfCurrentSuggestion = GetThermostatSuggestionIndexWithEarliestEffectiveTime(currentMatterEpochTimestampInSeconds);
    SetCurrentThermostatSuggestion(mIndexOfCurrentSuggestion);

    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> nullableCurrentThermostatSuggestion;
    GetCurrentThermostatSuggestion(nullableCurrentThermostatSuggestion);

    if (!nullableCurrentThermostatSuggestion.IsNull())
    {

        ThermostatSuggestionStructWithOwnedMembers & currentThermostatSuggestion = nullableCurrentThermostatSuggestion.Value();

        // TODO: Check if a hold is set and set the ThermostatSuggestionNotFollowingReason to OngoingHold and do not update
        // ActivePresetHandle. Otherwise set the ActivePresetHandle to the preset handle in the suggestion and set
        // ThermostatSuggestionNotFollowingReason to null.
        SetActivePresetHandle(currentThermostatSuggestion.GetPresetHandle());
        MatterReportingAttributeChangeCallback(mEndpointId, Thermostat::Id, Attributes::ActivePresetHandle::Id);
        SetThermostatSuggestionNotFollowingReason(DataModel::NullNullable);

        // Start a timer from the timestamp in currentMatterEpochTimestampInSeconds to the timestamp in the expiration time.
        if (currentThermostatSuggestion.GetExpirationTime() > currentMatterEpochTimestampInSeconds)
        {
            StartExpirationTimer(currentThermostatSuggestion.GetExpirationTime() - currentMatterEpochTimestampInSeconds);
        }
    }

    return CHIP_NO_ERROR;
}

size_t ThermostatDelegate::GetThermostatSuggestionIndexWithEarliestEffectiveTime(uint32_t currentMatterEpochTimestampInSeconds)
{
    uint8_t maxThermostatSuggestions = GetMaxThermostatSuggestions();
    VerifyOrReturnValue(GetNumberOfThermostatSuggestions() > 0, maxThermostatSuggestions);

    uint32_t minEffectiveTimeValue         = UINT32_MAX;
    size_t minEffectiveTimeSuggestionIndex = maxThermostatSuggestions;

    for (size_t index = 0; index < static_cast<size_t>(GetNumberOfThermostatSuggestions()); index++)
    {
        ThermostatSuggestionStructWithOwnedMembers suggestion;
        CHIP_ERROR err = GetThermostatSuggestionAtIndex(index, suggestion);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, maxThermostatSuggestions);

        // Check for the least effective time that is less than the current timestamp.
        uint32_t effectiveTime = suggestion.GetEffectiveTime();
        if (effectiveTime < minEffectiveTimeValue && effectiveTime <= currentMatterEpochTimestampInSeconds)
        {
            minEffectiveTimeValue           = effectiveTime;
            minEffectiveTimeSuggestionIndex = index;
        }
    }
    return minEffectiveTimeSuggestionIndex;
}
