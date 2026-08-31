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

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/util/basic-types.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>

class FakeReadings : public chip::TimerContext, public chip::TestEventTriggerHandler
{
    enum class EnergyReportingTrigger : uint64_t
    {
        // Stop Fake readings
        kFakeReadingsStop = 0x0091'0000'0000'0000,
        // Fake a load (importing) readings at 1kW 230V 4.34A with 2s updates
        kFakeReadingsLoadStart_1kW_2s = 0x0091'0000'0000'0001,
        // Fake a generator (exporting) readings at 3kW 230V 3.33A with 5s updates
        kFakeReadingsGenStart_3kW_5s = 0x0091'0000'0000'0002
    };
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

    using ElectricalEnergyMeasurementCluster = chip::app::Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster;

public:
    FakeReadings(chip::TimerDelegate & timerDelegate) : mTimerDelegate(timerDelegate) {}
    ~FakeReadings() override { StopFakeReadings(); }

    struct Parameters
    {
        // Starts a fake load/generator to periodically callback the power and energy
        // clusters.
        // The mean power of the load
        // Positive power indicates Imported energy (e.g. a load)
        // Negative power indicated Exported energy (e.g. a generator)
        int64_t power_mW;

        // This is used to define the max randomness of the
        // random power values around the mean power of the load
        uint32_t powerRandomness_mW;

        // The nominal voltage measurement
        int64_t voltage_mV;

        // This is used to define the max randomness of the
        // random voltage values
        uint32_t voltageRandomness_mV;

        // The nominal current measurement
        int64_t current_mA;

        // This is used to define the max randomness of the
        // random current values
        uint32_t currentRandomness_mA;

        // The callback interval in seconds
        uint8_t interval_s;

        // If true, will reset the energy values to 0
        bool reset;
    };

    void StartFakeReadings(Parameters params);

    /**
     * @brief   Stops any active updates to the fake load data callbacks
     */
    void StopFakeReadings();

    /**
     * @brief   Updates fake reading values and restarts the timer
     */
    void FakeReadingsUpdate();

    chip::Power_mW GetActivePower() { return mPower_mW; }
    chip::Voltage_mV GetVoltage() { return mVoltage_mV; }
    chip::Amperage_mA GetActiveCurrent() { return mCurrent_mA; }
    int64_t GetPeriodicEnergyImported() { return mPeriodicEnergyImported; }
    int64_t GetPeriodicEnergyExported() { return mPeriodicEnergyExported; }
    int64_t GetCumulativeEnergyImported() { return mTotalEnergyImported; }
    int64_t GetCumulativeEnergyExported() { return mTotalEnergyExported; }
    void SetEEMCluster(ElectricalEnergyMeasurementCluster * eemCluster) { mEEMCluster = eemCluster; }
    ElectricalEnergyMeasurementCluster * GetEEMCluster() { return mEEMCluster; }

    // TimerContext override
    void TimerFired() override { FakeReadingsUpdate(); }

private:
    /* Interval in seconds to callback */
    uint8_t mInterval_s;

    /* Active Power on the load in mW (signed value) +ve = imported */
    int64_t mBasePower_mW;

    /* The amount to randomize the Power on the load in mW */
    uint32_t mPowerRandomness_mW;

    /* Voltage reading in mV (signed value) */
    int64_t mBaseVoltage_mV;

    /* The amount to randomize the Voltage in mV */
    uint32_t mVoltageRandomness_mV;

    /* ActiveCurrent reading in mA (signed value) */
    int64_t mBaseCurrent_mA;

    /* The amount to randomize the ActiveCurrent in mA */
    uint32_t mCurrentRandomness_mA;

    /* These energy values can only be positive values. However the underlying
     * energy type (power_mWh) is signed, so keeping with that convention.
     */

    /* Current power reading (base + randomness) */
    int64_t mPower_mW = 0;

    /* Current voltage reading (base + randomness) */
    int64_t mVoltage_mV = 0;

    /* Current ActiveCurrent reading (base + randomness) */
    int64_t mCurrent_mA = 0;

    /* Cumulative Energy Imported which is updated if mPower > 0 */
    int64_t mTotalEnergyImported = 0;

    /* Cumulative Energy Imported which is updated if mPower < 0 */
    int64_t mTotalEnergyExported = 0;

    /* Periodic Energy Imported which is updated if mPower > 0 */
    int64_t mPeriodicEnergyImported = 0;

    /* Periodic Energy Imported which is updated if mPower < 0 */
    int64_t mPeriodicEnergyExported = 0;

    // TimerDelegate running this TimerContext
    chip::TimerDelegate & mTimerDelegate;

    // EEM cluster to call `GenerateSnapshot()` on
    ElectricalEnergyMeasurementCluster * mEEMCluster = nullptr;

    // For faking and testing purposes, insead of random values, we will incrementally change the value in a range
    uint32_t mDeterministicOffsetStep = 0;
    // Because of some hardcoded values in the test plan for ElectricalEnergyMeasurement, we need this number to be less then 20 to
    // ensure the test TC_EEM_2_4 will definitly pass. 9 is chosen to have some nice values when the code actually runs.
    constexpr static uint32_t mDeterministicOffsetStepCount = 9;
};
