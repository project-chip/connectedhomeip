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

#include <devices/power-source/BatteryPowerSourceDevice.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app {

/**
 * Simulated battery power source that decreases the reported battery level over time.
 */
class DecreasingBatteryPowerSourceDevice : public BatteryPowerSourceDevice, public TimerContext
{
public:
    DecreasingBatteryPowerSourceDevice();
    ~DecreasingBatteryPowerSourceDevice() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TimerContext
    void TimerFired() override;

private:
    DefaultTimerDelegate mTimerDelegate;
};

} // namespace chip::app
