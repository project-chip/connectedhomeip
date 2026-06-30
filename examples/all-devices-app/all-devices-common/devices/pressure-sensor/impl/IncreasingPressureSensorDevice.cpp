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
constexpr int16_t kPressureStepValue                            = 50;
constexpr int16_t kDefaultMinPressure                           = 0;
constexpr int16_t kDefaultMaxPressure                           = 10000;
constexpr uint16_t kDefaultPressureTolerance                    = 10;

const PressureMeasurementCluster::Config kDefaultPressureConfig = []() {
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable<int16_t>(kDefaultMinPressure);
    config.maxMeasuredValue = DataModel::MakeNullable<int16_t>(kDefaultMaxPressure);
    config.WithTolerance(kDefaultPressureTolerance);
    return config;
}();

} // namespace

IncreasingPressureSensorDevice::IncreasingPressureSensorDevice(TimerDelegate & timerDelegate) :
    PressureSensorDevice(timerDelegate, kDefaultPressureConfig)
{}

IncreasingPressureSensorDevice::~IncreasingPressureSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingPressureSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                    EndpointComposition composition)
{
    ReturnErrorOnFailure(PressureSensorDevice::Register(endpoint, provider, composition));

    // Initialize with the minimum configured value
    mPressureMeasuredValue = kDefaultPressureConfig.minMeasuredValue.Value();
    ReturnErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mPressureMeasuredValue)));

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
    mPressureMeasuredValue = static_cast<int16_t>(mPressureMeasuredValue + kPressureStepValue);
    if (mPressureMeasuredValue > kDefaultPressureConfig.maxMeasuredValue.Value())
    {
        mPressureMeasuredValue = kDefaultPressureConfig.minMeasuredValue.Value();
    }

    ChipLogProgress(AppServer, "IncreasingPressureValue: Increasing to %d", static_cast<int>(mPressureMeasuredValue));
    LogErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mPressureMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec));
}

} // namespace app
} // namespace chip
