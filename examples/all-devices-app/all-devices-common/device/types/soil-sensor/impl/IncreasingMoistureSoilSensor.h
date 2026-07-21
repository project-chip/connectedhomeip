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

#include <app/clusters/soil-measurement-server/SoilMeasurementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/soil-sensor/SoilSensor.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip {
namespace app {

/**
 * Simulated soil sensor that increases the reported moisture level over time.
 */
class IncreasingMoistureSoilSensor : public SoilSensor, public TimerContext
{
public:
    IncreasingMoistureSoilSensor();
    ~IncreasingMoistureSoilSensor() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext
    void TimerFired() override;

private:
    DefaultTimerDelegate mTimerDelegate;
    Clusters::SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type mSoilMoistureMeasuredValue;
    DataModel::Nullable<int16_t> mTemperatureMeasuredValue;
};

} // namespace app
} // namespace chip
