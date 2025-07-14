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

#pragma once

#include <app/clusters/thermostat-server/thermostat-delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
 * The ThermostatDelegate class serves as the instance delegate for storing Presets related information and providing it to the
 * Thermostat server code. It also manages the presets attribute and provides methods to write to presets, edit presets, maintain a
 * pending presets list and either commit the presets when requested or discard the changes. It also provides APIs to get and set
 * the attribute values.
 *
 */

static constexpr uint8_t kMaxNumberOfPresetTypes = 6;

static constexpr uint8_t kMaxNumberOfThermostatSuggestions = 5;

// TODO: #34556 Support multiple presets of each type.
// We will support only one preset of each preset type.
static constexpr uint8_t kMaxNumberOfPresetsOfEachType = 1;

// For testing the use case where number of presets added exceeds the number of presets supported, we will have the value of
// kMaxNumberOfPresetsSupported < kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType
static constexpr uint8_t kMaxNumberOfPresetsSupported = kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType - 1;

class ThermostatDelegate : public Delegate
{
public:
    static inline ThermostatDelegate & GetInstance() { return sInstance; }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;

    CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType) override;

    uint8_t GetNumberOfPresets() override;

    CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override;

    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle) override;

    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle) override;

    void InitializePendingPresets() override;

    CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) override;

    CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override;

    CHIP_ERROR CommitPendingPresets() override;

    void ClearPendingPresetList() override;

    uint8_t GetMaxThermostatSuggestions() override;

    uint8_t GetNumberOfThermostatSuggestions() override;

    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                              ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) override;

    void GetCurrentThermostatSuggestion(
        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) override;

    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override;

    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion) override;

    CHIP_ERROR RemoveFromThermostatSuggestionsList(uint8_t uniqueID) override;

    uint8_t GetUniqueID() override;

    CHIP_ERROR ReEvaluateCurrentSuggestion() override;

private:
    static ThermostatDelegate sInstance;

    ThermostatDelegate();
    ~ThermostatDelegate();

    ThermostatDelegate(const ThermostatDelegate &)             = delete;
    ThermostatDelegate & operator=(const ThermostatDelegate &) = delete;

    /**
     * @brief Initializes the preset types array with all preset types corresponding to PresetScenarioEnum.
     */
    void InitializePresetTypes();

    /**
     * @brief Initializes the presets array with some sample presets for testing.
     */
    void InitializePresets();

    /**
     * @brief return the index of the thermostat suggestion in the ThermostatSuggestions attribute with the earliest EffectiveTime
     * field. If there are no entries or an error occurs, returns the value in the MaxThermostatSuggestions attribute as an
     * invalid index.
     *
     */
    size_t GetThermostatSuggestionIndexWithEarliestEffectiveTime(uint32_t currentMatterEpochTimestampInSeconds);

    CHIP_ERROR StartExpirationTimer(uint32_t timeoutInSecs);

    static void TimerExpiredCallback(System::Layer * systemLayer, void * appState);

    void CancelExpirationTimer();

    CHIP_ERROR RemoveExpiredSuggestions(uint32_t currentMatterEpochTimestampInSeconds);

    CHIP_ERROR SetThermostatSuggestionNotFollowingReason(
        const DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> & thermostatSuggestionNotFollowingReason);

    void SetCurrentThermostatSuggestion(size_t index);

    uint8_t mNumberOfPresets;

    Structs::PresetTypeStruct::Type mPresetTypes[kMaxNumberOfPresetTypes];
    PresetStructWithOwnedMembers mPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];
    PresetStructWithOwnedMembers mPendingPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];

    uint8_t mNextFreeIndexInPendingPresetsList;
    uint8_t mNextFreeIndexInPresetsList;

    uint8_t mActivePresetHandleData[kPresetHandleSize];
    size_t mActivePresetHandleDataSize;

    uint8_t mMaxThermostatSuggestions;
    ThermostatSuggestionStructWithOwnedMembers mThermostatSuggestions[kMaxNumberOfThermostatSuggestions];
    uint8_t mNextFreeIndexInThermostatSuggestionsList;
    uint8_t mUniqueID;

    // TODO: #39949 - This information should be stored in the cluster instance.
    size_t mIndexOfCurrentSuggestion;
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> mThermostatSuggestionNotFollowingReason;

    bool IsExpirationTimerRunning = false;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
