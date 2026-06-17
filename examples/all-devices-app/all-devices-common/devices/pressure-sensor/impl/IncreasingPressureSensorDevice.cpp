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
#include "IncreasingPressureSensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreasePressureIntervalSec = System::Clock::Seconds16(10);

const PressureMeasurementCluster::Config kDefaultPressureConfig = []() {
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable<int16_t>(0);
    config.maxMeasuredValue = DataModel::MakeNullable<int16_t>(10000);
    config.WithTolerance(10);
    return config;
}();

} // namespace

IncreasingPressureSensorDevice::IncreasingPressureSensorDevice(TimerDelegate & timerDelegate) : PressureSensorDevice(timerDelegate, kDefaultPressureConfig) {}

IncreasingPressureSensorDevice::~IncreasingPressureSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingPressureSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                    EndpointId parentId)
{
    ReturnErrorOnFailure(PressureSensorDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to increase pressure every few seconds
    return mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec);
}

void IncreasingPressureSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    PressureSensorDevice::Unregister(provider);
}

void IncreasingPressureSensorDevice::TimerFired()
{
    // Simulate increasing pressure
    if (mPressureMeasuredValue.IsNull())
    {
        mPressureMeasuredValue.SetNonNull(kDefaultPressureConfig.minMeasuredValue.Value());
    }
    else if (mPressureMeasuredValue.Value() >= kDefaultPressureConfig.maxMeasuredValue.Value())
    {
        mPressureMeasuredValue.SetNonNull(kDefaultPressureConfig.minMeasuredValue.Value());
    }
    else
    {
        mPressureMeasuredValue.SetNonNull(static_cast<int16_t>(mPressureMeasuredValue.Value() + 50));
    }

    ChipLogProgress(AppServer, "IncreasingPressureValue: Increasing to %d", mPressureMeasuredValue.Value());
    LogErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(mPressureMeasuredValue));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec));
}

} // namespace app
} // namespace chip
