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

#include "../include/thermostat-suggestions-delegate-impl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace System::Clock;

// Suggestions Implementation

ThermostatSuggestionsDelegate::ThermostatSuggestionsDelegate(EndpointId endpoint, ThermostatPresetsDelegate & presetsDelegate) :
    mEndpointId(endpoint), mPresetsDelegate(presetsDelegate)
{
    // Initialize Suggestions state
    mMaxThermostatSuggestions                 = kMaxNumberOfThermostatSuggestions;
    mIndexOfCurrentSuggestion                 = mMaxThermostatSuggestions;
    mNextFreeIndexInThermostatSuggestionsList = 0;
    mUniqueID                                 = 0;
}

ThermostatSuggestionsDelegate::~ThermostatSuggestionsDelegate()
{
    CancelExpirationTimer();
}

uint8_t ThermostatSuggestionsDelegate::GetMaxThermostatSuggestions()
{
    return mMaxThermostatSuggestions;
}

uint8_t ThermostatSuggestionsDelegate::GetNumberOfThermostatSuggestions()
{
    return mNextFreeIndexInThermostatSuggestionsList;
}

CHIP_ERROR
ThermostatSuggestionsDelegate::GetThermostatSuggestionAtIndex(size_t index,
                                                              ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion)
{
    if (index < mNextFreeIndexInThermostatSuggestionsList)
    {
        thermostatSuggestion = mThermostatSuggestions[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

void ThermostatSuggestionsDelegate::GetCurrentThermostatSuggestion(
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

DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap>
ThermostatSuggestionsDelegate::GetThermostatSuggestionNotFollowingReason()
{
    return mThermostatSuggestionNotFollowingReason;
}

CHIP_ERROR ThermostatSuggestionsDelegate::SetThermostatSuggestionNotFollowingReason(
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

void ThermostatSuggestionsDelegate::SetCurrentThermostatSuggestion(size_t index)
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
ThermostatSuggestionsDelegate::AppendToThermostatSuggestionsList(
    const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion)
{
    if (mNextFreeIndexInThermostatSuggestionsList < MATTER_ARRAY_SIZE(mThermostatSuggestions))
    {
        mThermostatSuggestions[mNextFreeIndexInThermostatSuggestionsList++] = thermostatSuggestion;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatSuggestionsDelegate::RemoveFromThermostatSuggestionsList(size_t indexToRemove)
{
    if (indexToRemove >= GetNumberOfThermostatSuggestions())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Shift elements to the left to fill the gap.
    for (size_t index = indexToRemove; index < static_cast<size_t>(mNextFreeIndexInThermostatSuggestionsList - 1); index++)
    {
        mThermostatSuggestions[index] = mThermostatSuggestions[index + 1];
    }
    if (indexToRemove == mIndexOfCurrentSuggestion)
    {
        CancelExpirationTimer();
        SetCurrentThermostatSuggestion(GetMaxThermostatSuggestions());
    }
    else if (mIndexOfCurrentSuggestion < mNextFreeIndexInThermostatSuggestionsList && indexToRemove < mIndexOfCurrentSuggestion)
    {
        SetCurrentThermostatSuggestion(mIndexOfCurrentSuggestion - 1);
    }
    mNextFreeIndexInThermostatSuggestionsList--;
    return CHIP_NO_ERROR;
}

bool ThermostatSuggestionsDelegate::HaveSuggestionWithID(uint8_t uniqueIDToFind)
{
    for (auto & suggestion : Span(mThermostatSuggestions, mNextFreeIndexInThermostatSuggestionsList))
    {
        if (uniqueIDToFind == suggestion.GetUniqueID())
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR ThermostatSuggestionsDelegate::GetUniqueID(uint8_t & uniqueID)
{
    uint8_t maxUniqueId = 0;

    for (auto & suggestion : Span(mThermostatSuggestions, mNextFreeIndexInThermostatSuggestionsList))
    {
        uint8_t existingUniqueID = suggestion.GetUniqueID();
        if (existingUniqueID > maxUniqueId)
        {
            maxUniqueId = existingUniqueID;
        }
    }

    uniqueID = maxUniqueId + 1;

    // If overflow occurs, check for next available uniqueID.
    if (uniqueID == 0)
    {
        while (HaveSuggestionWithID(uniqueID))
        {
            uniqueID++;
            if (uniqueID == UINT8_MAX)
            {
                return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
            }
        };
    }
    return CHIP_NO_ERROR;
}

/**
 * @brief Starts a timer to wait for the expiration of the current thermostat suggestion.
 *
 * @param[in] timeout The timeout in seconds.
 */
CHIP_ERROR ThermostatSuggestionsDelegate::StartExpirationTimer(Seconds32 timeout)
{
    ChipLogProgress(Zcl, "Starting timer to wait for %" PRIu32 "seconds for the current thermostat suggestion to expire",
                    timeout.count());
    mIsExpirationTimerRunning = true;
    return DeviceLayer::SystemLayer().StartTimer(std::chrono::duration_cast<Milliseconds32>(timeout), TimerExpiredCallback,
                                                 static_cast<void *>(this));
}

void ThermostatSuggestionsDelegate::TimerExpiredCallback(System::Layer * systemLayer, void * appState)
{
    auto ctx = static_cast<ThermostatSuggestionsDelegate *>(appState);
    if (ctx == nullptr)
    {
        ChipLogError(Zcl, "TimerExpiredCallback: Failed to ReEvaluateCurrentSuggestion since context is null");
        return;
    }
    TEMPORARY_RETURN_IGNORED ctx->ReEvaluateCurrentSuggestion();
}

void ThermostatSuggestionsDelegate::CancelExpirationTimer()
{
    if (mIsExpirationTimerRunning)
    {
        ChipLogProgress(Zcl, "Cancelling expiration timer for the current thermostat suggestion");
        DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, static_cast<void *>(this));
        mIsExpirationTimerRunning = false;
    }
}

CHIP_ERROR ThermostatSuggestionsDelegate::ReEvaluateCurrentSuggestion()
{
    CancelExpirationTimer();

    uint32_t currentMatterEpochTimestampInSeconds = 0;
    CHIP_ERROR err                                = System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get the current time stamp with error: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    Seconds32 currentMatterEpochTimestamp = Seconds32(currentMatterEpochTimestampInSeconds);

    // For the reference thermostat app, we will always choose a suggestion with the earliest effective time.
    SetCurrentThermostatSuggestion(GetThermostatSuggestionIndexWithEarliestEffectiveTime(currentMatterEpochTimestamp));

    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> nullableCurrentThermostatSuggestion;
    GetCurrentThermostatSuggestion(nullableCurrentThermostatSuggestion);

    if (!nullableCurrentThermostatSuggestion.IsNull())
    {
        ThermostatSuggestionStructWithOwnedMembers & currentThermostatSuggestion = nullableCurrentThermostatSuggestion.Value();

        // TODO: Check if a hold is set and set the ThermostatSuggestionNotFollowingReason to OngoingHold and do not update
        // ActivePresetHandle. Otherwise set the ActivePresetHandle to the preset handle in the suggestion and set
        // ThermostatSuggestionNotFollowingReason to null.
        TEMPORARY_RETURN_IGNORED mPresetsDelegate.SetActivePresetHandle(currentThermostatSuggestion.GetPresetHandle());
        TEMPORARY_RETURN_IGNORED SetThermostatSuggestionNotFollowingReason(DataModel::NullNullable);

        // Start a timer from the timestamp in currentMatterEpochTimestamp to the timestamp in the expiration time.
        if (currentThermostatSuggestion.GetExpirationTime() > currentMatterEpochTimestamp)
        {
            TEMPORARY_RETURN_IGNORED StartExpirationTimer(currentThermostatSuggestion.GetExpirationTime() -
                                                          currentMatterEpochTimestamp);
        }
    }

    return CHIP_NO_ERROR;
}

size_t ThermostatSuggestionsDelegate::GetThermostatSuggestionIndexWithEarliestEffectiveTime(Seconds32 currentMatterEpochTimestamp)
{
    uint8_t maxThermostatSuggestions = GetMaxThermostatSuggestions();
    VerifyOrReturnValue(GetNumberOfThermostatSuggestions() > 0, maxThermostatSuggestions);

    Seconds32 minEffectiveTimeValue        = Seconds32(UINT32_MAX);
    size_t minEffectiveTimeSuggestionIndex = maxThermostatSuggestions;

    for (size_t index = 0; index < static_cast<size_t>(GetNumberOfThermostatSuggestions()); index++)
    {
        ThermostatSuggestionStructWithOwnedMembers suggestion;
        CHIP_ERROR err = GetThermostatSuggestionAtIndex(index, suggestion);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, maxThermostatSuggestions);

        // Check for the least effective time that is less than the current timestamp.
        Seconds32 effectiveTime = suggestion.GetEffectiveTime();
        if (effectiveTime < minEffectiveTimeValue && effectiveTime <= currentMatterEpochTimestamp)
        {
            minEffectiveTimeValue           = effectiveTime;
            minEffectiveTimeSuggestionIndex = index;
        }
    }
    return minEffectiveTimeSuggestionIndex;
}
