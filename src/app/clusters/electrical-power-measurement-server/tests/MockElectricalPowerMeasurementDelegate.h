/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

/**
 * @brief Mock delegate implementation for testing ElectricalPowerMeasurementCluster
 *
 * This delegate provides a simple implementation with configurable behavior
 * suitable for unit testing. It stores all attribute values in memory and
 * uses fixed-size arrays for list attributes.
 */
class MockElectricalPowerMeasurementDelegate : public Delegate
{
public:
    MockElectricalPowerMeasurementDelegate() = default;

    // Mandatory attribute getters
    PowerModeEnum GetPowerMode() override { return mPowerMode; }
    uint8_t GetNumberOfMeasurementTypes() override { return mNumberOfMeasurementTypes; }

    // Accuracy list management
    CHIP_ERROR StartAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetAccuracyByIndex(uint8_t index, Structs::MeasurementAccuracyStruct::Type & accuracy) override
    {
        if (index >= mAccuracyCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        accuracy = mAccuracyList[index];
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EndAccuracyRead() override { return CHIP_NO_ERROR; }

    // Ranges list management
    CHIP_ERROR StartRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetRangeByIndex(uint8_t index, Structs::MeasurementRangeStruct::Type & range) override
    {
        if (index >= mRangesCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        range = mRangesList[index];
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EndRangesRead() override { return CHIP_NO_ERROR; }

    // HarmonicCurrents list management
    CHIP_ERROR StartHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t index, Structs::HarmonicMeasurementStruct::Type & current) override
    {
        if (index >= mHarmonicCurrentsCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        current = mHarmonicCurrentsList[index];
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EndHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }

    // HarmonicPhases list management
    CHIP_ERROR StartHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t index, Structs::HarmonicMeasurementStruct::Type & phase) override
    {
        if (index >= mHarmonicPhasesCount)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        phase = mHarmonicPhasesList[index];
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR EndHarmonicPhasesRead() override { return CHIP_NO_ERROR; }

    // Optional attribute getters
    DataModel::Nullable<int64_t> GetVoltage() override { return mVoltage; }
    DataModel::Nullable<int64_t> GetActiveCurrent() override { return mActiveCurrent; }
    DataModel::Nullable<int64_t> GetReactiveCurrent() override { return mReactiveCurrent; }
    DataModel::Nullable<int64_t> GetApparentCurrent() override { return mApparentCurrent; }
    DataModel::Nullable<int64_t> GetActivePower() override { return mActivePower; }
    DataModel::Nullable<int64_t> GetReactivePower() override { return mReactivePower; }
    DataModel::Nullable<int64_t> GetApparentPower() override { return mApparentPower; }
    DataModel::Nullable<int64_t> GetRMSVoltage() override { return mRMSVoltage; }
    DataModel::Nullable<int64_t> GetRMSCurrent() override { return mRMSCurrent; }
    DataModel::Nullable<int64_t> GetRMSPower() override { return mRMSPower; }
    DataModel::Nullable<int64_t> GetFrequency() override { return mFrequency; }
    DataModel::Nullable<int64_t> GetPowerFactor() override { return mPowerFactor; }
    DataModel::Nullable<int64_t> GetNeutralCurrent() override { return mNeutralCurrent; }

    // Setters for test configuration
    void SetPowerMode(PowerModeEnum mode) { mPowerMode = mode; }
    void SetNumberOfMeasurementTypes(uint8_t count) { mNumberOfMeasurementTypes = count; }

    void AddAccuracy(const Structs::MeasurementAccuracyStruct::Type & accuracy)
    {
        if (mAccuracyCount < kMaxListSize)
        {
            mAccuracyList[mAccuracyCount++] = accuracy;
        }
    }

    void AddRange(const Structs::MeasurementRangeStruct::Type & range)
    {
        if (mRangesCount < kMaxListSize)
        {
            mRangesList[mRangesCount++] = range;
        }
    }

    void AddHarmonicCurrent(const Structs::HarmonicMeasurementStruct::Type & current)
    {
        if (mHarmonicCurrentsCount < kMaxListSize)
        {
            mHarmonicCurrentsList[mHarmonicCurrentsCount++] = current;
        }
    }

    void AddHarmonicPhase(const Structs::HarmonicMeasurementStruct::Type & phase)
    {
        if (mHarmonicPhasesCount < kMaxListSize)
        {
            mHarmonicPhasesList[mHarmonicPhasesCount++] = phase;
        }
    }

    void SetVoltage(DataModel::Nullable<int64_t> value) { mVoltage = value; }
    void SetActiveCurrent(DataModel::Nullable<int64_t> value) { mActiveCurrent = value; }
    void SetReactiveCurrent(DataModel::Nullable<int64_t> value) { mReactiveCurrent = value; }
    void SetApparentCurrent(DataModel::Nullable<int64_t> value) { mApparentCurrent = value; }
    void SetActivePower(DataModel::Nullable<int64_t> value) { mActivePower = value; }
    void SetReactivePower(DataModel::Nullable<int64_t> value) { mReactivePower = value; }
    void SetApparentPower(DataModel::Nullable<int64_t> value) { mApparentPower = value; }
    void SetRMSVoltage(DataModel::Nullable<int64_t> value) { mRMSVoltage = value; }
    void SetRMSCurrent(DataModel::Nullable<int64_t> value) { mRMSCurrent = value; }
    void SetRMSPower(DataModel::Nullable<int64_t> value) { mRMSPower = value; }
    void SetFrequency(DataModel::Nullable<int64_t> value) { mFrequency = value; }
    void SetPowerFactor(DataModel::Nullable<int64_t> value) { mPowerFactor = value; }
    void SetNeutralCurrent(DataModel::Nullable<int64_t> value) { mNeutralCurrent = value; }

private:
    static constexpr uint8_t kMaxListSize = 16;

    // Mandatory attributes
    PowerModeEnum mPowerMode          = PowerModeEnum::kUnknown;
    uint8_t mNumberOfMeasurementTypes = 0;

    // List attributes
    Structs::MeasurementAccuracyStruct::Type mAccuracyList[kMaxListSize];
    uint8_t mAccuracyCount = 0;

    Structs::MeasurementRangeStruct::Type mRangesList[kMaxListSize];
    uint8_t mRangesCount = 0;

    Structs::HarmonicMeasurementStruct::Type mHarmonicCurrentsList[kMaxListSize];
    uint8_t mHarmonicCurrentsCount = 0;

    Structs::HarmonicMeasurementStruct::Type mHarmonicPhasesList[kMaxListSize];
    uint8_t mHarmonicPhasesCount = 0;

    // Optional attributes
    DataModel::Nullable<int64_t> mVoltage;
    DataModel::Nullable<int64_t> mActiveCurrent;
    DataModel::Nullable<int64_t> mReactiveCurrent;
    DataModel::Nullable<int64_t> mApparentCurrent;
    DataModel::Nullable<int64_t> mActivePower;
    DataModel::Nullable<int64_t> mReactivePower;
    DataModel::Nullable<int64_t> mApparentPower;
    DataModel::Nullable<int64_t> mRMSVoltage;
    DataModel::Nullable<int64_t> mRMSCurrent;
    DataModel::Nullable<int64_t> mRMSPower;
    DataModel::Nullable<int64_t> mFrequency;
    DataModel::Nullable<int64_t> mPowerFactor;
    DataModel::Nullable<int64_t> mNeutralCurrent;
};

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
