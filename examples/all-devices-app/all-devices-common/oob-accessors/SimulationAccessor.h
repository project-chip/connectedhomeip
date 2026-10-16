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
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/OOBDataSerializer.h>

namespace chip::app {

/**
 * Global Out-Of-Band accessor that routes simulation lifecycle actions
 * (PauseSimulation, ResumeSimulation) to the Simulator instance.
 */
class SimulationAccessor : public OOBAccessor
{
public:
    explicit SimulationAccessor(Simulator & simulator) : mSimulator(simulator) {}
    ~SimulationAccessor() override = default;

    std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) override
    {
        if (actionName.data_equal(OOBDataSerializer::kPauseSimulationAction))
        {
            if (mSimulator.GetSimulatedDevices().empty())
            {
                return CHIP_ERROR_NOT_FOUND;
            }
            mSimulator.StopAll();
            return CHIP_NO_ERROR;
        }
        if (actionName.data_equal(OOBDataSerializer::kResumeSimulationAction))
        {
            if (mSimulator.GetSimulatedDevices().empty())
            {
                return CHIP_ERROR_NOT_FOUND;
            }
            mSimulator.StartAll();
            return CHIP_NO_ERROR;
        }
        return std::nullopt;
    }

private:
    Simulator & mSimulator;
};

} // namespace chip::app
