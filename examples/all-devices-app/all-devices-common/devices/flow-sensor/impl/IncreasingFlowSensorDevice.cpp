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
#include "IncreasingFlowSensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseFlowIntervalSec = System::Clock::Seconds16(10);

const FlowMeasurementCluster::Config kDefaultFlowConfig = []() {
    FlowMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable<uint16_t>(0);
    config.maxMeasuredValue = DataModel::MakeNullable<uint16_t>(10000);
    config.WithTolerance(10);
    return config;
}();

} // namespace

IncreasingFlowSensorDevice::IncreasingFlowSensorDevice(TimerDelegate & timerDelegate) :
    FlowSensorDevice(timerDelegate, kDefaultFlowConfig)
{}

IncreasingFlowSensorDevice::~IncreasingFlowSensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingFlowSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(FlowSensorDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to increase flow every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseFlowIntervalSec);
}

void IncreasingFlowSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    FlowSensorDevice::Unregister(provider);
}

void IncreasingFlowSensorDevice::TimerFired()
{
    // Simulate increasing flow
    if (mFlowMeasuredValue.IsNull())
    {
        mFlowMeasuredValue.SetNonNull(kDefaultFlowConfig.minMeasuredValue.Value());
    }
    else if (mFlowMeasuredValue.Value() >= kDefaultFlowConfig.maxMeasuredValue.Value())
    {
        mFlowMeasuredValue.SetNonNull(kDefaultFlowConfig.minMeasuredValue.Value());
    }
    else
    {
        mFlowMeasuredValue.SetNonNull(static_cast<uint16_t>(mFlowMeasuredValue.Value() + 100));
    }

    ChipLogProgress(AppServer, "IncreasingFlowValue: Increasing to %d", mFlowMeasuredValue.Value());
    LogErrorOnFailure(mFlowMeasurementCluster.Cluster().SetMeasuredValue(mFlowMeasuredValue));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseFlowIntervalSec));
}

} // namespace app
} // namespace chip
