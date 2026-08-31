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

#include <crypto/RandUtils.h>

#include "FakeReadings.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

CHIP_ERROR FakeReadings::HandleEventTrigger(uint64_t eventTrigger)
{
    EnergyReportingTrigger trigger = static_cast<EnergyReportingTrigger>(clearEndpointInEventTrigger(eventTrigger));

    switch (trigger)
    {
    case EnergyReportingTrigger::kFakeReadingsStop:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Stop Fake load");
        StopFakeReadings();
        break;
    case EnergyReportingTrigger::kFakeReadingsLoadStart_1kW_2s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake load 1kW @2s Import");
        StartFakeReadings({ .power_mW             = 1'000'000, // Fake load 1000 W
                            .powerRandomness_mW   = 20'000,    // randomness 20W
                            .voltage_mV           = 230'000,   // Fake Voltage 230V
                            .voltageRandomness_mV = 1'000,     // randomness 1V
                            .current_mA           = 4'348,     // Fake Current (at 1kW@230V = 4.3478 Amps)
                            .currentRandomness_mA = 500,       // randomness 500mA
                            .interval_s           = 2,         // 2s updates
                            .reset                = true });
        break;
    case EnergyReportingTrigger::kFakeReadingsGenStart_3kW_5s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake generator 3kW @5s Export");
        StartFakeReadings({ .power_mW             = -3'000'000, // Fake Generator -3000 W
                            .powerRandomness_mW   = 20'000,     // randomness 20W
                            .voltage_mV           = 230'000,    // Fake Voltage 230V
                            .voltageRandomness_mV = 1'000,      // randomness 1V
                            .current_mA           = -13'043,    // Fake Current (at -3kW@230V = -13.0434 Amps)
                            .currentRandomness_mA = 500,        // randomness 500mA
                            .interval_s           = 5,          // 5s updates
                            .reset                = true });
        break;

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void FakeReadings::StartFakeReadings(Parameters params)
{
    VerifyOrDie(params.interval_s > 0);
    mBasePower_mW         = params.power_mW;
    mPowerRandomness_mW   = params.powerRandomness_mW;
    mBaseVoltage_mV       = params.voltage_mV;
    mVoltageRandomness_mV = params.voltageRandomness_mV;
    mBaseCurrent_mA       = params.current_mA;
    mCurrentRandomness_mA = params.currentRandomness_mA;
    mInterval_s           = params.interval_s;

    if (params.reset)
    {
        mTotalEnergyImported = 0;
        mTotalEnergyExported = 0;
    }

    // Call update function to kick off regular readings
    FakeReadingsUpdate();
}

void FakeReadings::StopFakeReadings()
{
    mTimerDelegate.CancelTimer(this);
}

void FakeReadings::FakeReadingsUpdate()
{
    // Update readings

    // base + randomness

    // for faking and testing purposes, insead of true random values, we will incrementally change the value in the specified range
    // from base - randomness to base + randomness in mDeterministicOffsetStepCount steps, then repeat
    mPower_mW = mBasePower_mW +
        (mPowerRandomness_mW == 0
             ? 0
             : (static_cast<int64_t>(mDeterministicOffsetStep * (2 * mPowerRandomness_mW) / (mDeterministicOffsetStepCount - 1)) -
                mPowerRandomness_mW));

    mVoltage_mV = mBaseVoltage_mV +
        (mVoltageRandomness_mV == 0
             ? 0
             : (static_cast<int64_t>(mDeterministicOffsetStep * (2 * mVoltageRandomness_mV) / (mDeterministicOffsetStepCount - 1)) -
                mVoltageRandomness_mV));

    /* Note: whilst we could compute a current from the power and voltage,
     * there will always be some random error from the sensor
     * that measures it. To keep this simple and to avoid doing divides in integer
     * format etc use the same approach here too.
     * This is meant more as an example to show how to use the APIs, not
     * to be a real representation of laws of physics.
     */
    mCurrent_mA = mBaseCurrent_mA +
        (mCurrentRandomness_mA == 0
             ? 0
             : (static_cast<int64_t>(mDeterministicOffsetStep * (2 * mCurrentRandomness_mA) / (mDeterministicOffsetStepCount - 1)) -
                mCurrentRandomness_mA));

    // mDeterministicOffsetStep increment from 0 to mDeterministicOffsetStepCount exclusive then reset to 0
    mDeterministicOffsetStep++;
    mDeterministicOffsetStep %= mDeterministicOffsetStepCount;

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

    ChipLogProgress(Support,
                    "FakeReadingsUpdate: BasePower=%lld mW, Power=%lld mW, BaseVoltage=%lld mV, Voltage=%lld mV, BaseCurrent=%lld "
                    "mA, Current=%lld "
                    "mA, PeriodicEnergyImported=%lld Wh, PeriodicEnergyExported=%lld Wh, TotalEnergyImported=%lld Wh, "
                    "TotalEnergyExported=%lld Wh",
                    static_cast<long long>(mBasePower_mW), static_cast<long long>(mPower_mW),
                    static_cast<long long>(mBaseVoltage_mV), static_cast<long long>(mVoltage_mV),
                    static_cast<long long>(mBaseCurrent_mA), static_cast<long long>(mCurrent_mA),
                    static_cast<long long>(mPeriodicEnergyImported), static_cast<long long>(mPeriodicEnergyExported),
                    static_cast<long long>(mTotalEnergyImported), static_cast<long long>(mTotalEnergyExported));
    if (mEEMCluster)
    {
        mEEMCluster->GenerateSnapshots();
    }

    // start/restart the timer
    mTimerDelegate.CancelTimer(this);
    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds16(mInterval_s)));
}
