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
#include "IncreasingLightSensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseLightIntervalSec = System::Clock::Seconds16(10);
constexpr uint16_t kLightStepLux                             = 10;

} // namespace

IncreasingLightSensor::IncreasingLightSensor(TimerDelegate & timerDelegate,
                                             IlluminanceMeasurementCluster::StartupConfiguration lightConfig,
                                             IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributes) :
    LightSensor(timerDelegate, lightConfig, optionalAttributes)
{}

IncreasingLightSensor::~IncreasingLightSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingLightSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                           EndpointComposition composition)
{
    ReturnErrorOnFailure(LightSensor::Register(endpoint, provider, composition));

    // Initialize with the minimum configured value
    ReturnErrorOnFailure(mIlluminanceMeasurementCluster.Cluster().SetMeasuredValue(mLightConfig.minMeasuredValue));

    // Kick off the timer loop to increase light level every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseLightIntervalSec);
}

void IncreasingLightSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    LightSensor::Unregister(provider);
}

void IncreasingLightSensor::TimerFired()
{
    const uint16_t minLight     = mLightConfig.minMeasuredValue.ValueOr(1);
    const uint16_t maxLight     = mLightConfig.maxMeasuredValue.ValueOr(static_cast<uint16_t>(UINT16_MAX - 1));
    const uint16_t currentLight = mIlluminanceMeasurementCluster.Cluster().GetMeasuredValue().ValueOr(minLight);
    const uint32_t nextLight    = static_cast<uint32_t>(currentLight) + kLightStepLux;

    const uint16_t nextLightValue = nextLight > maxLight ? minLight : static_cast<uint16_t>(nextLight);

    ChipLogProgress(AppServer, "IncreasingLightValue: Increasing to %u", static_cast<unsigned int>(nextLightValue));
    LogErrorOnFailure(mIlluminanceMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(nextLightValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseLightIntervalSec));
}

} // namespace app
} // namespace chip
