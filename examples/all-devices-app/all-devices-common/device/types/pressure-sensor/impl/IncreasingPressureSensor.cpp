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
#include "IncreasingPressureSensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreasePressureIntervalSec = System::Clock::Seconds16(10);
constexpr int16_t kPressureStepValue                            = 50;

int16_t GetEffectiveMinPressure(const PressureMeasurementCluster::Config & config)
{
    return config.minMeasuredValue.ValueOr(
        !config.maxMeasuredValue.IsNull() && config.maxMeasuredValue.Value() < 0 ? config.maxMeasuredValue.Value() : 0);
}

} // namespace

IncreasingPressureSensor::IncreasingPressureSensor(TimerDelegate & timerDelegate,
                                                   PressureMeasurementCluster::Config pressureConfig) :
    PressureSensor(timerDelegate, pressureConfig)
{}

IncreasingPressureSensor::~IncreasingPressureSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingPressureSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    ReturnErrorOnFailure(PressureSensor::Register(endpoint, provider, composition));

    const int16_t initialPressure = GetEffectiveMinPressure(mPressureConfig);
    mPressureMeasuredValue        = initialPressure;
    ReturnErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mPressureMeasuredValue)));

    // Kick off the timer loop to increase pressure every few seconds
    return mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec);
}

void IncreasingPressureSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    PressureSensor::Unregister(provider);
}

void IncreasingPressureSensor::TimerFired()
{
    const int16_t minPressure  = GetEffectiveMinPressure(mPressureConfig);
    const int16_t maxPressure  = mPressureConfig.maxMeasuredValue.ValueOr(INT16_MAX);
    const int32_t nextPressure = static_cast<int32_t>(mPressureMeasuredValue) + kPressureStepValue;

    if (nextPressure > maxPressure)
    {
        mPressureMeasuredValue = minPressure;
    }
    else
    {
        mPressureMeasuredValue = static_cast<int16_t>(nextPressure);
    }

    ChipLogProgress(AppServer, "IncreasingPressureValue: Increasing to %d", static_cast<int>(mPressureMeasuredValue));
    LogErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mPressureMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec));
}

} // namespace app
} // namespace chip
