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

#include <device/api/Simulator.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

namespace chip::app {

/**
 * Base class for autonomous simulated devices that execute periodic background simulation loops.
 *
 * Automatically registers with the injected Simulator on construction and unregisters on destruction.
 * Inherits from TimerContext and provides default periodic timer scheduling driven by GetTickRate().
 *
 * Simulated devices are assumed to be continuously available. Calls to StopSimulation() suspend periodic
 * timer execution (acting as a pause) rather than tearing down the device. Permanent teardown occurs on destruction.
 */
class SimulatedDevice : public TimerContext
{
public:
    SimulatedDevice(Simulator & simulator, System::Clock::Timeout tickRate);
    virtual ~SimulatedDevice();

    System::Clock::Timeout GetTickRate() const { return mTickRate; }

    /// Sets the simulation tick rate. The new rate takes effect on the next scheduled tick.
    void SetTickRate(System::Clock::Timeout tickRate) { mTickRate = tickRate; }

    /// Starts or resumes simulation if not already running.
    virtual void StartSimulation();

    /// Stops or pauses simulation.
    virtual void StopSimulation();

    bool IsSimulationRunning() const;
    bool IsSimulationPaused() const { return !IsSimulationRunning(); }

    /// TimerContext: fires OnSimulationTick() and reschedules the timer.
    void TimerFired() override;

    /// Subclasses implement their attribute progression logic here.
    virtual void OnSimulationTick() = 0;

    Simulator & GetSimulator() { return mSimulator; }

protected:
    virtual void ScheduleNextTick();
    virtual void CancelSimulationTimer();

private:
    Simulator & mSimulator;
    System::Clock::Timeout mTickRate;
};

} // namespace chip::app
