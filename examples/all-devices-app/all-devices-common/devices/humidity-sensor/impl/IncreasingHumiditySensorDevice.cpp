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
#include "IncreasingHumiditySensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseHumidityIntervalSec = System::Clock::Seconds16(10);
constexpr uint16_t kHumidityStepValue                           = 100;   // step by 1.00%
constexpr uint16_t kDefaultMinHumidity                          = 0;     // 0.00%
constexpr uint16_t kDefaultMaxHumidity                          = 10000; // 100.00%
constexpr uint16_t kDefaultHumidityTolerance                    = 100;   // 1.00%

const RelativeHumidityMeasurementCluster::Config kDefaultHumidityConfig = []() {
    RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMinHumidity);
    config.maxMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMaxHumidity);
    config.WithTolerance(kDefaultHumidityTolerance);
    return config;
}();

} // namespace

IncreasingHumiditySensorDevice::IncreasingHumiditySensorDevice(TimerDelegate & timerDelegate) :
    HumiditySensorDevice(timerDelegate, kDefaultHumidityConfig)
{}

IncreasingHumiditySensorDevice::~IncreasingHumiditySensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingHumiditySensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                    EndpointId parentId)
{
    ReturnErrorOnFailure(HumiditySensorDevice::Register(endpoint, provider, parentId));

    // Initialize with the minimum configured value
    mHumidityMeasuredValue = kDefaultHumidityConfig.minMeasuredValue.Value();
    ReturnErrorOnFailure(
        mRelativeHumidityMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mHumidityMeasuredValue)));

    // Kick off the timer loop to increase humidity every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseHumidityIntervalSec);
}

void IncreasingHumiditySensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    HumiditySensorDevice::Unregister(provider);
}

void IncreasingHumiditySensorDevice::TimerFired()
{
    mHumidityMeasuredValue = static_cast<uint16_t>(mHumidityMeasuredValue + kHumidityStepValue);
    if (mHumidityMeasuredValue > kDefaultHumidityConfig.maxMeasuredValue.Value())
    {
        mHumidityMeasuredValue = kDefaultHumidityConfig.minMeasuredValue.Value();
    }

    ChipLogProgress(AppServer, "IncreasingHumidityValue: Increasing to %u", static_cast<unsigned int>(mHumidityMeasuredValue));
    LogErrorOnFailure(
        mRelativeHumidityMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mHumidityMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseHumidityIntervalSec));
}

} // namespace app
} // namespace chip
