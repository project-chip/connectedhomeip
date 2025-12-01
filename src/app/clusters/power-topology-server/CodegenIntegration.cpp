/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/power-topology-server/CodegenIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

CHIP_ERROR Instance::Init()
{
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

void Instance::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister Power Topology cluster");
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mCluster.Cluster().Features().Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    switch (aOptionalAttrs)
    {
    case OptionalAttributes::kOptionalAttributeAvailableEndpoints:
        return mCluster.Cluster().OptionalAttributes().IsSet(Attributes::AvailableEndpoints::Id);
    case OptionalAttributes::kOptionalAttributeActiveEndpoints:
        return mCluster.Cluster().OptionalAttributes().IsSet(Attributes::ActiveEndpoints::Id);
    default:
        return false;
    }
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterPowerTopologyClusterInitCallback(chip::EndpointId endpoint) {}

void MatterPowerTopologyClusterShutdownCallback(chip::EndpointId endpoint) {}
