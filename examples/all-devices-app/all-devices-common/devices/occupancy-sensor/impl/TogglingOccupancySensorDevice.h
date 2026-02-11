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
#pragma once

#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/occupancy-sensor/OccupancySensorDevice.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip {
namespace app {

/**
 * @brief An implementation of an Occupancy Sensor Device.
 *
 * This class serves as a simple example of an occupancy sensor. It emulates
 * occupancy state changes by toggling between "Occupied" and "Unoccupied"
 * states every 30 seconds using a timer.
 */
class TogglingOccupancySensorDevice : public OccupancySensorDevice, public Clusters::OccupancySensingDelegate, public TimerContext
{
public:
    TogglingOccupancySensorDevice();
    ~TogglingOccupancySensorDevice() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

    // OccupancySensingDelegate
    void OnOccupancyChanged(bool occupied) override;
    void OnHoldTimeChanged(uint16_t holdTime) override;

    // TimerContext
    void TimerFired() override;

private:
    DefaultTimerDelegate mTimerDelegate;
};

} // namespace app
} // namespace chip
