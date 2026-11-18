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

constexpr chip::Percent kMoistureStepPercent   = 1;
constexpr int16_t kTemperatureStepCentiCelsius = 100; // 1.00°C
} // namespace

IncreasingMoistureSoilSensor::IncreasingMoistureSoilSensor(TimerDelegate & timerDelegate,
                                                           SoilMoistureMeasurementLimits::TypeInfo::Type moistureLimits,
                                                           TemperatureMeasurementCluster::StartupConfiguration tempConfig) :
    SoilSensor(timerDelegate, moistureLimits, tempConfig)
{}

IncreasingMoistureSoilSensor::~IncreasingMoistureSoilSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingMoistureSoilSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    ReturnErrorOnFailure(SoilSensor::Register(endpoint, provider, composition));

    CHIP_ERROR err = mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec);
    if (err != CHIP_NO_ERROR)
    {
        SoilSensor::Unregister(provider);
        return err;
    }

    return CHIP_NO_ERROR;
}

void IncreasingMoistureSoilSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    SoilSensor::Unregister(provider);
}

void IncreasingMoistureSoilSensor::TimerFired()
{
    auto & soilCluster = SoilMeasurementCluster();
    auto moisture      = soilCluster.GetSoilMoistureMeasuredValue();

    const chip::Percent minMoisture = static_cast<chip::Percent>(mMoistureLimits.minMeasuredValue);
    const chip::Percent maxMoisture = static_cast<chip::Percent>(mMoistureLimits.maxMeasuredValue);

    if (moisture.IsNull() || static_cast<uint32_t>(moisture.Value()) + kMoistureStepPercent > maxMoisture)
    {
        moisture.SetNonNull(minMoisture);
    }
    else
    {
        moisture.SetNonNull(static_cast<chip::Percent>(moisture.Value() + kMoistureStepPercent));
    }

    ChipLogProgress(AppServer, "IncreasingMoistureValue: Increasing to %d", moisture.Value());
    LogErrorOnFailure(soilCluster.SetSoilMoistureMeasuredValue(moisture));

    const int16_t minTemp =
        mTempConfig.minMeasuredValue.ValueOr(!mTempConfig.maxMeasuredValue.IsNull() && mTempConfig.maxMeasuredValue.Value() < -1000
                                                 ? mTempConfig.maxMeasuredValue.Value()
                                                 : -1000);
    const int16_t maxTemp = mTempConfig.maxMeasuredValue.ValueOr(INT16_MAX);

    auto & tempCluster = TemperatureMeasurementCluster();
    auto temperature   = tempCluster.GetMeasuredValue();

    if (temperature.IsNull() || static_cast<int32_t>(temperature.Value()) + kTemperatureStepCentiCelsius > maxTemp)
    {
        temperature.SetNonNull(minTemp);
    }
    else
    {
        temperature.SetNonNull(static_cast<int16_t>(temperature.Value() + kTemperatureStepCentiCelsius));
    }

    ChipLogProgress(AppServer, "IncreasingTemperatureValue: Increasing to %d", temperature.Value());
    LogErrorOnFailure(tempCluster.SetMeasuredValue(temperature));

    VerifyOrDie(mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec) == CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
