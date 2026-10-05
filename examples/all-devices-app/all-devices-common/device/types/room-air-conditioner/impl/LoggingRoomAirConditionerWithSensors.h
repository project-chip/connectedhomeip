/*
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

#include <device/types/room-air-conditioner/impl/LoggingRoomAirConditioner.h>
#include <device/types/temperature-sensor/TemperatureSensor.h>

namespace chip::app {

/// A composite simulator with independently available indoor and outdoor temperature readings.
class LoggingRoomAirConditionerWithSensors : public LoggingRoomAirConditioner
{
public:
    LoggingRoomAirConditionerWithSensors(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                         std::optional<EndpointComposition::SemanticTag> tag = std::nullopt);
    ~LoggingRoomAirConditionerWithSensors() override = default;

    TemperatureSensor & IndoorSensor() { return mIndoorSensor; }
    TemperatureSensor & OutdoorSensor() { return mOutdoorSensor; }

protected:
    CHIP_ERROR RegisterAdditionalEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    TemperatureSensor mIndoorSensor;
    TemperatureSensor mOutdoorSensor;
};

} // namespace chip::app
