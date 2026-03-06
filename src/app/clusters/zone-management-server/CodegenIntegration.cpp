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

#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

constexpr size_t kZoneManagementFixedClusterCount = MATTER_DM_ZONE_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
constexpr size_t kZoneManagementMaxClusterCount = kZoneManagementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

struct EndpointConfiguration
{
    EndpointId endpointId = kInvalidEndpointId;
    Delegate * delegate   = nullptr;
    BitFlags<Feature> features;
    ZoneManagementCluster::Context::Config config = {};
};

EndpointConfiguration gEndpointConfigurations[kZoneManagementMaxClusterCount];
LazyRegisteredServerCluster<ZoneManagementCluster> gServers[kZoneManagementMaxClusterCount];

EndpointConfiguration * FindConfigurationByEndpoint(EndpointId endpointId)
{
    for (auto & configuration : gEndpointConfigurations)
    {
        if (configuration.endpointId == endpointId)
        {
            return &configuration;
        }
    }

    return nullptr;
}

void SetContext(EndpointId endpointId, Delegate & delegate, BitFlags<Feature> features, uint8_t maxUserDefinedZones,
                uint8_t maxZones, uint8_t sensitivityMax, const TwoDCartesianVertexStruct & twoDCartesianMax)
{
    EndpointConfiguration * configuration = FindConfigurationByEndpoint(endpointId);
    if (configuration == nullptr)
    {
        for (auto & candidate : gEndpointConfigurations)
        {
            if (candidate.endpointId == kInvalidEndpointId)
            {
                configuration = &candidate;
                configuration->endpointId = endpointId;
                break;
            }
        }
    }

    VerifyOrReturn(configuration != nullptr);

    configuration->delegate  = &delegate;
    configuration->features  = features;
    configuration->config    = {
        .maxUserDefinedZones = maxUserDefinedZones,
        .maxZones            = maxZones,
        .sensitivityMax      = sensitivityMax,
        .twoDCartesianMax    = twoDCartesianMax,
    };
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        static_cast<void>(optionalAttributeBits);
        static_cast<void>(featureMap);

        auto * configuration = FindConfigurationByEndpoint(endpointId);
        VerifyOrDie(configuration != nullptr);
        VerifyOrDie(configuration->delegate != nullptr);

        gServers[clusterInstanceIndex].Create(ZoneManagementCluster::Context{
            .delegate   = *configuration->delegate,
            .endpointId = endpointId,
            .features   = configuration->features,
            .config     = configuration->config,
        });

        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterZoneManagementClusterInitCallback(chip::EndpointId endpointId)
{
    auto * configuration = FindConfigurationByEndpoint(endpointId);
    if ((configuration == nullptr) || (configuration->delegate == nullptr))
    {
        ChipLogError(Zcl,
                     "Zone Management cluster on endpoint %u requires SetContext() before cluster initialization. Skipping.",
                     endpointId);
        return;
    }

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = chip::app::Clusters::ZoneManagement::Id,
            .fixedClusterInstanceCount = kZoneManagementFixedClusterCount,
            .maxClusterInstanceCount   = kZoneManagementMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterZoneManagementClusterShutdownCallback(chip::EndpointId endpointId, chip::app::MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = chip::app::Clusters::ZoneManagement::Id,
            .fixedClusterInstanceCount = kZoneManagementFixedClusterCount,
            .maxClusterInstanceCount   = kZoneManagementMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterZoneManagementPluginServerInitCallback() {}

void MatterZoneManagementPluginServerShutdownCallback() {}
