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

#include <app/clusters/time-synchronization-server/CodegenIntegration.h>
#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>
#include <app/static-cluster-config/TimeSynchronization.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kTimeSynchronizationFixedClusterCount = TimeSynchronization::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kTimeSynchronizationMaxClusterCount =
    kTimeSynchronizationFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<TimeSynchronizationCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<TimeSynchronization::Feature> featureMap(rawFeatureMap);

        TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type supportsDNSResolve = false;
        SupportsDNSResolve::Get(endpointId, &supportsDNSResolve);

        TimeSynchronization::TimeZoneDatabaseEnum timeZoneDatabase = TimeZoneDatabaseEnum::kNone;
        TimeZoneDatabase::Get(endpointId, &timeZoneDatabase);

        TimeSynchronization::TimeSourceEnum timeSource = TimeSourceEnum::kNone;
        TimeSource::Get(endpointId, &timeSource);

        gServer.Create(endpointId, featureMap, supportsDNSResolve, timeZoneDatabase, timeSource);
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

void MatterTimeSynchronizationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterTimeSynchronizationClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
        },
        integrationDelegate);
}

void MatterTimeSynchronizationPluginServerInitCallback() {}

namespace chip::app::Clusters::TimeSynchronization {

TimeSynchronizationCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * timeSynchronization = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<TimeSynchronizationCluster *>(timeSynchronization);
}

} // namespace chip::app::Clusters::TimeSynchronization
