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

#include "CodegenIntegration.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/static-cluster-config/OccupancySensing.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DefaultTimerDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;
using namespace chip::app::Clusters::OccupancySensing::Attributes;

namespace {

constexpr size_t kOccupancySensingFixedClusterCount = OccupancySensing::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kOccupancySensingMaxClusterCount = kOccupancySensingFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<OccupancySensingCluster> gServers[kOccupancySensingMaxClusterCount];

DefaultTimerDelegate gDefaultTimerDelegate;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        OccupancySensingCluster::Config config(endpointId);
        config.WithFeatures(static_cast<Feature>(featureMap));

        bool hasSensorFeature = (featureMap &
                                 (to_underlying(Feature::kPassiveInfrared) | to_underlying(Feature::kUltrasonic) |
                                  to_underlying(Feature::kPhysicalContact)));

        // If HoldTime is optional OR if any sensor feature is present, enable the HoldTime logic.
        // The delay attributes are required if the corresponding sensor feature is present.
        if ((optionalAttributeBits & (1 << Attributes::HoldTime::Id)) || hasSensorFeature)
        {
            // Initializes hold time with default limits and default timer delegate. Application can use SetHoldTimeLimits() and
            // SetHoldTime() later to customize.
            constexpr chip::app::Clusters::OccupancySensing::Structs::HoldTimeLimitsStruct::Type kDefaultHoldTimeLimits = {
                .holdTimeMin = 1, .holdTimeMax = 60, .holdTimeDefault = 10
            };
            config.WithHoldTime(kDefaultHoldTimeLimits.holdTimeDefault, kDefaultHoldTimeLimits, gDefaultTimerDelegate);
        }

        gServers[clusterInstanceIndex].Create(config);
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

void MatterOccupancySensingClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OccupancySensing::Id,
            .fixedClusterInstanceCount = kOccupancySensingFixedClusterCount,
            .maxClusterInstanceCount   = kOccupancySensingMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterOccupancySensingClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OccupancySensing::Id,
            .fixedClusterInstanceCount = kOccupancySensingFixedClusterCount,
            .maxClusterInstanceCount   = kOccupancySensingMaxClusterCount,
        },
        integrationDelegate);
}

namespace chip::app::Clusters::OccupancySensing {

OccupancySensingCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * occupancySensing = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = OccupancySensing::Id,
            .fixedClusterInstanceCount = kOccupancySensingFixedClusterCount,
            .maxClusterInstanceCount   = kOccupancySensingMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<OccupancySensingCluster *>(occupancySensing);
}

} // namespace chip::app::Clusters::OccupancySensing

// Legacy PluginServer callback stubs
void MatterOccupancySensingPluginServerInitCallback() {}
void MatterOccupancySensingPluginServerShutdownCallback() {}
