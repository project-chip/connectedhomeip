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
#include "LoggingOccupancySensorDevice.h"
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

LoggingOccupancySensorDevice::LoggingOccupancySensorDevice(TimerDelegate & timerDelegate) :
    OccupancySensorDevice(
        // Initialize with kInvalidEndpointId. The actual endpoint ID will be set
        // when Register() is called by the application with a valid endpoint ID.
        OccupancySensingCluster::Config(kInvalidEndpointId)
            .WithFeatures(BitFlags(OccupancySensing::Feature::kPassiveInfrared))
            .WithHoldTime(10,
                          {
                              .holdTimeMin     = 1,
                              .holdTimeMax     = 300,
                              .holdTimeDefault = 10,
                          },
                          timerDelegate)
            .WithDelegate(this),
        timerDelegate)
{}

CHIP_ERROR LoggingOccupancySensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    return OccupancySensorDevice::Register(endpoint, provider, parentId);
}

void LoggingOccupancySensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    OccupancySensorDevice::Unregister(provider);
}

void LoggingOccupancySensorDevice::OnOccupancyChanged(bool occupied)
{
    ChipLogProgress(AppServer, "LoggingOccupancySensorDevice::OnOccupancyChanged: %s", occupied ? "Occupied" : "Unoccupied");
}

void LoggingOccupancySensorDevice::OnHoldTimeChanged(uint16_t holdTime)
{
    ChipLogProgress(AppServer, "LoggingOccupancySensorDevice::OnHoldTimeChanged: %u", holdTime);
}

} // namespace app
} // namespace chip
