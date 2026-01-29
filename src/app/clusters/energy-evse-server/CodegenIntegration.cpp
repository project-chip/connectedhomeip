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

#include <app/clusters/energy-evse-server/CodegenIntegration.h>

#include <app/util/generic-callbacks.h>
#include <clusters/EnergyEvse/Metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

Instance::Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
                   OptionalCommands aOptionalCmds) :
    mCluster(EnergyEvseCluster::Config(aEndpointId, aDelegate, aFeature, aOptionalAttrs, aOptionalCmds))
{
    aDelegate.SetInstance(this);
}

CHIP_ERROR Instance::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(EnergyEvse::Id), err.Format());
    }
    return err;
}

void Instance::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(EnergyEvse::Id), err.Format());
    }
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mCluster.Cluster().Features().Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mCluster.Cluster().OptionalAttrs().Has(aOptionalAttrs);
}

bool Instance::SupportsOptCmd(OptionalCommands aOptionalCmds) const
{
    return mCluster.Cluster().OptionalCmds().Has(aOptionalCmds);
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip

// The current implementation already manually instantiates and initializes the cluster, so no need for the codegen integration.
void MatterEnergyEvseClusterInitCallback(chip::EndpointId) {}
void MatterEnergyEvseClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}

// Legacy callback stubs
void MatterEnergyEvsePluginServerInitCallback() {}
void MatterEnergyEvsePluginServerShutdownCallback() {}
