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
constexpr int16_t kPressureStepTenthsKilopascal                 = 50;

int16_t GetEffectiveMinPressure(DataModel::Nullable<int16_t> minPressure, DataModel::Nullable<int16_t> maxPressure)
{
    return minPressure.ValueOr(!maxPressure.IsNull() && maxPressure.Value() < 0 ? maxPressure.Value() : 0);
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

    const int16_t initialPressure = GetEffectiveMinPressure(mPressureConfig.minMeasuredValue, mPressureConfig.maxMeasuredValue);
    ReturnErrorOnFailure(mPressureMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(initialPressure)));

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
    auto & cluster = mPressureMeasurementCluster.Cluster();

    const DataModel::Nullable<int16_t> maxPressureAttr = cluster.GetMaxMeasuredValue();
    const int16_t minPressure                          = GetEffectiveMinPressure(cluster.GetMinMeasuredValue(), maxPressureAttr);
    const int16_t maxPressure                          = maxPressureAttr.ValueOr(INT16_MAX);

    const DataModel::Nullable<int16_t> currentPressure = cluster.GetMeasuredValue();
    int16_t nextPressureValue                          = minPressure;
    if (!currentPressure.IsNull() && currentPressure.Value() >= minPressure &&
        static_cast<int32_t>(currentPressure.Value()) + kPressureStepTenthsKilopascal <= maxPressure)
    {
        nextPressureValue = static_cast<int16_t>(currentPressure.Value() + kPressureStepTenthsKilopascal);
    }

    ChipLogProgress(AppServer, "IncreasingPressureValue: Increasing to %d", static_cast<int>(nextPressureValue));
    LogErrorOnFailure(cluster.SetMeasuredValue(DataModel::MakeNullable(nextPressureValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreasePressureIntervalSec));
}

} // namespace app
} // namespace chip
