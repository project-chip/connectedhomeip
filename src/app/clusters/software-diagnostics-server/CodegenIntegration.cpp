/*
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
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsCluster.h>
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsLogic.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/SoftwareDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;
using namespace chip::app::Clusters::SoftwareDiagnostics::Attributes;

// for fixed endpoint, this file is ever only included IF software diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<SoftwareDiagnosticsServerCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(SoftwareDiagnosticsLogic::OptionalAttributeSet(optionalAttributeBits));
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterSoftwareDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = SoftwareDiagnostics::Id,
            .fixedClusterInstanceCount = SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterSoftwareDiagnosticsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = SoftwareDiagnostics::Id,
            .fixedClusterInstanceCount = SoftwareDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate, shutdownType);
}

void MatterSoftwareDiagnosticsPluginServerInitCallback() {}

void MatterSoftwareDiagnosticsPluginServerShutdownCallback() {}
