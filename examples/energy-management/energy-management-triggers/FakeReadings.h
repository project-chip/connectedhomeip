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

#include <app/util/basic-types.h>
#include <lib/core/DataModelTypes.h>
#include <system/SystemLayer.h>

class FakeReadings
{
public:
    static FakeReadings & GetInstance();

    /**
     * @brief   Starts a fake load/generator to periodically callback the power and energy
     *          clusters.
     * @param[in]   aEndpointId  - which endpoint is the meter to be updated on
     * @param[in]   aPower_mW    - the mean power of the load
     *                             Positive power indicates Imported energy (e.g. a load)
     *                             Negative power indicated Exported energy (e.g. a generator)
     * @param[in]   aPowerRandomness_mW  This is used to define the max randomness of the
     *                             random power values around the mean power of the load
     * @param[in]   aVoltage_mV  - the nominal voltage measurement
     * @param[in]   aVoltageRandomness_mV  This is used to define the max randomness of the
     *                             random voltage values
     * @param[in]   aCurrent_mA  - the nominal current measurement
     * @param[in]   aCurrentRandomness_mA  This is used to define the max randomness of the
     *                             random current values
     * @param[in]   aInterval_s  - the callback interval in seconds
     * @param[in]   aReset       - boolean: true will reset the energy values to 0
     */
    void StartFakeReadings(chip::EndpointId aEndpointId, int64_t aPower_mW, uint32_t aPowerRandomness_mW, int64_t aVoltage_mV,
                           uint32_t aVoltageRandomness_mV, int64_t aCurrent_mA, uint32_t aCurrentRandomness_mA, uint8_t aInterval_s,
                           bool aReset);

    /**
     * @brief   Stops any active updates to the fake load data callbacks
     */
    void StopFakeReadings();

    /**
     * @brief   Sends fake meter data into the cluster and restarts the timer
     */
    void FakeReadingsUpdate();

    void SetPower(chip::Power_mW power_mW);
    chip::Power_mW GetPower();

    void SetVoltage(chip::Voltage_mV voltage_mV);
    chip::Voltage_mV GetVoltage();

    void SetCurrent(chip::Amperage_mA current_mA);
    chip::Amperage_mA GetCurrent();

    /**
     * @brief   Timer expiry callback to handle fake load
     */
    static void FakeReadingsTimerExpiry(chip::System::Layer * systemLayer, void * manufacturer);

    CHIP_ERROR ConfigureForecast(uint16_t numSlots);

private:
    FakeReadings();
    ~FakeReadings();

private:
    /* If enabled then the timer callback will re-trigger */
    bool bEnabled;

    /* Which endpoint the meter is on */
    chip::EndpointId mEndpointId;

    /* Interval in seconds to callback */
    uint8_t mInterval_s;

    /* Active Power on the load in mW (signed value) +ve = imported */
    int64_t mPower_mW;

    /* The amount to randomize the Power on the load in mW */
    uint32_t mPowerRandomness_mW;

    /* Voltage reading in mV (signed value) */
    int64_t mVoltage_mV;

    /* The amount to randomize the Voltage in mV */
    uint32_t mVoltageRandomness_mV;

    /* ActiveCurrent reading in mA (signed value) */
    int64_t mCurrent_mA;

    /* The amount to randomize the ActiveCurrent in mA */
    uint32_t mCurrentRandomness_mA;

    /* These energy values can only be positive values. However the underlying
     * energy type (power_mWh) is signed, so keeping with that convention.
     */

    /* Cumulative Energy Imported which is updated if mPower > 0 */
    int64_t mTotalEnergyImported = 0;

    /* Cumulative Energy Imported which is updated if mPower < 0 */
    int64_t mTotalEnergyExported = 0;

    /* Periodic Energy Imported which is updated if mPower > 0 */
    int64_t mPeriodicEnergyImported = 0;

    /* Periodic Energy Imported which is updated if mPower < 0 */
    int64_t mPeriodicEnergyExported = 0;
};
