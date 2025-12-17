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
#include "OccupancySensorDeviceImpl.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
constexpr uint16_t kOccupancyStateChangeInterval = 30; // seconds
} // namespace

OccupancySensorDeviceImpl::OccupancySensorDeviceImpl() :
    OccupancySensorDevice(
        // Initialize with kInvalidEndpointId. The actual endpoint ID will be set
        // when Register() is called by the application with a valid endpoint ID.
        chip::app::Clusters::OccupancySensingCluster::Config(kInvalidEndpointId)
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
{
    // Kick off the timer loop to flip occupancy every few seconds
    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds16(kOccupancyStateChangeInterval)));
}

OccupancySensorDeviceImpl::~OccupancySensorDeviceImpl()
{
    mTimerDelegate.CancelTimer(this);
}

void OccupancySensorDeviceImpl::OnOccupancyChanged(bool occupied)
{
    ChipLogProgress(AppServer, "OccupancySensorDeviceImpl::OnOccupancyChanged: %s", occupied ? "Occupied" : "Unoccupied");
}

void OccupancySensorDeviceImpl::OnHoldTimeChanged(uint16_t holdTime)
{
    ChipLogProgress(AppServer, "OccupancySensorDeviceImpl::OnHoldTimeChanged: %u", holdTime);
}

void OccupancySensorDeviceImpl::TimerFired()
{
    // Flips the occupancy state every kOccupancyStateChangeInterval seconds
     
    // Only interact with the cluster if it has been constructed/registered
    if (mOccupancySensingCluster.IsConstructed())
    {
        bool nextState = !mOccupancySensingCluster.Cluster().IsOccupied();

        ChipLogProgress(AppServer, "OccupancySensorDeviceImpl: Toggling occupancy to %s", nextState ? "Occupied" : "Unoccupied");
        mOccupancySensingCluster.Cluster().SetOccupancy(nextState);
    }

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds16(kOccupancyStateChangeInterval)));
}
