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
        ThreadNetworkDiagnosticsCluster::StartupConfiguration config;

        config.activeTs           = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, ActiveTimestamp::Id);
        config.pendingTs          = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, PendingTimestamp::Id);
        config.delay              = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, Delay::Id);
        config.detachedRoleCount  = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, DetachedRoleCount::Id);
        config.childRoleCount     = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, ChildRoleCount::Id);
        config.routerRoleCount    = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RouterRoleCount::Id);
        config.leaderRoleCount    = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, LeaderRoleCount::Id);
        config.attachAttemptCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, AttachAttemptCount::Id);
        config.partitionIdChangeCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, PartitionIdChangeCount::Id);
        config.betterPartitionAttachAttemptCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, BetterPartitionAttachAttemptCount::Id);
        config.parentChangeCount   = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, ParentChangeCount::Id);
        config.txTotalCount        = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxTotalCount::Id);
        config.txUnicastCount      = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxUnicastCount::Id);
        config.txBroadcastCount    = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxBroadcastCount::Id);
        config.txAckRequestedCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxAckRequestedCount::Id);
        config.txAckedCount        = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxAckedCount::Id);
        config.txNoAckRequestedCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxNoAckRequestedCount::Id);
        config.txDataCount          = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxDataCount::Id);
        config.txDataPollCount      = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxDataPollCount::Id);
        config.txBeaconCount        = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxBeaconCount::Id);
        config.txBeaconRequestCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxBeaconRequestCount::Id);
        config.txOtherCount         = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxOtherCount::Id);
        config.txRetryCount         = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxRetryCount::Id);
        config.txDirectMaxRetryExpiryCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxDirectMaxRetryExpiryCount::Id);
        config.txIndirectMaxRetryExpiryCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxIndirectMaxRetryExpiryCount::Id);
        config.txErrCcaCount   = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxErrCcaCount::Id);
        config.txErrAbortCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxErrAbortCount::Id);
        config.txErrBusyChannelCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, TxErrBusyChannelCount::Id);
        config.rxTotalCount         = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxTotalCount::Id);
        config.rxUnicastCount       = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxUnicastCount::Id);
        config.rxBroadcastCount     = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxBroadcastCount::Id);
        config.rxDataCount          = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxDataCount::Id);
        config.rxDataPollCount      = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxDataPollCount::Id);
        config.rxBeaconCount        = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxBeaconCount::Id);
        config.rxBeaconRequestCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxBeaconRequestCount::Id);
        config.rxOtherCount         = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxOtherCount::Id);
        config.rxAddressFilteredCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxAddressFilteredCount::Id);
        config.rxDestAddrFilteredCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxDestAddrFilteredCount::Id);
        config.rxDuplicatedCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxDuplicatedCount::Id);
        config.rxErrNoFrameCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrNoFrameCount::Id);
        config.rxErrUnknownNeighborCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrUnknownNeighborCount::Id);
        config.rxErrInvalidSrcAddrCount =
            emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrInvalidSrcAddrCount::Id);
        config.rxErrSecCount   = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrSecCount::Id);
        config.rxErrFcsCount   = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrFcsCount::Id);
        config.rxErrOtherCount = emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, RxErrOtherCount::Id);

        gServers[clusterInstanceIndex].Create(endpointId, featureMap, config);
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

namespace chip::app::Clusters::ThreadNetworkDiagnostics {

ThreadNetworkDiagnosticsCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * threadNetworkDiagnostics = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ThreadNetworkDiagnosticsCluster *>(threadNetworkDiagnostics);
}

} // namespace chip::app::Clusters::ThreadNetworkDiagnostics

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}
