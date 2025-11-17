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
#include <app/static-cluster-config/OperationalCredentials.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

static_assert((OperationalCredentials::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               OperationalCredentials::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              (OperationalCredentials::StaticApplicationConfig::kFixedClusterConfig.size() == 0));

namespace {

LazyRegisteredServerCluster<OperationalCredentialsCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        OperationalCredentialsCluster::Context context = { .fabricTable     = Server::GetInstance().GetFabricTable(),
                                                           .failSafeContext = Server::GetInstance().GetFailSafeContext(),
                                                           .sessionManager  = Server::GetInstance().GetSecureSessionManager(),
                                                           .dnssdServer     = app::DnssdServer::Instance(),
                                                           .commissioningWindowManager =
                                                               Server::GetInstance().GetCommissioningWindowManager() };
        gServer.Create(endpointId, context);
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned emberEndpointIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterOperationalCredentialsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OperationalCredentials::Id,
            .fixedClusterInstanceCount = OperationalCredentials::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterOperationalCredentialsClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OperationalCredentials::Id,
            .fixedClusterInstanceCount = OperationalCredentials::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1,
        },
        integrationDelegate);
}

void MatterOperationalCredentialsPluginServerInitCallback() {}
