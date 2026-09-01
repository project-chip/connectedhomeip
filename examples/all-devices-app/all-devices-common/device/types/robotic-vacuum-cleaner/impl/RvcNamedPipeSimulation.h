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

#include <app/util/basic-types.h>
#include <string>

namespace chip::app {

// Named-pipe simulation hooks for RVC devices. Implemented by the operational-state cluster
// delegate when the device provides a full simulation (see SimulatedRoboticVacuumCleaner).
class RvcNamedPipeSimulation
{
public:
    virtual ~RvcNamedPipeSimulation() = default;

    virtual void HandleCharged()                             = 0;
    virtual void HandleCharging()                            = 0;
    virtual void HandleDocked()                              = 0;
    virtual void HandleChargerFound()                        = 0;
    virtual void HandleLowCharge()                           = 0;
    virtual void HandleActivityComplete()                    = 0;
    virtual void HandleAreaComplete()                        = 0;
    virtual void HandleClearError()                          = 0;
    virtual void HandleErrorEvent(const std::string & error) = 0;
};

void RegisterRvcNamedPipeSimulation(EndpointId endpoint, RvcNamedPipeSimulation * simulation);
void UnregisterRvcNamedPipeSimulation(EndpointId endpoint);
RvcNamedPipeSimulation * GetRvcNamedPipeSimulation(EndpointId endpoint);

} // namespace chip::app
