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
#include "IncreasingLightSensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseLightIntervalSec = System::Clock::Seconds16(10);
constexpr uint16_t kLightStepValue                           = 10;
constexpr uint16_t kDefaultMinLight                          = 1;
constexpr uint16_t kDefaultMaxLight                          = 10000;
constexpr uint16_t kDefaultLightTolerance                    = 100; // 1% tolerance

const IlluminanceMeasurementCluster::StartupConfiguration kDefaultLightConfig = {
    .minMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMinLight),
    .maxMeasuredValue = DataModel::MakeNullable<uint16_t>(kDefaultMaxLight),
    .tolerance        = kDefaultLightTolerance,
    .lightSensorType  = DataModel::MakeNullable<IlluminanceMeasurement::LightSensorTypeEnum>(
        IlluminanceMeasurement::LightSensorTypeEnum::kPhotodiode),
};

} // namespace

IncreasingLightSensorDevice::IncreasingLightSensorDevice(TimerDelegate & timerDelegate) :
    LightSensorDevice(timerDelegate, kDefaultLightConfig, []() {
        IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributes;
        // Enable optional Tolerance and LightSensorType attributes.
        // These optional attributes are enabled to support the Illuminance Measurement
        // YAML certification tests (Test_TC_ILL_2_1.yaml) executed against this simulated device.
        optionalAttributes.Set<IlluminanceMeasurement::Attributes::Tolerance::Id>();
        optionalAttributes.Set<IlluminanceMeasurement::Attributes::LightSensorType::Id>();
        return optionalAttributes;
    }())
{}

IncreasingLightSensorDevice::~IncreasingLightSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingLightSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                 EndpointComposition composition)
{
    ReturnErrorOnFailure(LightSensorDevice::Register(endpoint, provider, composition));

    // Initialize with the minimum configured value
    mLightMeasuredValue = kDefaultLightConfig.minMeasuredValue.Value();
    ReturnErrorOnFailure(mIlluminanceMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mLightMeasuredValue)));

    // Kick off the timer loop to increase light level every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseLightIntervalSec);
}

void IncreasingLightSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    LightSensorDevice::Unregister(provider);
}

void IncreasingLightSensorDevice::TimerFired()
{
    mLightMeasuredValue = static_cast<uint16_t>(mLightMeasuredValue + kLightStepValue);
    if (mLightMeasuredValue > kDefaultLightConfig.maxMeasuredValue.Value())
    {
        mLightMeasuredValue = kDefaultLightConfig.minMeasuredValue.Value();
    }

    ChipLogProgress(AppServer, "IncreasingLightValue: Increasing to %u", static_cast<unsigned int>(mLightMeasuredValue));
    LogErrorOnFailure(mIlluminanceMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mLightMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseLightIntervalSec));
}

} // namespace app
} // namespace chip
