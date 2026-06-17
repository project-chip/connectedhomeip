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

const IlluminanceMeasurementCluster::StartupConfiguration kDefaultLightConfig = {
    .minMeasuredValue = DataModel::MakeNullable<uint16_t>(1),
    .maxMeasuredValue = DataModel::MakeNullable<uint16_t>(10000),
    .tolerance        = 100, // 1% tolerance
    .lightSensorType  = DataModel::MakeNullable<IlluminanceMeasurement::LightSensorTypeEnum>(
        IlluminanceMeasurement::LightSensorTypeEnum::kPhotodiode),
};

} // namespace

IncreasingLightSensorDevice::IncreasingLightSensorDevice(TimerDelegate & timerDelegate) : LightSensorDevice(timerDelegate, kDefaultLightConfig) {}

IncreasingLightSensorDevice::~IncreasingLightSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingLightSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(LightSensorDevice::Register(endpoint, provider, parentId));
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
    // Simulate increasing light level
    if (mLightMeasuredValue.IsNull())
    {
        mLightMeasuredValue.SetNonNull(kDefaultLightConfig.minMeasuredValue.Value());
    }
    else if (mLightMeasuredValue.Value() >= kDefaultLightConfig.maxMeasuredValue.Value())
    {
        mLightMeasuredValue.SetNonNull(kDefaultLightConfig.minMeasuredValue.Value());
    }
    else
    {
        mLightMeasuredValue.SetNonNull(static_cast<uint16_t>(mLightMeasuredValue.Value() + 10));
    }

    ChipLogProgress(AppServer, "IncreasingLightValue: Increasing to %d", mLightMeasuredValue.Value());
    LogErrorOnFailure(mIlluminanceMeasurementCluster.Cluster().SetMeasuredValue(mLightMeasuredValue));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseLightIntervalSec));
}

} // namespace app
} // namespace chip
