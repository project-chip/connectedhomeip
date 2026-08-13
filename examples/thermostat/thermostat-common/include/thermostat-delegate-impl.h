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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/PresetStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatClusterOccupancy.h>
#include <app/clusters/thermostat-server/ThermostatClusterPresets.h>
#include <app/clusters/thermostat-server/ThermostatClusterSuggestions.h>
#include <app/clusters/thermostat-server/ThermostatClusterWithFeatures.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/clusters/thermostat-server/ThermostatSuggestionStructWithOwnedMembers.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr uint8_t kMaxNumberOfPresetTypes           = 6;
static constexpr uint8_t kMaxNumberOfScheduleTypes         = 2;
static constexpr uint8_t kMaxNumberOfPresetsOfEachType     = 1;
static constexpr uint8_t kMaxNumberOfSchedulesOfEachType   = 1;
static constexpr uint8_t kMaxNumberOfPresetsSupported      = kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType - 1;
static constexpr uint8_t kMaxNumberOfSchedulesSupported    = kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType - 1;
static constexpr uint8_t kMaxNumberOfThermostatSuggestions = 5;

class ThermostatDelegate : public Delegate,
                           public ThermostatPresets::Delegate,
                           public ThermostatSuggestions::Delegate,
                           public ThermostatOccupancy::Delegate
{
public:
    ThermostatDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr);
    ~ThermostatDelegate() override;

    SystemModeEnum GetSystemMode() const override;
    bool SetSystemMode(SystemModeEnum systemMode) override;
    ThermostatRunningModeEnum GetRunningMode() const override;
    bool SetRunningMode(ThermostatRunningModeEnum runningMode) override;
    BitMask<RelayStateBitmap> GetRunningState() const override;
    bool SetRunningState(BitMask<RelayStateBitmap> runningState) override;
    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override;
    bool SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation) override;
    DataModel::Nullable<temperature> GetLocalTemperature() const override;
    bool SetLocalTemperature(DataModel::Nullable<temperature> localTemperature) override;
    int8_t GetLocalTemperatureCalibration() const override;
    bool SetLocalTemperatureCalibration(int8_t localTemperatureCalibration) override;

    BitMask<RemoteSensingBitmap> GetRemoteSensing() const override;

    bool SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing) override;
    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override;
    bool SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold) override;
    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override;
    bool SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration) override;
    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override;
    bool SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp) override;

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints) override;
    Protocols::InteractionModel::Status SaveSetpoint(const Setpoint & oldSetpoint, const Setpoint & newSetpoint) override;

    // ThermostatOccupancy::Delegate methods
    BitMask<OccupancyBitmap> GetOccupancy() const override;
    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupancy) override;

    // ThermostatPresets::Delegate methods
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
    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override;

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
    AttributePersistenceProvider * mProvider = nullptr;

    ControlSequenceOfOperationEnum mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;

    SystemModeEnum mSystemMode                         = SystemModeEnum::kOff;
    ThermostatRunningModeEnum mRunningMode             = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState            = BitMask<RelayStateBitmap>(0);
    DataModel::Nullable<temperature> mLocalTemperature = DataModel::Nullable<int16_t>();
    int8_t mLocalTemperatureCalibration                = 0;

    BitMask<RemoteSensingBitmap> mRemoteSensing = BitMask<RemoteSensingBitmap>(0);

    TemperatureSetpointHoldEnum mTemperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp;

    // Occupancy state
    BitMask<OccupancyBitmap> mOccupancy{ OccupancyBitmap::kOccupied };

    // Presets state and private methods
    void InitializePresets();
    void InitializeScheduleTypes();

    uint8_t mNumberOfPresets;
    Structs::PresetTypeStruct::Type mPresetTypes[kMaxNumberOfPresetTypes];
    PresetStructWithOwnedMembers mPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];
    PresetStructWithOwnedMembers mPendingPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];

    uint8_t mNextFreeIndexInPendingPresetsList;
    uint8_t mNextFreeIndexInPresetsList;

    uint8_t mActivePresetHandleData[kPresetHandleSize];
    size_t mActivePresetHandleDataSize;
    bool mActivePresetHandleIsNull = true;

    uint8_t mMaxNumberOfSchedulesAllowedPerScheduleType;
    Structs::ScheduleTypeStruct::Type mScheduleTypes[kMaxNumberOfScheduleTypes];

    // Suggestions state and private methods
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

    size_t mIndexOfCurrentSuggestion;
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> mThermostatSuggestionNotFollowingReason;
    bool mIsExpirationTimerRunning = false;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
