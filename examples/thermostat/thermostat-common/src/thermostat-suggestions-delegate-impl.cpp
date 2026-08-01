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

#include "../include/thermostat-delegate-impl.h"

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

CHIP_ERROR ThermostatDelegate::RemoveFromThermostatSuggestionsList(size_t indexToRemove)
{
    if (indexToRemove >= GetNumberOfThermostatSuggestions())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (size_t index = indexToRemove; index < static_cast<size_t>(mNextFreeIndexInThermostatSuggestionsList - 1); index++)
    {
        mThermostatSuggestions[index] = mThermostatSuggestions[index + 1];
    }
    if (indexToRemove == mIndexOfCurrentSuggestion)
    {
        CancelExpirationTimer();
        SetCurrentThermostatSuggestion(GetMaxThermostatSuggestions());
    }
    mNextFreeIndexInThermostatSuggestionsList--;
    return CHIP_NO_ERROR;
}

bool ThermostatDelegate::HaveSuggestionWithID(uint8_t uniqueIDToFind)
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

CHIP_ERROR ThermostatDelegate::GetUniqueID(uint8_t & uniqueID)
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

CHIP_ERROR ThermostatDelegate::StartExpirationTimer(Seconds32 timeout)
{
    ChipLogProgress(Zcl, "Starting timer to wait for %" PRIu32 "seconds for the current thermostat suggestion to expire",
                    timeout.count());
    mIsExpirationTimerRunning = true;
    return DeviceLayer::SystemLayer().StartTimer(std::chrono::duration_cast<Milliseconds32>(timeout), TimerExpiredCallback,
                                                 static_cast<void *>(this));
}

void ThermostatDelegate::TimerExpiredCallback(System::Layer * systemLayer, void * appState)
{
    auto ctx = static_cast<ThermostatDelegate *>(appState);
    if (ctx == nullptr)
    {
        ChipLogError(Zcl, "TimerExpiredCallback: Failed to ReEvaluateCurrentSuggestion since context is null");
        return;
    }
    TEMPORARY_RETURN_IGNORED ctx->ReEvaluateCurrentSuggestion();
}

void ThermostatDelegate::CancelExpirationTimer()
{
    if (mIsExpirationTimerRunning)
    {
        ChipLogProgress(Zcl, "Cancelling expiration timer for the current thermostat suggestion");
        DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, static_cast<void *>(this));
        mIsExpirationTimerRunning = false;
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

    Seconds32 currentMatterEpochTimestamp = Seconds32(currentMatterEpochTimestampInSeconds);

    SetCurrentThermostatSuggestion(GetThermostatSuggestionIndexWithEarliestEffectiveTime(currentMatterEpochTimestamp));

    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> nullableCurrentThermostatSuggestion;
    GetCurrentThermostatSuggestion(nullableCurrentThermostatSuggestion);

    if (!nullableCurrentThermostatSuggestion.IsNull())
    {
        ThermostatSuggestionStructWithOwnedMembers & currentThermostatSuggestion = nullableCurrentThermostatSuggestion.Value();

        TEMPORARY_RETURN_IGNORED SetActivePresetHandle(currentThermostatSuggestion.GetPresetHandle());
        TEMPORARY_RETURN_IGNORED SetThermostatSuggestionNotFollowingReason(DataModel::NullNullable);

        if (currentThermostatSuggestion.GetExpirationTime() > currentMatterEpochTimestamp)
        {
            TEMPORARY_RETURN_IGNORED StartExpirationTimer(currentThermostatSuggestion.GetExpirationTime() -
                                                          currentMatterEpochTimestamp);
        }
    }

    return CHIP_NO_ERROR;
}

size_t ThermostatDelegate::GetThermostatSuggestionIndexWithEarliestEffectiveTime(Seconds32 currentMatterEpochTimestamp)
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

        Seconds32 effectiveTime = suggestion.GetEffectiveTime();
        if (effectiveTime < minEffectiveTimeValue && effectiveTime <= currentMatterEpochTimestamp)
        {
            minEffectiveTimeValue           = effectiveTime;
            minEffectiveTimeSuggestionIndex = index;
        }
    }
    return minEffectiveTimeSuggestionIndex;
}
