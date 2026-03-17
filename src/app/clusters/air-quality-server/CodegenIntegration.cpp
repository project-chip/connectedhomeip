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

#include "CodegenIntegration.h"
#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/static-cluster-config/AirQuality.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr uint16_t kAirQualityFixedClusterCount =
    static_cast<uint16_t>(Clusters::AirQuality::StaticApplicationConfig::kFixedClusterConfig.size());
constexpr uint16_t kAirQualityMaxClusterCount = kAirQualityFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<AirQualityCluster> gServers[kAirQualityMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId);
        gServers[clusterInstanceIndex].Cluster().SetFeatureMap(BitFlags<Clusters::AirQuality::Feature>(featureMap));
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

void MatterAirQualityClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Clusters::AirQuality::Id,
            .fixedClusterInstanceCount = kAirQualityFixedClusterCount,
            .maxClusterInstanceCount   = kAirQualityMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterAirQualityClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Clusters::AirQuality::Id,
            .fixedClusterInstanceCount = kAirQualityFixedClusterCount,
            .maxClusterInstanceCount   = kAirQualityMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::AirQuality {

AirQualityCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * airQuality = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = chip::app::Clusters::AirQuality::Id,
            .fixedClusterInstanceCount = kAirQualityFixedClusterCount,
            .maxClusterInstanceCount   = kAirQualityMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<AirQualityCluster *>(airQuality);
}

} // namespace chip::app::Clusters::AirQuality
