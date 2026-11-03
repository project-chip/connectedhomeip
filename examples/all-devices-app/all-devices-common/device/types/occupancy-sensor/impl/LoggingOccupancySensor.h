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
#include <device/types/occupancy-sensor/OccupancySensor.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * Concrete implementation of OccupancySensor that logs occupancy and hold time transitions.
 */
class LoggingOccupancySensor : public OccupancySensor, public Clusters::OccupancySensingDelegate
{
public:
    LoggingOccupancySensor(TimerDelegate & timerDelegate);
    ~LoggingOccupancySensor() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // OccupancySensingDelegate
    void OnOccupancyChanged(bool occupied) override;
    void OnHoldTimeChanged(uint16_t holdTime) override;
};

} // namespace app
} // namespace chip
