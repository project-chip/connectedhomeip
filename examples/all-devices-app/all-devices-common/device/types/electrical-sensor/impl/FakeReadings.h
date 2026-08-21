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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/util/basic-types.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>

class FakeReadings : public chip::TimerContext
{
    using ElectricalEnergyMeasurementCluster = chip::app::Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster;

public:
    ~FakeReadings() override { StopFakeReadings(); }

    /**
     * @brief   Starts a fake load/generator to periodically callback the power and energy
     *          clusters.
     * @param[in]   power_mW    - the mean power of the load
     *                             Positive power indicates Imported energy (e.g. a load)
     *                             Negative power indicated Exported energy (e.g. a generator)
     * @param[in]   powerRandomness_mW  This is used to define the max randomness of the
     *                             random power values around the mean power of the load
     * @param[in]   voltage_mV  - the nominal voltage measurement
     * @param[in]   voltageRandomness_mV  This is used to define the max randomness of the
     *                             random voltage values
     * @param[in]   current_mA  - the nominal current measurement
     * @param[in]   currentRandomness_mA  This is used to define the max randomness of the
     *                             random current values
     * @param[in]   interval_s  - the callback interval in seconds
     * @param[in]   reset       - boolean: true will reset the energy values to 0
     */
    void StartFakeReadings(int64_t power_mW, uint32_t powerRandomness_mW, int64_t voltage_mV, uint32_t voltageRandomness_mV,
                           int64_t current_mA, uint32_t currentRandomness_mA, uint8_t interval_s, bool reset);

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
    void SetTimerDelegate(chip::TimerDelegate & timerDelegate);
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
    int64_t mPower_mW;

    /* Current voltage reading (base + randomness) */
    int64_t mVoltage_mV;

    /* Current ActiveCurrent reading (base + randomness) */
    int64_t mCurrent_mA;

    /* Cumulative Energy Imported which is updated if mPower > 0 */
    int64_t mTotalEnergyImported = 0;

    /* Cumulative Energy Imported which is updated if mPower < 0 */
    int64_t mTotalEnergyExported = 0;

    /* Periodic Energy Imported which is updated if mPower > 0 */
    int64_t mPeriodicEnergyImported = 0;

    /* Periodic Energy Imported which is updated if mPower < 0 */
    int64_t mPeriodicEnergyExported = 0;

    // TimerDelegate running this TimerContext
    chip::TimerDelegate * mTimerDelegate = nullptr;

    // EEM cluster to call GenerateSnapshot() on
    ElectricalEnergyMeasurementCluster * mEEMCluster = nullptr;
};
