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
#include "IncreasingMoistureSoilSensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace chip {
namespace app {

namespace {
constexpr System::Clock::Seconds16 kIncreaseMoistureIntervalSec = System::Clock::Seconds16(10);

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

const TemperatureMeasurementCluster::StartupConfiguration kDefaultTemperatureConfig = {
    .minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-10)),
    .maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(50)),
    .tolerance        = 0,
};

} // namespace

IncreasingMoistureSoilSensorDevice::IncreasingMoistureSoilSensorDevice() :
    SoilSensorDevice(mTimerDelegate, kDefaultSoilMoistureMeasurementLimits, kDefaultTemperatureConfig)
{}

IncreasingMoistureSoilSensorDevice::~IncreasingMoistureSoilSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingMoistureSoilSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                        EndpointId parentId)
{
    ReturnErrorOnFailure(SoilSensorDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to increase moisture every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec);
}

void IncreasingMoistureSoilSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    SoilSensorDevice::Unregister(provider);
}

void IncreasingMoistureSoilSensorDevice::TimerFired()
{
    if (mSoilMoistureMeasuredValue.IsNull())
    {
        mSoilMoistureMeasuredValue.SetNonNull(kDefaultSoilMoistureMeasurementLimits.minMeasuredValue);
    }
    else if (mSoilMoistureMeasuredValue.Value() >= kDefaultSoilMoistureMeasurementLimits.maxMeasuredValue)
    {
        mSoilMoistureMeasuredValue.SetNonNull(kDefaultSoilMoistureMeasurementLimits.minMeasuredValue);
    }
    else
    {
        mSoilMoistureMeasuredValue.SetNonNull(mSoilMoistureMeasuredValue.Value() + 1U);
    }

    ChipLogProgress(AppServer, "IncreasingMoistureValue: Increasing to %d", mSoilMoistureMeasuredValue.Value());
    LogErrorOnFailure(mSoilMeasurementCluster.Cluster().SetSoilMoistureMeasuredValue(mSoilMoistureMeasuredValue));

    // Simulate increasing temperature
    if (mTemperatureMeasuredValue.IsNull())
    {
        mTemperatureMeasuredValue.SetNonNull(kDefaultTemperatureConfig.minMeasuredValue.Value());
    }
    else if (mTemperatureMeasuredValue.Value() >= kDefaultTemperatureConfig.maxMeasuredValue.Value())
    {
        mTemperatureMeasuredValue.SetNonNull(kDefaultTemperatureConfig.minMeasuredValue.Value());
    }
    else
    {
        mTemperatureMeasuredValue.SetNonNull(static_cast<int16_t>(mTemperatureMeasuredValue.Value() + 1));
    }

    ChipLogProgress(AppServer, "IncreasingTemperatureValue: Increasing to %d", mTemperatureMeasuredValue.Value());
    LogErrorOnFailure(mTemperatureMeasurementCluster.Cluster().SetMeasuredValue(mTemperatureMeasuredValue));

    VerifyOrDie(mTimerDelegate.StartTimer(this, kIncreaseMoistureIntervalSec) == CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
