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
#include "IncreasingHumiditySensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseHumidityIntervalSec = System::Clock::Seconds16(10);
constexpr uint16_t kHumidityStepValue                           = 100; // step by 1.00%

} // namespace

IncreasingHumiditySensor::IncreasingHumiditySensor(TimerDelegate & timerDelegate,
                                                   RelativeHumidityMeasurementCluster::Config humidityConfig) :
    HumiditySensor(timerDelegate, humidityConfig)
{}

IncreasingHumiditySensor::~IncreasingHumiditySensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingHumiditySensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    ReturnErrorOnFailure(HumiditySensor::Register(endpoint, provider, composition));

    // Initialize with the minimum configured value
    mHumidityMeasuredValue = mHumidityConfig.minMeasuredValue.ValueOr(0);
    ReturnErrorOnFailure(
        mRelativeHumidityMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mHumidityMeasuredValue)));

    // Kick off the timer loop to increase humidity every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseHumidityIntervalSec);
}

void IncreasingHumiditySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    HumiditySensor::Unregister(provider);
}

void IncreasingHumiditySensor::TimerFired()
{
    const uint16_t minHumidity  = mHumidityConfig.minMeasuredValue.ValueOr(0);
    const uint16_t maxHumidity  = mHumidityConfig.maxMeasuredValue.ValueOr(10000);
    const uint32_t nextHumidity = static_cast<uint32_t>(mHumidityMeasuredValue) + kHumidityStepValue;

    if (nextHumidity > maxHumidity)
    {
        mHumidityMeasuredValue = minHumidity;
    }
    else
    {
        mHumidityMeasuredValue = static_cast<uint16_t>(nextHumidity);
    }

    ChipLogProgress(AppServer, "IncreasingHumidityValue: Increasing to %u", static_cast<unsigned int>(mHumidityMeasuredValue));
    LogErrorOnFailure(
        mRelativeHumidityMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mHumidityMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseHumidityIntervalSec));
}

} // namespace app
} // namespace chip
