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
#include "IncreasingFlowSensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseFlowIntervalSec = System::Clock::Seconds16(10);
constexpr uint16_t kFlowStepValue                           = 100;

} // namespace

IncreasingFlowSensor::IncreasingFlowSensor(TimerDelegate & timerDelegate, FlowMeasurementCluster::Config flowConfig) :
    FlowSensor(timerDelegate, flowConfig)
{}

IncreasingFlowSensor::~IncreasingFlowSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingFlowSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    ReturnErrorOnFailure(FlowSensor::Register(endpoint, provider, composition));

    // Initialize with the minimum configured value
    mFlowMeasuredValue = mFlowConfig.minMeasuredValue.ValueOr(0);
    ReturnErrorOnFailure(mFlowMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mFlowMeasuredValue)));

    // Kick off the timer loop to increase flow every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseFlowIntervalSec);
}

void IncreasingFlowSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    FlowSensor::Unregister(provider);
}

void IncreasingFlowSensor::TimerFired()
{
    const uint16_t minFlow  = mFlowConfig.minMeasuredValue.ValueOr(0);
    const uint16_t maxFlow  = mFlowConfig.maxMeasuredValue.ValueOr(static_cast<uint16_t>(UINT16_MAX - 1));
    const uint32_t nextFlow = static_cast<uint32_t>(mFlowMeasuredValue) + kFlowStepValue;

    if (nextFlow > maxFlow)
    {
        mFlowMeasuredValue = minFlow;
    }
    else
    {
        mFlowMeasuredValue = static_cast<uint16_t>(nextFlow);
    }

    ChipLogProgress(AppServer, "IncreasingFlowValue: Increasing to %u", static_cast<unsigned int>(mFlowMeasuredValue));
    LogErrorOnFailure(mFlowMeasurementCluster.Cluster().SetMeasuredValue(DataModel::MakeNullable(mFlowMeasuredValue)));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseFlowIntervalSec));
}

} // namespace app
} // namespace chip
