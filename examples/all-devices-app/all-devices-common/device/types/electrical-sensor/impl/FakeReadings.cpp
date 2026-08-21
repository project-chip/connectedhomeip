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

#include <app/server/Server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <protocols/interaction_model/StatusCode.h>

#include "FakeReadings.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;

using Protocols::InteractionModel::Status;

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
void FakeReadings::StartFakeReadings(int64_t power_mW, uint32_t powerRandomness_mW, int64_t voltage_mV,
                                     uint32_t voltageRandomness_mV, int64_t current_mA, uint32_t currentRandomness_mA,
                                     uint8_t interval_s, bool reset)
{
    VerifyOrDie(interval_s > 0);
    mBasePower_mW         = power_mW;
    mPowerRandomness_mW   = powerRandomness_mW;
    mBaseVoltage_mV       = voltage_mV;
    mVoltageRandomness_mV = voltageRandomness_mV;
    mBaseCurrent_mA       = current_mA;
    mCurrentRandomness_mA = currentRandomness_mA;
    mInterval_s           = interval_s;

    if (reset)
    {
        // Use a fixed random seed to try to avoid random CI test failures
        // which are caused when the test is checking for 2 different numbers.
        // This is statistically more likely when the test runs for a long time
        // or if the seed is not set

        // Disable clang-tidy for this line because it complains that srand(1) will make the random numbers predictable, which is
        // the exact point of this line.
        srand(1); // NOLINT(bugprone-random-generator-seed)

        mTotalEnergyImported = 0;
        mTotalEnergyExported = 0;
    }

    // Call update function to kick off regular readings
    FakeReadingsUpdate();
}

void FakeReadings::StopFakeReadings()
{
    if (mTimerDelegate)
    {
        mTimerDelegate->CancelTimer(this);
    }
}

/**
 * @brief   Sends fake meter data into the cluster and restarts the timer
 */
void FakeReadings::FakeReadingsUpdate()
{
    // Update readings
    // Avoid using floats - so we will do a basic rand() call which will generate a integer value between 0 and RAND_MAX
    // first compute power as a mean + some random value in range +/- mPowerRandomness_mW
    mPower_mW = mPowerRandomness_mW == 0 ? 0 : (static_cast<int64_t>(rand()) % (2 * static_cast<int64_t>(mPowerRandomness_mW))) - mPowerRandomness_mW;
    mPower_mW += mBasePower_mW; // add in the base power

    mVoltage_mV = mVoltageRandomness_mV == 0 ? 0 : (static_cast<int64_t>(rand()) % (2 * static_cast<int64_t>(mVoltageRandomness_mV))) - mVoltageRandomness_mV;
    mVoltage_mV += mBaseVoltage_mV; // add in the base voltage

    /* Note: whilst we could compute a current from the power and voltage,
     * there will always be some random error from the sensor
     * that measures it. To keep this simple and to avoid doing divides in integer
     * format etc use the same approach here too.
     * This is meant more as an example to show how to use the APIs, not
     * to be a real representation of laws of physics.
     */
    mCurrent_mA = mCurrentRandomness_mA == 0 ? 0 : (static_cast<int64_t>(rand()) % (2 * static_cast<int64_t>(mCurrentRandomness_mA))) - mCurrentRandomness_mA;
    mCurrent_mA += mBaseCurrent_mA; // add in the base current

    // update the energy meter - we'll assume that the power has been constant during the previous interval
    if (mPower_mW > 0)
    {
        // Positive power - means power is imported
        mPeriodicEnergyImported = ((mPower_mW * mInterval_s) / 3600);
        mPeriodicEnergyExported = 0;
        mTotalEnergyImported += mPeriodicEnergyImported;
    }
    else
    {
        // Negative power - means power is exported, but the exported energy is reported positive
        mPeriodicEnergyImported = 0;
        mPeriodicEnergyExported = ((-mPower_mW * mInterval_s) / 3600);
        mTotalEnergyExported += mPeriodicEnergyExported;
    }

    if (mEEMCluster)
    {
        mEEMCluster->GenerateSnapshots();
    }

    // start/restart the timer if can
    if (mTimerDelegate)
    {
        mTimerDelegate->CancelTimer(this);
        LogErrorOnFailure(mTimerDelegate->StartTimer(this, System::Clock::Seconds16(mInterval_s)));
    }
}

void FakeReadings::SetTimerDelegate(chip::TimerDelegate & timerDelegate)
{
    // stop previous timer if active
    if (mTimerDelegate)
    {
        StopFakeReadings();
    }
    mTimerDelegate = &timerDelegate;
}
