/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "IncreasingMoistureSoilSensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace chip {
namespace app {

namespace {
constexpr System::Clock::Seconds16 kIncreaseMoistureIntervalSec = System::Clock::Seconds16(10);
} // namespace

IncreasingMoistureSoilSensor::IncreasingMoistureSoilSensor() : SoilSensor(mTimerDelegate, /* includeTemperature = */ true) {}

IncreasingMoistureSoilSensor::~IncreasingMoistureSoilSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingMoistureSoilSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    ReturnErrorOnFailure(SoilSensor::Register(endpoint, provider, composition));
    // Kick off the timer loop to increase moisture every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec);
}

void IncreasingMoistureSoilSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    SoilSensor::Unregister(provider);
}

void IncreasingMoistureSoilSensor::TimerFired()
{
    if (mSoilMoistureMeasuredValue.IsNull())
    {
        mSoilMoistureMeasuredValue.SetNonNull(mMoistureLimits.minMeasuredValue);
    }
    else if (mSoilMoistureMeasuredValue.Value() >= mMoistureLimits.maxMeasuredValue)
    {
        mSoilMoistureMeasuredValue.SetNonNull(mMoistureLimits.minMeasuredValue);
    }
    else
    {
        mSoilMoistureMeasuredValue.SetNonNull(static_cast<chip::Percent>(mSoilMoistureMeasuredValue.Value() + 1U));
    }

    ChipLogProgress(AppServer, "IncreasingMoistureValue: Increasing to %d", mSoilMoistureMeasuredValue.Value());
    LogErrorOnFailure(mSoilMeasurementCluster.Cluster().SetSoilMoistureMeasuredValue(mSoilMoistureMeasuredValue));

    // Simulate increasing temperature if enabled
    if (HasTemperature() && mTempConfig.has_value())
    {
        int16_t minTemp = mTempConfig->minMeasuredValue.ValueOr(-1000);
        int16_t maxTemp = mTempConfig->maxMeasuredValue.ValueOr(5000);

        if (mTemperatureMeasuredValue.IsNull() || mTemperatureMeasuredValue.Value() >= maxTemp)
        {
            mTemperatureMeasuredValue.SetNonNull(minTemp);
        }
        else
        {
            mTemperatureMeasuredValue.SetNonNull(static_cast<int16_t>(mTemperatureMeasuredValue.Value() + 100));
        }

        ChipLogProgress(AppServer, "IncreasingTemperatureValue: Increasing to %d", mTemperatureMeasuredValue.Value());
        LogErrorOnFailure(TemperatureMeasurementCluster().SetMeasuredValue(mTemperatureMeasuredValue));
    }

    VerifyOrDie(mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec) == CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
