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

#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/static-cluster-config/ThreadNetworkDiagnostics.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kThreadNetworkDiagnosticsFixedClusterCount =
    ThreadNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThreadNetworkDiagnosticsMaxClusterCount =
    kThreadNetworkDiagnosticsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ThreadNetworkDiagnosticsCluster> gServers[kThreadNetworkDiagnosticsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<Feature> featureMap(rawFeatureMap);

        static constexpr AttributeId optionalAttributeIds[] = { ActiveTimestamp::Id,
                                                                PendingTimestamp::Id,
                                                                Delay::Id,
                                                                DetachedRoleCount::Id,
                                                                ChildRoleCount::Id,
                                                                RouterRoleCount::Id,
                                                                LeaderRoleCount::Id,
                                                                AttachAttemptCount::Id,
                                                                PartitionIdChangeCount::Id,
                                                                BetterPartitionAttachAttemptCount::Id,
                                                                ParentChangeCount::Id,
                                                                TxTotalCount::Id,
                                                                TxUnicastCount::Id,
                                                                TxBroadcastCount::Id,
                                                                TxAckRequestedCount::Id,
                                                                TxAckedCount::Id,
                                                                TxNoAckRequestedCount::Id,
                                                                TxDataCount::Id,
                                                                TxDataPollCount::Id,
                                                                TxBeaconCount::Id,
                                                                TxBeaconRequestCount::Id,
                                                                TxOtherCount::Id,
                                                                TxRetryCount::Id,
                                                                TxDirectMaxRetryExpiryCount::Id,
                                                                TxIndirectMaxRetryExpiryCount::Id,
                                                                TxErrCcaCount::Id,
                                                                TxErrAbortCount::Id,
                                                                TxErrBusyChannelCount::Id,
                                                                RxTotalCount::Id,
                                                                RxUnicastCount::Id,
                                                                RxBroadcastCount::Id,
                                                                RxDataCount::Id,
                                                                RxDataPollCount::Id,
                                                                RxBeaconCount::Id,
                                                                RxBeaconRequestCount::Id,
                                                                RxOtherCount::Id,
                                                                RxAddressFilteredCount::Id,
                                                                RxDestAddrFilteredCount::Id,
                                                                RxDuplicatedCount::Id,
                                                                RxErrNoFrameCount::Id,
                                                                RxErrUnknownNeighborCount::Id,
                                                                RxErrInvalidSrcAddrCount::Id,
                                                                RxErrSecCount::Id,
                                                                RxErrFcsCount::Id,
                                                                RxErrOtherCount::Id };

        VerifyOrDie(featureMap.Raw() == 0 || featureMap.Raw() == 0xF);

        if (featureMap.Raw() == 0xF)
        {
            for (auto id : optionalAttributeIds)
            {
                VerifyOrDie(emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, id));
            }
        }

        using ClusterType       = ThreadNetworkDiagnosticsCluster::ClusterType;
        ClusterType clusterType = (featureMap.Raw() == 0 ? ClusterType::kMinimal : ClusterType::kFull);
        gServers[clusterInstanceIndex].Create(endpointId, clusterType);
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

void MatterThreadNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterThreadNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}
