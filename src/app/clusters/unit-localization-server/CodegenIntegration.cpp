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

#include <app/clusters/unit-localization-server/CodegenIntegration.h>
#include <app/clusters/unit-localization-server/UnitLocalizationCluster.h>
#include <app/static-cluster-config/UnitLocalization.h>
#include <clusters/UnitLocalization/Ids.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;

LazyRegisteredServerCluster<UnitLocalizationServer> gServer;

UnitLocalizationServer & UnitLocalizationServer::Instance()
{
    VerifyOrDie(gServer.IsConstructed());
    return gServer.Cluster();
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<UnitLocalization::Feature> featureMap{ rawFeatureMap };
        gServer.Create(endpointId, featureMap);
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

void MatterUnitLocalizationClusterInitCallback(chip::EndpointId endpointId)
{
    // This cluster should only exist in Root endpoint.
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = UnitLocalization::Id,
            .fixedClusterInstanceCount = UnitLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterUnitLocalizationClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    // This cluster should only exist in Root endpoint.
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = UnitLocalization::Id,
            .fixedClusterInstanceCount = UnitLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1,
        },
        integrationDelegate, shutdownType);
}

void MatterUnitLocalizationPluginServerInitCallback() {}
