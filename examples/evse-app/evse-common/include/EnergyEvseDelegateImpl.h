/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
 *    All rights reserved.
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

#include <EvseSimulationDelegate.h>
#include <lib/support/TimerDelegate.h>
#include <app/clusters/energy-evse-server/CodegenIntegration.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/**
 * evse-app delegate: shared simulation logic with codegen Instance wiring.
 */
class EnergyEvseDelegate : public EvseSimulationDelegate
{
public:
    EnergyEvseDelegate(EvseTargetsDelegate & aDelegate, TimerDelegate & timerDelegate) :
        EvseSimulationDelegate(aDelegate, timerDelegate)
    {}

    void SetInstance(Instance * aInstance);
    Instance * GetInstance() { return mInstance; }

private:
    Instance * mInstance = nullptr;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
