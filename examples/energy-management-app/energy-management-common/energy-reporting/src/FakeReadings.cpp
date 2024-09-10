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

#include <DEMManufacturerDelegate.h>
#include <DeviceEnergyManagementDelegateImpl.h>
#include <EVSEManufacturerImpl.h>
#include <EnergyEvseManager.h>

#include <EnergyTimeUtils.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <protocols/interaction_model/StatusCode.h>

#include "FakeReadings.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;

using Protocols::InteractionModel::Status;

FakeReadings::FakeReadings() {}

FakeReadings::~FakeReadings() {}

/* static */
FakeReadings & FakeReadings::GetInstance()
{
    static FakeReadings sInstance;

    return sInstance;
}

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
 * @param[in]   bReset       - boolean: true will reset the energy values to 0
 */
void FakeReadings::StartFakeReadings(EndpointId aEndpointId, int64_t aPower_mW, uint32_t aPowerRandomness_mW, int64_t aVoltage_mV,
                                     uint32_t aVoltageRandomness_mV, int64_t aCurrent_mA, uint32_t aCurrentRandomness_mA,
                                     uint8_t aInterval_s, bool bReset)
{
    bEnabled              = true;
    mEndpointId           = aEndpointId;
    mPower_mW             = aPower_mW;
    mPowerRandomness_mW   = aPowerRandomness_mW;
    mVoltage_mV           = aVoltage_mV;
    mVoltageRandomness_mV = aVoltageRandomness_mV;
    mCurrent_mA           = aCurrent_mA;
    mCurrentRandomness_mA = aCurrentRandomness_mA;
    mInterval_s           = aInterval_s;

    if (bReset)
    {
        // Use a fixed random seed to try to avoid random CI test failures
        // which are caused when the test is checking for 2 different numbers.
        // This is statistically more likely when the test runs for a long time
        // or if the seed is not set
        srand(1);

        mTotalEnergyImported = 0;
        mTotalEnergyExported = 0;
    }

    // Call update function to kick off regular readings
    FakeReadingsUpdate();
}

/**
 * @brief   Stops any active updates to the fake load data callbacks
 */
void FakeReadings::StopFakeReadings()
{
    bEnabled = false;
}

/**
 * @brief   Sends fake meter data into the cluster and restarts the timer
 */
void FakeReadings::FakeReadingsUpdate()
{
    /* Check to see if the fake Load is still running - don't send updates if the timer was already cancelled */
    if (!bEnabled)
    {
        return;
    }

    // Update readings
    // Avoid using floats - so we will do a basic rand() call which will generate a integer value between 0 and RAND_MAX
    // first compute power as a mean + some random value in range +/- mPowerRandomness_mW
    int64_t power = (static_cast<int64_t>(rand()) % (2 * mPowerRandomness_mW)) - mPowerRandomness_mW;
    power += mPower_mW; // add in the base power

    int64_t voltage = (static_cast<int64_t>(rand()) % (2 * mVoltageRandomness_mV)) - mVoltageRandomness_mV;
    voltage += mVoltage_mV; // add in the base voltage

    /* Note: whilst we could compute a current from the power and voltage,
     * there will always be some random error from the sensor
     * that measures it. To keep this simple and to avoid doing divides in integer
     * format etc use the same approach here too.
     * This is meant more as an example to show how to use the APIs, not
     * to be a real representation of laws of physics.
     */
    int64_t current = (static_cast<int64_t>(rand()) % (2 * mCurrentRandomness_mA)) - mCurrentRandomness_mA;
    current += mCurrent_mA; // add in the base current

    GetEvseManufacturer()->SendPowerReading(mEndpointId, power, voltage, current);

    // update the energy meter - we'll assume that the power has been constant during the previous interval
    if (mPower_mW > 0)
    {
        // Positive power - means power is imported
        mPeriodicEnergyImported = ((power * mInterval_s) / 3600);
        mPeriodicEnergyExported = 0;
        mTotalEnergyImported += mPeriodicEnergyImported;
    }
    else
    {
        // Negative power - means power is exported, but the exported energy is reported positive
        mPeriodicEnergyImported = 0;
        mPeriodicEnergyExported = ((-power * mInterval_s) / 3600);
        mTotalEnergyExported += mPeriodicEnergyExported;
    }

    GetEvseManufacturer()->SendPeriodicEnergyReading(mEndpointId, mPeriodicEnergyImported, mPeriodicEnergyExported);

    GetEvseManufacturer()->SendCumulativeEnergyReading(mEndpointId, mTotalEnergyImported, mTotalEnergyExported);

    // start/restart the timer
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(mInterval_s), FakeReadingsTimerExpiry, this);
}

/**
 * @brief   Timer expiry callback to handle fake load
 */
void FakeReadings::FakeReadingsTimerExpiry(System::Layer * systemLayer, void * manufacturer)
{
    FakeReadings * mn = reinterpret_cast<FakeReadings *>(manufacturer);

    mn->FakeReadingsUpdate();
}

void FakeReadings::SetPower(Power_mW aPower_mW)
{
    mPower_mW = aPower_mW;
}
Power_mW FakeReadings::GetPower()
{
    return mPower_mW;
};
void FakeReadings::SetVoltage(Voltage_mV aVoltage_mV)
{
    mVoltage_mV = aVoltage_mV;
}
Voltage_mV FakeReadings::GetVoltage()
{
    return mVoltage_mV;
};
void FakeReadings::SetCurrent(Amperage_mA aCurrent_mA)
{
    mCurrent_mA = aCurrent_mA;
}
Amperage_mA FakeReadings::GetCurrent()
{
    return mCurrent_mA;
}
