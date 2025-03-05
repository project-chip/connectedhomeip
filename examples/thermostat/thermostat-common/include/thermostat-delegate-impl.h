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

// TODO: #34556 Support multiple presets of each type.
// We will support only one preset of each preset type.
static constexpr uint8_t kMaxNumberOfPresetsOfEachType = 1;

// For testing the use case where number of presets added exceeds the number of presets supported, we will have the value of
// kMaxNumberOfPresetsSupported < kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType
static constexpr uint8_t kMaxNumberOfPresetsSupported = kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType - 1;

static constexpr uint8_t kMaxNumberOfScheduleTypes = 9;

// TODO:  Support multiple schedules of each type.
// We will support only one schedule of each schedule type.
static constexpr uint8_t kMaxNumberOfSchedulesOfEachType = 1;

static constexpr uint8_t kMaxNumberOfSchedulesSupported = kMaxNumberOfScheduleTypes * kMaxNumberOfSchedulesOfEachType;

static constexpr uint8_t kMaxNumberOfScheduleTransitionsSupported = 5;

static constexpr uint8_t kMaxNumberOfScheduleTransitionPerDaySupported = 3;

class ThermostatDelegate : public Delegate
{
public:
    static inline ThermostatDelegate & GetInstance() { return sInstance; }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;

    /**
     * Thermostat Delegate members for Presets feature
     */

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

    /**
     * Thermostat Delegate Code for Enhanced Scheduling feature
     */

    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & ScheduleType) override;

    uint8_t GetNumberOfSchedules() override;

    uint8_t GetNumberOfScheduleTransitions() override;

    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionPerDay() override;

    CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override;

    CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) override;

    CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle) override;

    void InitializePendingSchedules() override;

    CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) override;

    CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override;

    CHIP_ERROR CommitPendingSchedules() override;

    void ClearPendingScheduleList() override;

private:
    static ThermostatDelegate sInstance;

    ThermostatDelegate();
    ~ThermostatDelegate() = default;

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

    CHIP_ERROR GetUniquePresetHandle(DataModel::Nullable<chip::ByteSpan> & presetHandle, uint8_t input);

    uint8_t mNumberOfPresets;

    Structs::PresetTypeStruct::Type mPresetTypes[kMaxNumberOfPresetTypes];
    PresetStructWithOwnedMembers mPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];
    PresetStructWithOwnedMembers mPendingPresets[kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType];

    uint8_t mNextFreeIndexInPendingPresetsList;
    uint8_t mNextFreeIndexInPresetsList;

    uint8_t mActivePresetHandleData[kPresetHandleSize];
    size_t mActivePresetHandleDataSize;

    /**
     * @brief Initializes the schedule types array with all schedule types corresponding to SystemModeEnum.
     */
    void InitializeScheduleTypes();

    /**
     * @brief Initializes the schedules array with some sample schedules for testing.
     */
    void InitializeScheduleTransitions();
    /**
     * @brief Initializes the schedules array with some sample schedules for testing.
     */
    void InitializeSchedules();

    CHIP_ERROR GetUniqueScheduleHandle(DataModel::Nullable<chip::ByteSpan> & scheduleHandle, uint8_t input);

    uint8_t mNumberOfSchedules;
    uint8_t mNumberOfScheduleTransitions;
    DataModel::Nullable<uint8_t> mNumberOfScheduleTransitionPerDay;

    Structs::ScheduleTypeStruct::Type mScheduleTypes[kMaxNumberOfScheduleTypes];
    Structs::ScheduleTransitionStruct::Type mScheduleTransitionData[kMaxNumberOfSchedulesSupported]
                                                                   [kMaxNumberOfScheduleTransitionsSupported];
    ScheduleStructWithOwnedMembers mSchedules[kMaxNumberOfSchedulesSupported];
    Structs::ScheduleTransitionStruct::Type mPendingScheduleTransitionData[kMaxNumberOfSchedulesSupported]
                                                                          [kMaxNumberOfScheduleTransitionsSupported];
    ScheduleStructWithOwnedMembers mPendingSchedules[kMaxNumberOfSchedulesSupported];

    uint8_t mNextFreeIndexInPendingSchedulesList;
    uint8_t mNextFreeIndexInSchedulesList;

    uint8_t mActiveScheduleHandleData[kScheduleHandleSize];
    size_t mActiveScheduleHandleDataSize;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
