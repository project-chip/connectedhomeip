/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#pragma once

#include "thermostat-presets-delegate-impl.h"

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/PresetStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatClusterPresets.h>
#include <app/clusters/thermostat-server/ThermostatClusterSuggestions.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/clusters/thermostat-server/ThermostatSuggestionStructWithOwnedMembers.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr uint8_t kMaxNumberOfThermostatSuggestions = 5;

/*
 * A simple implementation of ThermostatSuggestions::Delegate.
 * It reports and persists the state of the thermostat's suggestions attributes.
 */
class ThermostatSuggestionsDelegate : public ThermostatSuggestions::Delegate
{
public:
    ThermostatSuggestionsDelegate(EndpointId endpoint, ThermostatPresetsDelegate & presetsDelegate);
    ~ThermostatSuggestionsDelegate() override;

    // ThermostatSuggestions::Delegate methods
    uint8_t GetMaxThermostatSuggestions() override;
    uint8_t GetNumberOfThermostatSuggestions() override;
    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                              ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) override;
    void GetCurrentThermostatSuggestion(
        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) override;
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override;
    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion) override;
    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t indexToRemove) override;
    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override;
    CHIP_ERROR ReEvaluateCurrentSuggestion() override;

private:
    EndpointId mEndpointId;
    ThermostatPresetsDelegate & mPresetsDelegate;

    /**
     * @brief return the index of the thermostat suggestion in the ThermostatSuggestions attribute with the earliest EffectiveTime
     * field. If there are no entries or an error occurs, returns the value in the MaxThermostatSuggestions attribute as an
     * invalid index.
     *
     */
    size_t GetThermostatSuggestionIndexWithEarliestEffectiveTime(System::Clock::Seconds32 currentMatterEpochTimestamp);
    CHIP_ERROR StartExpirationTimer(System::Clock::Seconds32 timeout);
    static void TimerExpiredCallback(System::Layer * systemLayer, void * appState);
    void CancelExpirationTimer();
    CHIP_ERROR SetThermostatSuggestionNotFollowingReason(
        const DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> & thermostatSuggestionNotFollowingReason);
    void SetCurrentThermostatSuggestion(size_t index);
    bool HaveSuggestionWithID(uint8_t uniqueIDToFind);

    uint8_t mMaxThermostatSuggestions;
    ThermostatSuggestionStructWithOwnedMembers mThermostatSuggestions[kMaxNumberOfThermostatSuggestions];
    uint8_t mNextFreeIndexInThermostatSuggestionsList;
    uint8_t mUniqueID;

    // TODO: #39949 - This information should be stored in the cluster instance.
    size_t mIndexOfCurrentSuggestion;
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> mThermostatSuggestionNotFollowingReason;

    bool mIsExpirationTimerRunning = false;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
