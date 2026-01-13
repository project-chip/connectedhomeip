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
#include <app/clusters/time-synchronization-server/TimeSynchronizationCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
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

static constexpr size_t kTimeSynchronizationFixedClusterCount =
    TimeSynchronization::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kTimeSynchronizationFixedClusterCount == 0) ||
                  ((kTimeSynchronizationFixedClusterCount == 1) &&
                   TimeSynchronization::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Time Synchronization cluster MUST be on endpoint 0");

LazyRegisteredServerCluster<TimeSynchronizationCluster> gServer;

TimeSynchronization::Delegate * gDelegate = nullptr;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<Feature> featureMap(rawFeatureMap);

        SupportsDNSResolve::TypeInfo::Type supportsDNSResolve = false;
        if (featureMap.Has(Feature::kNTPClient))
        {
            VerifyOrDie(SupportsDNSResolve::Get(endpointId, &supportsDNSResolve) == Status::Success);
        }

        TimeZoneDatabaseEnum timeZoneDatabase = TimeZoneDatabaseEnum::kNone;
        if (featureMap.Has(Feature::kTimeZone))
        {
            VerifyOrDie(TimeZoneDatabase::Get(endpointId, &timeZoneDatabase) == Status::Success);
        }

        TimeSynchronizationCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        TimeSourceEnum timeSource = TimeSourceEnum::kNone;
        if (optionalAttributeSet.IsSet(TimeSource::Id))
        {
            VerifyOrDie(TimeSource::Get(endpointId, &timeSource) == Status::Success);
        }

        NTPServerAvailable::TypeInfo::Type ntpServerAvailable = false;
        if (featureMap.Has(Feature::kNTPServer))
        {
            VerifyOrDie(SupportsDNSResolve::Get(endpointId, &ntpServerAvailable) == Status::Success);
        }

        gServer.Create(endpointId, featureMap, optionalAttributeSet,
                       TimeSynchronizationCluster::StartupConfiguration{ .supportsDNSResolve = supportsDNSResolve,
                                                                         .ntpServerAvailable = ntpServerAvailable,
                                                                         .timeZoneDatabase   = timeZoneDatabase,
                                                                         .timeSource         = timeSource,
                                                                         .delegate = TimeSynchronization::GetDefaultDelegate() });
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
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterTimeSynchronizationClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate, shutdownType);
}

void MatterTimeSynchronizationPluginServerInitCallback() {}

namespace chip::app::Clusters::TimeSynchronization {

TimeSynchronizationCluster * GetClusterInstance()
{
    VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
    return &gServer.Cluster();
}

void SetDefaultDelegate(Delegate * delegate)
{
    VerifyOrReturn(delegate != nullptr);
    gDelegate                = delegate;
    auto timeSynchronization = GetClusterInstance();
    if (timeSynchronization != nullptr)
    {
        timeSynchronization->SetDelegate(gDelegate);
    }
}

Delegate * GetDefaultDelegate()
{
    auto timeSynchronization = GetClusterInstance();
    if (timeSynchronization != nullptr)
    {
        return timeSynchronization->GetDelegate();
    }
    if (gDelegate == nullptr)
    {
        static DefaultTimeSyncDelegate delegate;
        gDelegate = &delegate;
    }
    return gDelegate;
}

} // namespace chip::app::Clusters::TimeSynchronization
