/*
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

#pragma once

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

constexpr EndpointId kTestEndpointId   = 1;
constexpr FabricIndex kTestFabricIndex = 1;
constexpr NodeId kTestNodeId           = 0x11223344ULL;

class MockThermostatDelegate : public Thermostat::Delegate
{
public:
    FabricTable & GetFabricTable() const override
    {
        VerifyOrDie(mFabricTable != nullptr);
        return *mFabricTable;
    }
    void SetFabricTable(FabricTable * fabricTable) { mFabricTable = fabricTable; }

    DataModel::Nullable<temperature> GetLocalTemperature() const override { return mLocalTemperature; }
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
                                                            bool & changed) override
    {
        changed           = (mLocalTemperature != localTemperature);
        mLocalTemperature = localTemperature;
        return mSetLocalTemperatureStatus;
    }

    Protocols::InteractionModel::Status GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemp) const override
    {
        outdoorTemp = mOutdoorTemperature;
        return mGetOutdoorTemperatureStatus;
    }

    SystemModeEnum GetSystemMode() const override { return mSystemMode; }
    Protocols::InteractionModel::Status SetSystemMode(SystemModeEnum systemMode, bool & changed) override
    {
        changed     = (mSystemMode != systemMode);
        mSystemMode = systemMode;
        return mSetSystemModeStatus;
    }

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override { return mControlSequenceOfOperation; }
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation,
                                                                      bool & changed) override
    {
        changed                     = (mControlSequenceOfOperation != controlSequenceOfOperation);
        mControlSequenceOfOperation = controlSequenceOfOperation;
        return mSetControlSequenceOfOperationStatus;
    }

    Protocols::InteractionModel::Status GetRunningMode(ThermostatRunningModeEnum & runningMode) const override
    {
        runningMode = mRunningMode;
        return mGetRunningModeStatus;
    }
    Protocols::InteractionModel::Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed) override
    {
        changed      = (mRunningMode != runningMode);
        mRunningMode = runningMode;
        return mSetRunningModeStatus;
    }

    Protocols::InteractionModel::Status GetRunningState(BitMask<RelayStateBitmap> & runningState) const override
    {
        runningState = mRunningState;
        return mGetRunningStateStatus;
    }
    Protocols::InteractionModel::Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed) override
    {
        changed       = (mRunningState != runningState);
        mRunningState = runningState;
        return mSetRunningStateStatus;
    }

    int8_t GetLocalTemperatureCalibration() const override { return mCalibration; }
    Protocols::InteractionModel::Status SetLocalTemperatureCalibration(int8_t localTemperatureCalibration, bool & changed) override
    {
        changed      = (mCalibration != localTemperatureCalibration);
        mCalibration = localTemperatureCalibration;
        return mSetCalibrationStatus;
    }

    Protocols::InteractionModel::Status GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const override
    {
        remoteSensing = mRemoteSensing;
        return mGetRemoteSensingStatus;
    }
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing, bool & changed) override
    {
        changed        = (mRemoteSensing != remoteSensing);
        mRemoteSensing = remoteSensing;
        return mSetRemoteSensingStatus;
    }

    FabricTable * mFabricTable                                               = nullptr;
    DataModel::Nullable<temperature> mLocalTemperature                       = 2000;
    DataModel::Nullable<temperature> mOutdoorTemperature                     = 1500;
    SystemModeEnum mSystemMode                                               = SystemModeEnum::kOff;
    ControlSequenceOfOperationEnum mControlSequenceOfOperation               = ControlSequenceOfOperationEnum::kCoolingAndHeating;
    ThermostatRunningModeEnum mRunningMode                                   = ThermostatRunningModeEnum::kOff;
    BitMask<RelayStateBitmap> mRunningState                                  = 0;
    int8_t mCalibration                                                      = 0;
    BitMask<RemoteSensingBitmap> mRemoteSensing                              = 0;
    Protocols::InteractionModel::Status mSetLocalTemperatureStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetOutdoorTemperatureStatus         = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetSystemModeStatus                 = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetControlSequenceOfOperationStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetRunningModeStatus                = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetRunningModeStatus                = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetRunningStateStatus               = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetRunningStateStatus               = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetCalibrationStatus                = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetRemoteSensingStatus              = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetRemoteSensingStatus              = Protocols::InteractionModel::Status::Success;
};

class MockHeatingDelegate : public ThermostatHeatingSetpoints::Delegate
{
public:
    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const override
    {
        occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
        return mGetOccupiedHeatingStatus;
    }
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint, bool & changed) override
    {
        changed                  = (mOccupiedHeatingSetpoint != occupiedHeatingSetpoint);
        mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
        return mSetOccupiedHeatingStatus;
    }

    Protocols::InteractionModel::Status GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const override
    {
        unoccupiedHeatingSetpoint = mUnoccupiedHeatingSetpoint;
        return mGetUnoccupiedHeatingStatus;
    }
    Protocols::InteractionModel::Status SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint, bool & changed) override
    {
        changed                    = (mUnoccupiedHeatingSetpoint != unoccupiedHeatingSetpoint);
        mUnoccupiedHeatingSetpoint = unoccupiedHeatingSetpoint;
        return mSetUnoccupiedHeatingStatus;
    }

    Protocols::InteractionModel::Status GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const override
    {
        absMinHeatSetpointLimit = mAbsMinHeat;
        return mGetAbsMinHeatStatus;
    }
    Protocols::InteractionModel::Status GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const override
    {
        absMaxHeatSetpointLimit = mAbsMaxHeat;
        return mGetAbsMaxHeatStatus;
    }

    Protocols::InteractionModel::Status GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const override
    {
        minHeatSetpointLimit = mMinHeat;
        return mGetMinHeatStatus;
    }
    Protocols::InteractionModel::Status SetMinHeatSetpointLimit(temperature minHeatSetpointLimit, bool & changed) override
    {
        changed  = (mMinHeat != minHeatSetpointLimit);
        mMinHeat = minHeatSetpointLimit;
        return mSetMinHeatStatus;
    }

    Protocols::InteractionModel::Status GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const override
    {
        maxHeatSetpointLimit = mMaxHeat;
        return mGetMaxHeatStatus;
    }
    Protocols::InteractionModel::Status SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit, bool & changed) override
    {
        changed  = (mMaxHeat != maxHeatSetpointLimit);
        mMaxHeat = maxHeatSetpointLimit;
        return mSetMaxHeatStatus;
    }

    temperature mOccupiedHeatingSetpoint                            = 2000;
    temperature mUnoccupiedHeatingSetpoint                          = 1600;
    temperature mAbsMinHeat                                         = 700;
    temperature mAbsMaxHeat                                         = 3000;
    temperature mMinHeat                                            = 700;
    temperature mMaxHeat                                            = 3000;
    Protocols::InteractionModel::Status mGetOccupiedHeatingStatus   = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetOccupiedHeatingStatus   = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetUnoccupiedHeatingStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetUnoccupiedHeatingStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetAbsMinHeatStatus        = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetAbsMaxHeatStatus        = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetMinHeatStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetMinHeatStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetMaxHeatStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetMaxHeatStatus           = Protocols::InteractionModel::Status::Success;
};

class MockCoolingDelegate : public ThermostatCoolingSetpoints::Delegate
{
public:
    Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const override
    {
        occupiedCoolingSetpoint = mOccupiedCoolingSetpoint;
        return mGetOccupiedCoolingStatus;
    }
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint, bool & changed) override
    {
        changed                  = (mOccupiedCoolingSetpoint != occupiedCoolingSetpoint);
        mOccupiedCoolingSetpoint = occupiedCoolingSetpoint;
        return mSetOccupiedCoolingStatus;
    }

    Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const override
    {
        absMinCoolSetpointLimit = mAbsMinCool;
        return mGetAbsMinCoolStatus;
    }
    Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const override
    {
        absMaxCoolSetpointLimit = mAbsMaxCool;
        return mGetAbsMaxCoolStatus;
    }

    Protocols::InteractionModel::Status GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const override
    {
        minCoolSetpointLimit = mMinCool;
        return mGetMinCoolStatus;
    }
    Protocols::InteractionModel::Status SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed) override
    {
        changed  = (mMinCool != minCoolSetpointLimit);
        mMinCool = minCoolSetpointLimit;
        return mSetMinCoolStatus;
    }

    Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const override
    {
        maxCoolSetpointLimit = mMaxCool;
        return mGetMaxCoolStatus;
    }
    Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed) override
    {
        changed  = (mMaxCool != maxCoolSetpointLimit);
        mMaxCool = maxCoolSetpointLimit;
        return mSetMaxCoolStatus;
    }

    Protocols::InteractionModel::Status GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const override
    {
        unoccupiedCoolingSetpoint = mUnoccupiedCoolingSetpoint;
        return mGetUnoccupiedCoolingStatus;
    }
    Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint, bool & changed) override
    {
        changed                    = (mUnoccupiedCoolingSetpoint != unoccupiedCoolingSetpoint);
        mUnoccupiedCoolingSetpoint = unoccupiedCoolingSetpoint;
        return mSetUnoccupiedCoolingStatus;
    }

    temperature mOccupiedCoolingSetpoint                            = 2600;
    temperature mUnoccupiedCoolingSetpoint                          = 2800;
    temperature mAbsMinCool                                         = 1600;
    temperature mAbsMaxCool                                         = 3200;
    temperature mMinCool                                            = 1600;
    temperature mMaxCool                                            = 3200;
    Protocols::InteractionModel::Status mGetOccupiedCoolingStatus   = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetOccupiedCoolingStatus   = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetAbsMinCoolStatus        = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetAbsMaxCoolStatus        = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetMinCoolStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetMinCoolStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetMaxCoolStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetMaxCoolStatus           = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mGetUnoccupiedCoolingStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetUnoccupiedCoolingStatus = Protocols::InteractionModel::Status::Success;
};

class MockAutoDelegate : public ThermostatAutoSetpoints::Delegate
{
public:
    Protocols::InteractionModel::Status GetMinDeadband(temperature & minDeadband) const override
    {
        minDeadband = mDeadband;
        return mGetMinDeadbandStatus;
    }

    temperature mDeadband                                     = kDefaultDeadBand;
    Protocols::InteractionModel::Status mGetMinDeadbandStatus = Protocols::InteractionModel::Status::Success;
};

class MockHoldDelegate : public ThermostatHold::Delegate
{
public:
    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override { return mHold; }
    Protocols::InteractionModel::Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum hold, bool & changed) override
    {
        changed = (mHold != hold);
        mHold   = hold;
        return mSetHoldStatus;
    }

    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override { return mDuration; }
    Protocols::InteractionModel::Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration,
                                                                           bool & changed) override
    {
        changed   = (mDuration != duration);
        mDuration = duration;
        return mSetDurationStatus;
    }

    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override { return mExpiryTimestamp; }
    Protocols::InteractionModel::Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp,
                                                                       bool & changed) override
    {
        changed          = (mExpiryTimestamp != timestamp);
        mExpiryTimestamp = timestamp;
        return mSetExpiryStatus;
    }

    TemperatureSetpointHoldEnum mHold                      = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mDuration                = 60;
    DataModel::Nullable<uint32_t> mExpiryTimestamp         = 0;
    Protocols::InteractionModel::Status mSetHoldStatus     = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetDurationStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status mSetExpiryStatus   = Protocols::InteractionModel::Status::Success;
};

class MockOccupancyDelegate : public ThermostatOccupancy::Delegate
{
public:
    BitMask<OccupancyBitmap> GetOccupancy() const override { return mOccupancy; }
    Protocols::InteractionModel::Status SetOccupancy(BitMask<OccupancyBitmap> occupied, bool & changed) override
    {
        changed    = (mOccupancy != occupied);
        mOccupancy = occupied;
        return mSetOccupancyStatus;
    }

    BitMask<OccupancyBitmap> mOccupancy                     = OccupancyBitmap::kOccupied;
    Protocols::InteractionModel::Status mSetOccupancyStatus = Protocols::InteractionModel::Status::Success;
};

class MockPresetsDelegate : public ThermostatPresets::Delegate
{
public:
    CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType) override
    {
        if (index >= mPresetTypes.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        presetType = mPresetTypes[index];
        return CHIP_NO_ERROR;
    }

    uint8_t GetNumberOfPresets() override { return static_cast<uint8_t>(mMaxPresets); }

    CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        if (index >= mPresets.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        preset = mPresets[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        if (index >= mPendingPresets.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        preset = mPendingPresets[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle) override
    {
        if (mActivePresetHandle.IsNull())
        {
            activePresetHandle.SetNull();
            return CHIP_NO_ERROR;
        }
        if (activePresetHandle.IsNull() || activePresetHandle.Value().size() < mActivePresetHandle.Value().size())
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(activePresetHandle.Value().data(), mActivePresetHandle.Value().data(), mActivePresetHandle.Value().size());
        activePresetHandle.Value().reduce_size(mActivePresetHandle.Value().size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & activePresetHandle) override
    {
        if (activePresetHandle.IsNull())
        {
            mActivePresetHandle.SetNull();
        }
        else
        {
            if (activePresetHandle.Value().size() > sizeof(mActivePresetHandleBuffer))
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            memcpy(mActivePresetHandleBuffer, activePresetHandle.Value().data(), activePresetHandle.Value().size());
            mActivePresetHandle = DataModel::MakeNullable(ByteSpan(mActivePresetHandleBuffer, activePresetHandle.Value().size()));
        }
        return mSetActivePresetHandleError;
    }

    void InitializePendingPresets() override { mPendingPresets = mPresets; }

    void ClearPendingPresetList() override { mPendingPresets.clear(); }

    CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) override
    {
        mPendingPresets.push_back(preset);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitPendingPresets() override
    {
        ReturnErrorOnFailure(mCommitPendingPresetsError);
        mPresets = mPendingPresets;
        mPendingPresets.clear();
        return CHIP_NO_ERROR;
    }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        if (attributeId == Attributes::Presets::Id)
        {
            return mMaxAtomicWriteTimeout;
        }
        return std::nullopt;
    }

    size_t mMaxPresets = 10;
    std::vector<Structs::PresetTypeStruct::Type> mPresetTypes;
    std::vector<PresetStructWithOwnedMembers> mPresets;
    std::vector<PresetStructWithOwnedMembers> mPendingPresets;
    uint8_t mActivePresetHandleBuffer[kPresetHandleSize]                = { 0 };
    DataModel::Nullable<ByteSpan> mActivePresetHandle                   = DataModel::NullNullable;
    CHIP_ERROR mSetActivePresetHandleError                              = CHIP_NO_ERROR;
    CHIP_ERROR mCommitPendingPresetsError                               = CHIP_NO_ERROR;
    std::optional<System::Clock::Milliseconds16> mMaxAtomicWriteTimeout = System::Clock::Milliseconds16(10000);
};

class MockSuggestionsDelegate : public ThermostatSuggestions::Delegate
{
public:
    uint8_t GetMaxThermostatSuggestions() override { return mMaxSuggestions; }
    uint8_t GetNumberOfThermostatSuggestions() override { return static_cast<uint8_t>(mSuggestions.size()); }

    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index, ThermostatSuggestionStructWithOwnedMembers & suggestion) override
    {
        if (index >= mSuggestions.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        suggestion = mSuggestions[index];
        return CHIP_NO_ERROR;
    }

    void GetCurrentThermostatSuggestion(DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & current) override
    {
        current = mCurrentSuggestion;
    }

    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override
    {
        return mNotFollowingReason;
    }

    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override
    {
        if (mFailGetUniqueID)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        uniqueID = mNextUniqueID++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & suggestion) override
    {
        if (mFailAppend)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mSuggestions.push_back(suggestion);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t index) override
    {
        if (index >= mSuggestions.size())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        mSuggestions.erase(mSuggestions.begin() + static_cast<ptrdiff_t>(index));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReEvaluateCurrentSuggestion() override
    {
        mReEvaluateCalled = true;
        if (!mSuggestions.empty())
        {
            mCurrentSuggestion.SetNonNull(mSuggestions[0]);
        }
        else
        {
            mCurrentSuggestion.SetNull();
        }
        return CHIP_NO_ERROR;
    }

    uint8_t mMaxSuggestions = 5;
    uint8_t mNextUniqueID   = 1;
    bool mFailGetUniqueID   = false;
    bool mFailAppend        = false;
    bool mReEvaluateCalled  = false;
    std::vector<ThermostatSuggestionStructWithOwnedMembers> mSuggestions;
    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> mCurrentSuggestion    = DataModel::NullNullable;
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> mNotFollowingReason = DataModel::NullNullable;
};

class MockSchedulesDelegate : public ThermostatSchedules::Delegate
{
public:
    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override
    {
        if (index >= mScheduleTypes.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        scheduleType = mScheduleTypes[index];
        return CHIP_NO_ERROR;
    }

    uint8_t GetNumberOfSchedules() override { return static_cast<uint8_t>(mMaxSchedules); }
    uint8_t GetNumberOfScheduleTransitions() override { return mMaxScheduleTransitions; }
    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionsPerDay() override { return mMaxScheduleTransitionsPerDay; }

    CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        if (index >= mSchedules.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        schedule = mSchedules[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        if (index >= mPendingSchedules.size())
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        schedule = mPendingSchedules[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) override
    {
        if (mActiveScheduleHandle.IsNull())
        {
            activeScheduleHandle.SetNull();
            return CHIP_NO_ERROR;
        }
        if (activeScheduleHandle.IsNull() || activeScheduleHandle.Value().size() < mActiveScheduleHandle.Value().size())
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(activeScheduleHandle.Value().data(), mActiveScheduleHandle.Value().data(), mActiveScheduleHandle.Value().size());
        activeScheduleHandle.Value().reduce_size(mActiveScheduleHandle.Value().size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & activeScheduleHandle) override
    {
        if (activeScheduleHandle.IsNull())
        {
            mActiveScheduleHandle.SetNull();
        }
        else
        {
            if (activeScheduleHandle.Value().size() > sizeof(mActiveScheduleHandleBuffer))
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            memcpy(mActiveScheduleHandleBuffer, activeScheduleHandle.Value().data(), activeScheduleHandle.Value().size());
            mActiveScheduleHandle =
                DataModel::MakeNullable(ByteSpan(mActiveScheduleHandleBuffer, activeScheduleHandle.Value().size()));
        }
        return mSetActiveScheduleHandleError;
    }

    void InitializePendingSchedules() override { mPendingSchedules = mSchedules; }

    void ClearPendingScheduleList() override { mPendingSchedules.clear(); }

    CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) override
    {
        mPendingSchedules.push_back(schedule);
        if (schedule.GetScheduleHandle().IsNull())
        {
            const uint8_t handle[] = { mNextHandleValue++ };
            ReturnErrorOnFailure(mPendingSchedules.back().SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle))));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitPendingSchedules() override
    {
        ReturnErrorOnFailure(mCommitPendingSchedulesError);
        mSchedules = mPendingSchedules;
        mPendingSchedules.clear();
        return CHIP_NO_ERROR;
    }

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        if (attributeId == Attributes::Schedules::Id)
        {
            return mMaxAtomicWriteTimeout;
        }
        return std::nullopt;
    }

    // Seeds a single built-in schedule with the given handle byte and no transitions, so a Precommit's
    // "built-in schedules cannot be silently removed" rule has something to exercise.
    void SeedBuiltInSchedule(uint8_t handleByte)
    {
        ScheduleStructWithOwnedMembers schedule;
        schedule.SetSystemMode(SystemModeEnum::kHeat);
        const uint8_t handle[] = { handleByte };
        VerifyOrDie(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle))) == CHIP_NO_ERROR);
        VerifyOrDie(schedule.SetName(NullOptional) == CHIP_NO_ERROR);
        VerifyOrDie(schedule.SetPresetHandle(NullOptional) == CHIP_NO_ERROR);
        schedule.SetBuiltIn(DataModel::MakeNullable(true));
        mSchedules.push_back(schedule);
    }

    size_t mMaxSchedules                                       = 10;
    uint8_t mMaxScheduleTransitions                            = 10;
    DataModel::Nullable<uint8_t> mMaxScheduleTransitionsPerDay = DataModel::NullNullable;
    std::vector<Structs::ScheduleTypeStruct::Type> mScheduleTypes;
    std::vector<ScheduleStructWithOwnedMembers> mSchedules;
    std::vector<ScheduleStructWithOwnedMembers> mPendingSchedules;
    uint8_t mActiveScheduleHandleBuffer[kScheduleHandleSize]            = { 0 };
    DataModel::Nullable<ByteSpan> mActiveScheduleHandle                 = DataModel::NullNullable;
    uint8_t mNextHandleValue                                            = 100;
    CHIP_ERROR mSetActiveScheduleHandleError                            = CHIP_NO_ERROR;
    CHIP_ERROR mCommitPendingSchedulesError                             = CHIP_NO_ERROR;
    std::optional<System::Clock::Milliseconds16> mMaxAtomicWriteTimeout = System::Clock::Milliseconds16(9000);
};

inline bool HasAttribute(ServerClusterInterface & cluster, AttributeId attrId)
{
    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> builder;
    if (cluster.Attributes(cluster.GetPaths()[0], builder) != CHIP_NO_ERROR)
    {
        return false;
    }
    ReadOnlyBuffer<app::DataModel::AttributeEntry> entries = builder.TakeBuffer();
    return std::any_of(entries.begin(), entries.end(),
                       [attrId](const app::DataModel::AttributeEntry & entry) { return entry.attributeId == attrId; });
}

struct ThermostatTestFixture : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    ::chip::Testing::TestServerClusterContext mTestContext;
    ::chip::Testing::FabricTestFixture mFabricHelper{ &mTestContext.StorageDelegate() };
    TimerDelegateMock mMockTimerDelegate;
    OptionalAttributes mOptionalAttributes;

    MockThermostatDelegate mThermostatDelegate;
    MockHeatingDelegate mHeatingDelegate;
    MockCoolingDelegate mCoolingDelegate;
    MockAutoDelegate mAutoDelegate;
    MockHoldDelegate mHoldDelegate;
    MockOccupancyDelegate mOccupancyDelegate;
    MockPresetsDelegate mPresetsDelegate;
    MockSuggestionsDelegate mSuggestionsDelegate;
    MockSchedulesDelegate mSchedulesDelegate;

    void SetUp() override
    {
        mThermostatDelegate.SetFabricTable(&mFabricHelper.GetFabricTable());
        FabricIndex fabricIndex = kTestFabricIndex;
        ASSERT_EQ(mFabricHelper.SetUpTestFabric(fabricIndex), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        FabricIndex fabricIndex = kTestFabricIndex;
        EXPECT_EQ(mFabricHelper.TearDownTestFabric(fabricIndex), CHIP_NO_ERROR);
    }

    ThermostatClusterBase::Config MakeConfig() { return ThermostatClusterBase::Config(mOptionalAttributes, mMockTimerDelegate); }

    void SetupTesterSubject(::chip::Testing::ClusterTester & tester)
    {
        Access::SubjectDescriptor subjectDescriptor;
        subjectDescriptor.fabricIndex = kTestFabricIndex;
        subjectDescriptor.authMode    = Access::AuthMode::kCase;
        subjectDescriptor.subject     = kTestNodeId;
        tester.SetSubjectDescriptor(subjectDescriptor);
        tester.SetFabricIndex(kTestFabricIndex);
    }
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
