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
#pragma once

#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/temperature-sensor/TemperatureSensorDevice.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip {
namespace app {

/**
 * @brief An implementation of a Temperature Sensor Device that increases temperature over time.
 *
 */
class IncreasingTemperatureSensorDevice : public TemperatureSensorDevice, public TimerContext
{
public:
    IncreasingTemperatureSensorDevice();
    ~IncreasingTemperatureSensorDevice() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext
    void TimerFired() override;

private:
    DefaultTimerDelegate mTimerDelegate;
    DataModel::Nullable<int16_t> mTemperatureMeasuredValue;
};

} // namespace app
} // namespace chip
