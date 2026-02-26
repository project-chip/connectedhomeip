/*
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/static-cluster-config/AmbientContextSensing.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DefaultTimerDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AmbientContextSensing;
using namespace chip::app::Clusters::AmbientContextSensing::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {
constexpr size_t kAmbientContextSensingFixedClusterCount =
    AmbientContextSensing::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kAmbientContextSensingMaxClusterCount =
    kAmbientContextSensingFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<AmbientContextSensingCluster> gServers[kAmbientContextSensingMaxClusterCount];
DefaultTimerDelegate gDefaultTimerDelegate;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        AmbientContextSensingCluster::Config config(endpointId);
        config.WithFeatures(static_cast<Feature>(featureMap));
        constexpr chip::app::Clusters::AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type kDefaultHoldTimeLimits = {
            .holdTimeMin = kDefaultHoldTimeMin, .holdTimeMax = kDefaultHoldTimeMax, .holdTimeDefault = kDefaultHoldTimeDefault
        };
        config.WithHoldTime(kDefaultHoldTimeLimits.holdTimeDefault, kDefaultHoldTimeLimits, gDefaultTimerDelegate);
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

void MatterAmbientContextSensingClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AmbientContextSensing::Id,
            .fixedClusterInstanceCount = kAmbientContextSensingFixedClusterCount,
            .maxClusterInstanceCount   = kAmbientContextSensingMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterAmbientContextSensingClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = AmbientContextSensing::Id,
            .fixedClusterInstanceCount = kAmbientContextSensingFixedClusterCount,
            .maxClusterInstanceCount   = kAmbientContextSensingMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::AmbientContextSensing {

AmbientContextSensingCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * ambientContextSensing = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = AmbientContextSensing::Id,
            .fixedClusterInstanceCount = kAmbientContextSensingFixedClusterCount,
            .maxClusterInstanceCount   = kAmbientContextSensingMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<AmbientContextSensingCluster *>(ambientContextSensing);
}

} // namespace chip::app::Clusters::AmbientContextSensing

void MatterAmbientContextSensingPluginServerInitCallback() {}
void MatterAmbientContextSensingPluginServerShutdownCallback() {}
