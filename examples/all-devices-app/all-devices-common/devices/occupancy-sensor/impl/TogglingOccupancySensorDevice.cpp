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
#include "TogglingOccupancySensorDevice.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
constexpr uint16_t kOccupancyStateChangeIntervalSec = 30;
} // namespace

TogglingOccupancySensorDevice::TogglingOccupancySensorDevice() :
    OccupancySensorDevice(
        // Initialize with kInvalidEndpointId. The actual endpoint ID will be set
        // when Register() is called by the application with a valid endpoint ID.
        OccupancySensingCluster::Config(kInvalidEndpointId)
            .WithFeatures(OccupancySensing::Feature::kPassiveInfrared)
            .WithHoldTime(10,
                          {
                              .holdTimeMin     = 1,
                              .holdTimeMax     = 300,
                              .holdTimeDefault = 10,
                          },
                          mTimerDelegate)
            .WithDelegate(this),
        mTimerDelegate)
{}

TogglingOccupancySensorDevice::~TogglingOccupancySensorDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR TogglingOccupancySensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(OccupancySensorDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to flip occupancy every few seconds
    return mTimerDelegate.StartTimer(this, System::Clock::Seconds16(kOccupancyStateChangeIntervalSec));
}

void TogglingOccupancySensorDevice::UnRegister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    OccupancySensorDevice::UnRegister(provider);
}

void TogglingOccupancySensorDevice::OnOccupancyChanged(bool occupied)
{
    ChipLogProgress(AppServer, "TogglingOccupancySensorDevice::OnOccupancyChanged: %s", occupied ? "Occupied" : "Unoccupied");
}

void TogglingOccupancySensorDevice::OnHoldTimeChanged(uint16_t holdTime)
{
    ChipLogProgress(AppServer, "TogglingOccupancySensorDevice::OnHoldTimeChanged: %u", holdTime);
}

void TogglingOccupancySensorDevice::TimerFired()
{
    // Flips the occupancy state every kOccupancyStateChangeIntervalSec seconds

    bool nextState = !mOccupancySensingCluster.Cluster().IsOccupied();

    ChipLogProgress(AppServer, "TogglingOccupancySensorDevice: Toggling occupancy to %s", nextState ? "Occupied" : "Unoccupied");
    mOccupancySensingCluster.Cluster().SetOccupancy(nextState);

    VerifyOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds16(kOccupancyStateChangeIntervalSec)) == CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
