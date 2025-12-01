/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/unit-localization-server/unit-localization-server.h>
#include <app/static-cluster-config/UnitLocalization.h>
#include <app/util/attribute-metadata.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

#include <app/persistence/AttributePersistenceMigration.h>
#include <app/persistence/AttributePersistenceProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

namespace {

LazyRegisteredServerCluster<UnitLocalizationCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {

        const BitFlags<UnitLocalization::Feature> featureMap(rawFeatureMap);

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

} // namespace

void MatterUnitLocalizationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = UnitLocalization::Id,
            .fixedClusterInstanceCount = UnitLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterUnitLocalizationClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = UnitLocalization::Id,
            .fixedClusterInstanceCount = UnitLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate);
}

void MatterUnitLocalizationPluginServerInitCallback() {}
