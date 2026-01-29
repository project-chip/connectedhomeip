/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/LinkedList.h>
namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
             OptionalCommands aOptionalCmds);

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;
    bool SupportsOptCmd(OptionalCommands aOptionalCmds) const;

private:
    RegisteredServerCluster<EnergyEvseCluster> mCluster;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
