/*
 *
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
#include "IncreasingTemperatureSensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseTemperatureIntervalSec = System::Clock::Seconds16(10);

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTemperatureConfig = {
    .minMeasuredValue = DataModel::MakeNullable<int16_t>(-10),
    .maxMeasuredValue = DataModel::MakeNullable<int16_t>(50),
    .tolerance        = 0,
};

} // namespace

IncreasingTemperatureSensorDevice::IncreasingTemperatureSensorDevice() :
    TemperatureSensorDevice(mTimerDelegate, kDefaultTemperatureConfig)
{}

IncreasingTemperatureSensorDevice::~IncreasingTemperatureSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingTemperatureSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                       EndpointId parentId)
{
    ReturnErrorOnFailure(TemperatureSensorDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to increase temperature every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseTemperatureIntervalSec);
}

void IncreasingTemperatureSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    TemperatureSensorDevice::Unregister(provider);
}

void IncreasingTemperatureSensorDevice::TimerFired()
{
    // Simulate increasing temperature
    if (mTemperatureMeasuredValue.IsNull())
    {
        mTemperatureMeasuredValue.SetNonNull(kDefaultTemperatureConfig.minMeasuredValue.Value());
    }
    else if (mTemperatureMeasuredValue.Value() >= kDefaultTemperatureConfig.maxMeasuredValue.Value())
    {
        mTemperatureMeasuredValue.SetNonNull(kDefaultTemperatureConfig.minMeasuredValue.Value());
    }
    else
    {
        mTemperatureMeasuredValue.SetNonNull(static_cast<int16_t>(mTemperatureMeasuredValue.Value() + 1));
    }

    ChipLogProgress(AppServer, "IncreasingTemperatureValue: Increasing to %d", mTemperatureMeasuredValue.Value());
    LogErrorOnFailure(mTemperatureMeasurementCluster.Cluster().SetMeasuredValue(mTemperatureMeasuredValue));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseTemperatureIntervalSec));
}

} // namespace app
} // namespace chip
