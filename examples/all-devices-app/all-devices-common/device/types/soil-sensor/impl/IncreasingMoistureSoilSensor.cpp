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

constexpr chip::Percent kMinMoisturePercent  = 0;
constexpr chip::Percent kMaxMoisturePercent  = 100;
constexpr chip::Percent kMoistureStepPercent = 1;

constexpr int16_t kMinTemperatureCentiCelsius  = -1000; // -10.00°C
constexpr int16_t kMaxTemperatureCentiCelsius  = 5000;  // +50.00°C
constexpr int16_t kTemperatureStepCentiCelsius = 100;   // 1.00°C

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin   = kMinMoisturePercent,
      .rangeMax   = kMaxMoisturePercent,
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = kMinMoisturePercent,
    .maxMeasuredValue = kMaxMoisturePercent,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTemperatureConfig = {
    .minMeasuredValue = DataModel::MakeNullable(kMinTemperatureCentiCelsius),
    .maxMeasuredValue = DataModel::MakeNullable(kMaxTemperatureCentiCelsius),
    .tolerance        = 0,
};

} // namespace

IncreasingMoistureSoilSensor::IncreasingMoistureSoilSensor() :
    SoilSensor(mTimerDelegate, kDefaultSoilMoistureMeasurementLimits, kDefaultTemperatureConfig)
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

    if (moisture.IsNull() || moisture.Value() + kMoistureStepPercent > kMaxMoisturePercent)
    {
        moisture.SetNonNull(kMinMoisturePercent);
    }
    else
    {
        moisture.SetNonNull(static_cast<chip::Percent>(moisture.Value() + kMoistureStepPercent));
    }

    ChipLogProgress(AppServer, "IncreasingMoistureValue: Increasing to %d", moisture.Value());
    LogErrorOnFailure(soilCluster.SetSoilMoistureMeasuredValue(moisture));

    auto & tempCluster = TemperatureMeasurementCluster();
    auto temperature   = tempCluster.GetMeasuredValue();

    if (temperature.IsNull() || temperature.Value() + kTemperatureStepCentiCelsius > kMaxTemperatureCentiCelsius)
    {
        temperature.SetNonNull(kMinTemperatureCentiCelsius);
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
