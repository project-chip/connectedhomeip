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

#include <device/types/air-quality-sensor/AirQualitySensor.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class SimulatedAirQualitySensor : public AirQualitySensor, public TimerContext
{
public:
    static constexpr System::Clock::Seconds16 kDefaultUpdateInterval = System::Clock::Seconds16(10);

    SimulatedAirQualitySensor(TimerDelegate & timerDelegate, const Config & config);
    explicit SimulatedAirQualitySensor(TimerDelegate & timerDelegate);
    ~SimulatedAirQualitySensor() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext
    void TimerFired() override;

private:
    uint32_t mTickCount = 0;
};

} // namespace app
} // namespace chip
