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
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

LazyRegisteredServerCluster<OperationalCredentialsCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId);
        return gServer.Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override { return gServer.Cluster(); }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServer.Destroy(); }
};

} // namespace

void emberAfOperationalCredentialsClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OperationalCredentials::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterOperationalCredentialsClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OperationalCredentials::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
        },
        integrationDelegate);
}

void MatterOperationalCredentialsPluginServerInitCallback() {}