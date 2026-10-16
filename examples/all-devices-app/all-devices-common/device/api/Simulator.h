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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>

#include <vector>

namespace chip::app {

class SimulatedDevice;

/**
 * Manages autonomous simulated devices and coordinates global simulation lifecycle
 * (pause, resume) across all registered devices.
 */
class Simulator
{
public:
    explicit Simulator(TimerDelegate & timerDelegate) : mTimerDelegate(timerDelegate) {}
    ~Simulator() = default;

    TimerDelegate & GetTimerDelegate() { return mTimerDelegate; }

    void Register(SimulatedDevice * device);
    void Unregister(SimulatedDevice * device);

    void StartAll();
    void StopAll();

    bool IsAnySimulationRunning() const;

    Span<SimulatedDevice * const> GetSimulatedDevices() const
    {
        return Span<SimulatedDevice * const>(mDevices.data(), mDevices.size());
    }

private:
    TimerDelegate & mTimerDelegate;
    std::vector<SimulatedDevice *> mDevices;
};

} // namespace chip::app
