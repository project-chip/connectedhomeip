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

#include <EnergyEvseDelegateImpl.h>
#include <app/clusters/energy-evse-server/energy-evse-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class EnergyEvseManager : public Instance
{
public:
    EnergyEvseManager(EndpointId aEndpointId, EnergyEvseDelegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
                      OptionalCommands aOptionalCmds) :
        EnergyEvse::Instance(aEndpointId, aDelegate, aFeature, aOptionalAttrs, aOptionalCmds)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    EnergyEvseManager(const EnergyEvseManager &)             = delete;
    EnergyEvseManager(const EnergyEvseManager &&)            = delete;
    EnergyEvseManager & operator=(const EnergyEvseManager &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    CHIP_ERROR LoadPersistentAttributes();

    EnergyEvseDelegate * GetDelegate() { return mDelegate; };

private:
    EnergyEvseDelegate * mDelegate;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
