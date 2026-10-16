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

#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessorRegistry.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <pigweed/rpc_services/Simulation.h>

namespace chip::app {

class PigweedSimulationDelegate : public chip::rpc::SimulationDelegate
{
public:
    CHIP_ERROR PauseSimulation() override
    {
        CHIP_ERROR err = OOBAccessorRegistry::Instance().HandleAction(OOBDataSerializer::kPauseSimulationAction, {});
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        return err;
    }

    CHIP_ERROR ResumeSimulation() override
    {
        CHIP_ERROR err = OOBAccessorRegistry::Instance().HandleAction(OOBDataSerializer::kResumeSimulationAction, {});
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        return err;
    }
};

} // namespace chip::app
