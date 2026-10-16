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
#include <device/api/SimulatedDevice.h>
#include <device/api/Simulator.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

SimulatedDevice::SimulatedDevice(Simulator & simulator, System::Clock::Timeout tickRate) :
    mSimulator(simulator), mTickRate(tickRate)
{
    mSimulator.Register(this);
}

SimulatedDevice::~SimulatedDevice()
{
    mSimulator.Unregister(this);
    StopSimulation();
}

void SimulatedDevice::StartSimulation()
{
    if (IsSimulationRunning())
    {
        return;
    }
    ScheduleNextTick();
}

void SimulatedDevice::StopSimulation()
{
    CancelSimulationTimer();
}

bool SimulatedDevice::IsSimulationRunning() const
{
    return mSimulator.GetTimerDelegate().IsTimerActive(const_cast<SimulatedDevice *>(this));
}

void SimulatedDevice::TimerFired()
{
    ScheduleNextTick();
    OnSimulationTick();
}

void SimulatedDevice::ScheduleNextTick()
{
    VerifyOrDie(mSimulator.GetTimerDelegate().StartTimer(this, GetTickRate()) == CHIP_NO_ERROR);
}

void SimulatedDevice::CancelSimulationTimer()
{
    mSimulator.GetTimerDelegate().CancelTimer(this);
}

} // namespace chip::app
