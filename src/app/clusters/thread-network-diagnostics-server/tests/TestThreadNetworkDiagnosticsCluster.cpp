/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ThreadNetworkDiagnostics/Attributes.h>
#include <clusters/ThreadNetworkDiagnostics/Enums.h>
#include <clusters/ThreadNetworkDiagnostics/Metadata.h>
#include <clusters/ThreadNetworkDiagnostics/Structs.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::Testing;

struct TestThreadNetworkDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestThreadNetworkDiagnosticsCluster() {}

    TestServerClusterContext testContext;
};

} // namespace

TEST_F(TestThreadNetworkDiagnosticsCluster, AttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(
            threadNetworkDiagnostics.Attributes(ConcreteClusterPath(kRootEndpointId, ThreadNetworkDiagnostics::Id), attributes),
            CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ 0U };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(
            kRootEndpointId, features,
            ThreadNetworkDiagnosticsCluster::StartupConfiguration{ .activeTs = true, .pendingTs = true, .delay = true });
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(
            threadNetworkDiagnostics.Attributes(ConcreteClusterPath(kRootEndpointId, ThreadNetworkDiagnostics::Id), attributes),
            CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = { { true, ActiveTimestamp::kMetadataEntry },
                                                                                    { true, PendingTimestamp::kMetadataEntry },
                                                                                    { true, Delay::kMetadataEntry } };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMLECounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(
            threadNetworkDiagnostics.Attributes(ConcreteClusterPath(kRootEndpointId, ThreadNetworkDiagnostics::Id), attributes),
            CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
            { true, DetachedRoleCount::kMetadataEntry },
            { true, ChildRoleCount::kMetadataEntry },
            { true, RouterRoleCount::kMetadataEntry },
            { true, LeaderRoleCount::kMetadataEntry },
            { true, AttachAttemptCount::kMetadataEntry },
            { true, PartitionIdChangeCount::kMetadataEntry },
            { true, BetterPartitionAttachAttemptCount::kMetadataEntry },
            { true, ParentChangeCount::kMetadataEntry }
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMACCounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(
            threadNetworkDiagnostics.Attributes(ConcreteClusterPath(kRootEndpointId, ThreadNetworkDiagnostics::Id), attributes),
            CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
            { true, TxTotalCount::kMetadataEntry },
            { true, TxUnicastCount::kMetadataEntry },
            { true, TxBroadcastCount::kMetadataEntry },
            { true, TxAckRequestedCount::kMetadataEntry },
            { true, TxAckedCount::kMetadataEntry },
            { true, TxNoAckRequestedCount::kMetadataEntry },
            { true, TxDataCount::kMetadataEntry },
            { true, TxDataPollCount::kMetadataEntry },
            { true, TxBeaconCount::kMetadataEntry },
            { true, TxBeaconRequestCount::kMetadataEntry },
            { true, TxOtherCount::kMetadataEntry },
            { true, TxRetryCount::kMetadataEntry },
            { true, TxDirectMaxRetryExpiryCount::kMetadataEntry },
            { true, TxIndirectMaxRetryExpiryCount::kMetadataEntry },
            { true, TxErrCcaCount::kMetadataEntry },
            { true, TxErrAbortCount::kMetadataEntry },
            { true, TxErrBusyChannelCount::kMetadataEntry },
            { true, RxTotalCount::kMetadataEntry },
            { true, RxUnicastCount::kMetadataEntry },
            { true, RxBroadcastCount::kMetadataEntry },
            { true, RxDataCount::kMetadataEntry },
            { true, RxDataPollCount::kMetadataEntry },
            { true, RxBeaconCount::kMetadataEntry },
            { true, RxBeaconRequestCount::kMetadataEntry },
            { true, RxOtherCount::kMetadataEntry },
            { true, RxAddressFilteredCount::kMetadataEntry },
            { true, RxDestAddrFilteredCount::kMetadataEntry },
            { true, RxDuplicatedCount::kMetadataEntry },
            { true, RxErrNoFrameCount::kMetadataEntry },
            { true, RxErrUnknownNeighborCount::kMetadataEntry },
            { true, RxErrInvalidSrcAddrCount::kMetadataEntry },
            { true, RxErrSecCount::kMetadataEntry },
            { true, RxErrFcsCount::kMetadataEntry },
            { true, RxErrOtherCount::kMetadataEntry }
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kErrorCounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(
            threadNetworkDiagnostics.Attributes(ConcreteClusterPath(kRootEndpointId, ThreadNetworkDiagnostics::Id), attributes),
            CHIP_NO_ERROR);

        AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = { { true, OverrunCount::kMetadataEntry } };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestThreadNetworkDiagnosticsCluster, ReadAttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(threadNetworkDiagnostics);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        Attributes::Channel::TypeInfo::Type channel;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Channel::Id, channel), CHIP_NO_ERROR);

        RoutingRole::TypeInfo::Type routingRole;
        ASSERT_EQ(tester.ReadAttribute(RoutingRole::Id, routingRole), CHIP_NO_ERROR);

        NetworkName::TypeInfo::Type networkName;
        ASSERT_EQ(tester.ReadAttribute(NetworkName::Id, networkName), CHIP_NO_ERROR);

        PanId::TypeInfo::Type panId;
        ASSERT_EQ(tester.ReadAttribute(PanId::Id, panId), CHIP_NO_ERROR);

        ExtendedPanId::TypeInfo::Type extendedPanId;
        ASSERT_EQ(tester.ReadAttribute(ExtendedPanId::Id, extendedPanId), CHIP_NO_ERROR);

        MeshLocalPrefix::TypeInfo::Type meshLocalPrefix;
        ASSERT_EQ(tester.ReadAttribute(MeshLocalPrefix::Id, meshLocalPrefix), CHIP_NO_ERROR);

        NeighborTable::TypeInfo::DecodableType neighborTable;
        ASSERT_EQ(tester.ReadAttribute(NeighborTable::Id, neighborTable), CHIP_NO_ERROR);

        RouteTable::TypeInfo::DecodableType routeTable;
        ASSERT_EQ(tester.ReadAttribute(RouteTable::Id, routeTable), CHIP_NO_ERROR);

        PartitionId::TypeInfo::Type partitionId;
        ASSERT_EQ(tester.ReadAttribute(PartitionId::Id, partitionId), CHIP_NO_ERROR);

        Weighting::TypeInfo::Type weighting;
        ASSERT_EQ(tester.ReadAttribute(Weighting::Id, weighting), CHIP_NO_ERROR);

        Attributes::DataVersion::TypeInfo::Type dataVersion;
        ASSERT_EQ(tester.ReadAttribute(Attributes::DataVersion::Id, dataVersion), CHIP_NO_ERROR);

        StableDataVersion::TypeInfo::Type stableDataVersion;
        ASSERT_EQ(tester.ReadAttribute(StableDataVersion::Id, stableDataVersion), CHIP_NO_ERROR);

        LeaderRouterId::TypeInfo::Type leaderRouterId;
        ASSERT_EQ(tester.ReadAttribute(LeaderRouterId::Id, leaderRouterId), CHIP_NO_ERROR);

        SecurityPolicy::TypeInfo::DecodableType securityPolicy;
        ASSERT_EQ(tester.ReadAttribute(SecurityPolicy::Id, securityPolicy), CHIP_NO_ERROR);

        ChannelPage0Mask::TypeInfo::Type channelPage0Mask;
        ASSERT_EQ(tester.ReadAttribute(ChannelPage0Mask::Id, channelPage0Mask), CHIP_NO_ERROR);

        OperationalDatasetComponents::TypeInfo::DecodableType operationalDatasetComponents;
        ASSERT_EQ(tester.ReadAttribute(OperationalDatasetComponents::Id, operationalDatasetComponents), CHIP_NO_ERROR);

        ActiveNetworkFaultsList::TypeInfo::DecodableType activeNetworkFaultsList;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkFaultsList::Id, activeNetworkFaultsList), CHIP_NO_ERROR);

        ExtAddress::TypeInfo::Type extAddress;
        ASSERT_EQ(tester.ReadAttribute(ExtAddress::Id, extAddress), CHIP_NO_ERROR);

        Rloc16::TypeInfo::Type rloc16;
        ASSERT_EQ(tester.ReadAttribute(Rloc16::Id, rloc16), CHIP_NO_ERROR);

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ 0U };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(
            kRootEndpointId, features,
            ThreadNetworkDiagnosticsCluster::StartupConfiguration{ .activeTs = true, .pendingTs = true, .delay = true });
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(threadNetworkDiagnostics);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        Attributes::Channel::TypeInfo::Type channel;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Channel::Id, channel), CHIP_NO_ERROR);

        RoutingRole::TypeInfo::Type routingRole;
        ASSERT_EQ(tester.ReadAttribute(RoutingRole::Id, routingRole), CHIP_NO_ERROR);

        NetworkName::TypeInfo::Type networkName;
        ASSERT_EQ(tester.ReadAttribute(NetworkName::Id, networkName), CHIP_NO_ERROR);

        PanId::TypeInfo::Type panId;
        ASSERT_EQ(tester.ReadAttribute(PanId::Id, panId), CHIP_NO_ERROR);

        ExtendedPanId::TypeInfo::Type extendedPanId;
        ASSERT_EQ(tester.ReadAttribute(ExtendedPanId::Id, extendedPanId), CHIP_NO_ERROR);

        MeshLocalPrefix::TypeInfo::Type meshLocalPrefix;
        ASSERT_EQ(tester.ReadAttribute(MeshLocalPrefix::Id, meshLocalPrefix), CHIP_NO_ERROR);

        NeighborTable::TypeInfo::DecodableType neighborTable;
        ASSERT_EQ(tester.ReadAttribute(NeighborTable::Id, neighborTable), CHIP_NO_ERROR);

        RouteTable::TypeInfo::DecodableType routeTable;
        ASSERT_EQ(tester.ReadAttribute(RouteTable::Id, routeTable), CHIP_NO_ERROR);

        PartitionId::TypeInfo::Type partitionId;
        ASSERT_EQ(tester.ReadAttribute(PartitionId::Id, partitionId), CHIP_NO_ERROR);

        Weighting::TypeInfo::Type weighting;
        ASSERT_EQ(tester.ReadAttribute(Weighting::Id, weighting), CHIP_NO_ERROR);

        Attributes::DataVersion::TypeInfo::Type dataVersion;
        ASSERT_EQ(tester.ReadAttribute(Attributes::DataVersion::Id, dataVersion), CHIP_NO_ERROR);

        StableDataVersion::TypeInfo::Type stableDataVersion;
        ASSERT_EQ(tester.ReadAttribute(StableDataVersion::Id, stableDataVersion), CHIP_NO_ERROR);

        LeaderRouterId::TypeInfo::Type leaderRouterId;
        ASSERT_EQ(tester.ReadAttribute(LeaderRouterId::Id, leaderRouterId), CHIP_NO_ERROR);

        SecurityPolicy::TypeInfo::DecodableType securityPolicy;
        ASSERT_EQ(tester.ReadAttribute(SecurityPolicy::Id, securityPolicy), CHIP_NO_ERROR);

        ChannelPage0Mask::TypeInfo::Type channelPage0Mask;
        ASSERT_EQ(tester.ReadAttribute(ChannelPage0Mask::Id, channelPage0Mask), CHIP_NO_ERROR);

        OperationalDatasetComponents::TypeInfo::DecodableType operationalDatasetComponents;
        ASSERT_EQ(tester.ReadAttribute(OperationalDatasetComponents::Id, operationalDatasetComponents), CHIP_NO_ERROR);

        ActiveNetworkFaultsList::TypeInfo::DecodableType activeNetworkFaultsList;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkFaultsList::Id, activeNetworkFaultsList), CHIP_NO_ERROR);

        ExtAddress::TypeInfo::Type extAddress;
        ASSERT_EQ(tester.ReadAttribute(ExtAddress::Id, extAddress), CHIP_NO_ERROR);

        Rloc16::TypeInfo::Type rloc16;
        ASSERT_EQ(tester.ReadAttribute(Rloc16::Id, rloc16), CHIP_NO_ERROR);

        ActiveTimestamp::TypeInfo::Type activeTimestamp;
        ASSERT_EQ(tester.ReadAttribute(ActiveTimestamp::Id, activeTimestamp), CHIP_NO_ERROR);

        PendingTimestamp::TypeInfo::Type pendingTimestamp;
        ASSERT_EQ(tester.ReadAttribute(PendingTimestamp::Id, pendingTimestamp), CHIP_NO_ERROR);

        Delay::TypeInfo::Type delay;
        ASSERT_EQ(tester.ReadAttribute(Delay::Id, delay), CHIP_NO_ERROR);

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMLECounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(threadNetworkDiagnostics);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        Attributes::Channel::TypeInfo::Type channel;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Channel::Id, channel), CHIP_NO_ERROR);

        RoutingRole::TypeInfo::Type routingRole;
        ASSERT_EQ(tester.ReadAttribute(RoutingRole::Id, routingRole), CHIP_NO_ERROR);

        NetworkName::TypeInfo::Type networkName;
        ASSERT_EQ(tester.ReadAttribute(NetworkName::Id, networkName), CHIP_NO_ERROR);

        PanId::TypeInfo::Type panId;
        ASSERT_EQ(tester.ReadAttribute(PanId::Id, panId), CHIP_NO_ERROR);

        ExtendedPanId::TypeInfo::Type extendedPanId;
        ASSERT_EQ(tester.ReadAttribute(ExtendedPanId::Id, extendedPanId), CHIP_NO_ERROR);

        MeshLocalPrefix::TypeInfo::Type meshLocalPrefix;
        ASSERT_EQ(tester.ReadAttribute(MeshLocalPrefix::Id, meshLocalPrefix), CHIP_NO_ERROR);

        NeighborTable::TypeInfo::DecodableType neighborTable;
        ASSERT_EQ(tester.ReadAttribute(NeighborTable::Id, neighborTable), CHIP_NO_ERROR);

        RouteTable::TypeInfo::DecodableType routeTable;
        ASSERT_EQ(tester.ReadAttribute(RouteTable::Id, routeTable), CHIP_NO_ERROR);

        PartitionId::TypeInfo::Type partitionId;
        ASSERT_EQ(tester.ReadAttribute(PartitionId::Id, partitionId), CHIP_NO_ERROR);

        Weighting::TypeInfo::Type weighting;
        ASSERT_EQ(tester.ReadAttribute(Weighting::Id, weighting), CHIP_NO_ERROR);

        Attributes::DataVersion::TypeInfo::Type dataVersion;
        ASSERT_EQ(tester.ReadAttribute(Attributes::DataVersion::Id, dataVersion), CHIP_NO_ERROR);

        StableDataVersion::TypeInfo::Type stableDataVersion;
        ASSERT_EQ(tester.ReadAttribute(StableDataVersion::Id, stableDataVersion), CHIP_NO_ERROR);

        LeaderRouterId::TypeInfo::Type leaderRouterId;
        ASSERT_EQ(tester.ReadAttribute(LeaderRouterId::Id, leaderRouterId), CHIP_NO_ERROR);

        SecurityPolicy::TypeInfo::DecodableType securityPolicy;
        ASSERT_EQ(tester.ReadAttribute(SecurityPolicy::Id, securityPolicy), CHIP_NO_ERROR);

        ChannelPage0Mask::TypeInfo::Type channelPage0Mask;
        ASSERT_EQ(tester.ReadAttribute(ChannelPage0Mask::Id, channelPage0Mask), CHIP_NO_ERROR);

        OperationalDatasetComponents::TypeInfo::DecodableType operationalDatasetComponents;
        ASSERT_EQ(tester.ReadAttribute(OperationalDatasetComponents::Id, operationalDatasetComponents), CHIP_NO_ERROR);

        ActiveNetworkFaultsList::TypeInfo::DecodableType activeNetworkFaultsList;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkFaultsList::Id, activeNetworkFaultsList), CHIP_NO_ERROR);

        ExtAddress::TypeInfo::Type extAddress;
        ASSERT_EQ(tester.ReadAttribute(ExtAddress::Id, extAddress), CHIP_NO_ERROR);

        Rloc16::TypeInfo::Type rloc16;
        ASSERT_EQ(tester.ReadAttribute(Rloc16::Id, rloc16), CHIP_NO_ERROR);

        DetachedRoleCount::TypeInfo::Type detachedRoleCount;
        ASSERT_EQ(tester.ReadAttribute(DetachedRoleCount::Id, detachedRoleCount), CHIP_NO_ERROR);

        ChildRoleCount::TypeInfo::Type childRoleCount;
        ASSERT_EQ(tester.ReadAttribute(ChildRoleCount::Id, childRoleCount), CHIP_NO_ERROR);

        RouterRoleCount::TypeInfo::Type routerRoleCount;
        ASSERT_EQ(tester.ReadAttribute(RouterRoleCount::Id, routerRoleCount), CHIP_NO_ERROR);

        LeaderRoleCount::TypeInfo::Type leaderRoleCount;
        ASSERT_EQ(tester.ReadAttribute(LeaderRoleCount::Id, leaderRoleCount), CHIP_NO_ERROR);

        AttachAttemptCount::TypeInfo::Type attachAttemptCount;
        ASSERT_EQ(tester.ReadAttribute(AttachAttemptCount::Id, attachAttemptCount), CHIP_NO_ERROR);

        PartitionIdChangeCount::TypeInfo::Type partitionIdChangeCount;
        ASSERT_EQ(tester.ReadAttribute(PartitionIdChangeCount::Id, partitionIdChangeCount), CHIP_NO_ERROR);

        BetterPartitionAttachAttemptCount::TypeInfo::Type betterPartitionAttachAttemptCount;
        ASSERT_EQ(tester.ReadAttribute(BetterPartitionAttachAttemptCount::Id, betterPartitionAttachAttemptCount), CHIP_NO_ERROR);

        ParentChangeCount::TypeInfo::Type parentChangeCount;
        ASSERT_EQ(tester.ReadAttribute(ParentChangeCount::Id, parentChangeCount), CHIP_NO_ERROR);

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMACCounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(threadNetworkDiagnostics);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        Attributes::Channel::TypeInfo::Type channel;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Channel::Id, channel), CHIP_NO_ERROR);

        RoutingRole::TypeInfo::Type routingRole;
        ASSERT_EQ(tester.ReadAttribute(RoutingRole::Id, routingRole), CHIP_NO_ERROR);

        NetworkName::TypeInfo::Type networkName;
        ASSERT_EQ(tester.ReadAttribute(NetworkName::Id, networkName), CHIP_NO_ERROR);

        PanId::TypeInfo::Type panId;
        ASSERT_EQ(tester.ReadAttribute(PanId::Id, panId), CHIP_NO_ERROR);

        ExtendedPanId::TypeInfo::Type extendedPanId;
        ASSERT_EQ(tester.ReadAttribute(ExtendedPanId::Id, extendedPanId), CHIP_NO_ERROR);

        MeshLocalPrefix::TypeInfo::Type meshLocalPrefix;
        ASSERT_EQ(tester.ReadAttribute(MeshLocalPrefix::Id, meshLocalPrefix), CHIP_NO_ERROR);

        NeighborTable::TypeInfo::DecodableType neighborTable;
        ASSERT_EQ(tester.ReadAttribute(NeighborTable::Id, neighborTable), CHIP_NO_ERROR);

        RouteTable::TypeInfo::DecodableType routeTable;
        ASSERT_EQ(tester.ReadAttribute(RouteTable::Id, routeTable), CHIP_NO_ERROR);

        PartitionId::TypeInfo::Type partitionId;
        ASSERT_EQ(tester.ReadAttribute(PartitionId::Id, partitionId), CHIP_NO_ERROR);

        Weighting::TypeInfo::Type weighting;
        ASSERT_EQ(tester.ReadAttribute(Weighting::Id, weighting), CHIP_NO_ERROR);

        Attributes::DataVersion::TypeInfo::Type dataVersion;
        ASSERT_EQ(tester.ReadAttribute(Attributes::DataVersion::Id, dataVersion), CHIP_NO_ERROR);

        StableDataVersion::TypeInfo::Type stableDataVersion;
        ASSERT_EQ(tester.ReadAttribute(StableDataVersion::Id, stableDataVersion), CHIP_NO_ERROR);

        LeaderRouterId::TypeInfo::Type leaderRouterId;
        ASSERT_EQ(tester.ReadAttribute(LeaderRouterId::Id, leaderRouterId), CHIP_NO_ERROR);

        SecurityPolicy::TypeInfo::DecodableType securityPolicy;
        ASSERT_EQ(tester.ReadAttribute(SecurityPolicy::Id, securityPolicy), CHIP_NO_ERROR);

        ChannelPage0Mask::TypeInfo::Type channelPage0Mask;
        ASSERT_EQ(tester.ReadAttribute(ChannelPage0Mask::Id, channelPage0Mask), CHIP_NO_ERROR);

        OperationalDatasetComponents::TypeInfo::DecodableType operationalDatasetComponents;
        ASSERT_EQ(tester.ReadAttribute(OperationalDatasetComponents::Id, operationalDatasetComponents), CHIP_NO_ERROR);

        ActiveNetworkFaultsList::TypeInfo::DecodableType activeNetworkFaultsList;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkFaultsList::Id, activeNetworkFaultsList), CHIP_NO_ERROR);

        ExtAddress::TypeInfo::Type extAddress;
        ASSERT_EQ(tester.ReadAttribute(ExtAddress::Id, extAddress), CHIP_NO_ERROR);

        Rloc16::TypeInfo::Type rloc16;
        ASSERT_EQ(tester.ReadAttribute(Rloc16::Id, rloc16), CHIP_NO_ERROR);

        TxTotalCount::TypeInfo::Type txTotalCount;
        ASSERT_EQ(tester.ReadAttribute(TxTotalCount::Id, txTotalCount), CHIP_NO_ERROR);

        TxUnicastCount::TypeInfo::Type txUnicastCount;
        ASSERT_EQ(tester.ReadAttribute(TxUnicastCount::Id, txUnicastCount), CHIP_NO_ERROR);

        TxBroadcastCount::TypeInfo::Type txBroadcastCount;
        ASSERT_EQ(tester.ReadAttribute(TxBroadcastCount::Id, txBroadcastCount), CHIP_NO_ERROR);

        TxAckRequestedCount::TypeInfo::Type txAckRequestedCount;
        ASSERT_EQ(tester.ReadAttribute(TxAckRequestedCount::Id, txAckRequestedCount), CHIP_NO_ERROR);

        TxAckedCount::TypeInfo::Type txAckedCount;
        ASSERT_EQ(tester.ReadAttribute(TxAckedCount::Id, txAckedCount), CHIP_NO_ERROR);

        TxNoAckRequestedCount::TypeInfo::Type txNoAckRequestedCount;
        ASSERT_EQ(tester.ReadAttribute(TxNoAckRequestedCount::Id, txNoAckRequestedCount), CHIP_NO_ERROR);

        TxDataCount::TypeInfo::Type txDataCount;
        ASSERT_EQ(tester.ReadAttribute(TxDataCount::Id, txDataCount), CHIP_NO_ERROR);

        TxDataPollCount::TypeInfo::Type txDataPollCount;
        ASSERT_EQ(tester.ReadAttribute(TxDataPollCount::Id, txDataPollCount), CHIP_NO_ERROR);

        TxBeaconCount::TypeInfo::Type txBeaconCount;
        ASSERT_EQ(tester.ReadAttribute(TxBeaconCount::Id, txBeaconCount), CHIP_NO_ERROR);

        TxBeaconRequestCount::TypeInfo::Type txBeaconRequestCount;
        ASSERT_EQ(tester.ReadAttribute(TxBeaconRequestCount::Id, txBeaconRequestCount), CHIP_NO_ERROR);

        TxOtherCount::TypeInfo::Type txOtherCount;
        ASSERT_EQ(tester.ReadAttribute(TxOtherCount::Id, txOtherCount), CHIP_NO_ERROR);

        TxRetryCount::TypeInfo::Type txRetryCount;
        ASSERT_EQ(tester.ReadAttribute(TxRetryCount::Id, txRetryCount), CHIP_NO_ERROR);

        TxDirectMaxRetryExpiryCount::TypeInfo::Type txDirectMaxRetryExpiryCount;
        ASSERT_EQ(tester.ReadAttribute(TxDirectMaxRetryExpiryCount::Id, txDirectMaxRetryExpiryCount), CHIP_NO_ERROR);

        TxIndirectMaxRetryExpiryCount::TypeInfo::Type txIndirectMaxRetryExpiryCount;
        ASSERT_EQ(tester.ReadAttribute(TxIndirectMaxRetryExpiryCount::Id, txIndirectMaxRetryExpiryCount), CHIP_NO_ERROR);

        TxErrCcaCount::TypeInfo::Type txErrCcaCount;
        ASSERT_EQ(tester.ReadAttribute(TxErrCcaCount::Id, txErrCcaCount), CHIP_NO_ERROR);

        TxErrAbortCount::TypeInfo::Type txErrAbortCount;
        ASSERT_EQ(tester.ReadAttribute(TxErrAbortCount::Id, txErrAbortCount), CHIP_NO_ERROR);

        TxErrBusyChannelCount::TypeInfo::Type txErrBusyChannelCount;
        ASSERT_EQ(tester.ReadAttribute(TxErrBusyChannelCount::Id, txErrBusyChannelCount), CHIP_NO_ERROR);

        RxTotalCount::TypeInfo::Type rxTotalCount;
        ASSERT_EQ(tester.ReadAttribute(RxTotalCount::Id, rxTotalCount), CHIP_NO_ERROR);

        RxUnicastCount::TypeInfo::Type rxUnicastCount;
        ASSERT_EQ(tester.ReadAttribute(RxUnicastCount::Id, rxUnicastCount), CHIP_NO_ERROR);

        RxBroadcastCount::TypeInfo::Type rxBroadcastCount;
        ASSERT_EQ(tester.ReadAttribute(RxBroadcastCount::Id, rxBroadcastCount), CHIP_NO_ERROR);

        RxDataCount::TypeInfo::Type rxDataCount;
        ASSERT_EQ(tester.ReadAttribute(RxDataCount::Id, rxDataCount), CHIP_NO_ERROR);

        RxDataPollCount::TypeInfo::Type rxDataPollCount;
        ASSERT_EQ(tester.ReadAttribute(RxDataPollCount::Id, rxDataPollCount), CHIP_NO_ERROR);

        RxBeaconCount::TypeInfo::Type rxBeaconCount;
        ASSERT_EQ(tester.ReadAttribute(RxBeaconCount::Id, rxBeaconCount), CHIP_NO_ERROR);

        RxBeaconRequestCount::TypeInfo::Type rxBeaconRequestCount;
        ASSERT_EQ(tester.ReadAttribute(RxBeaconRequestCount::Id, rxBeaconRequestCount), CHIP_NO_ERROR);

        RxOtherCount::TypeInfo::Type rxOtherCount;
        ASSERT_EQ(tester.ReadAttribute(RxOtherCount::Id, rxOtherCount), CHIP_NO_ERROR);

        RxAddressFilteredCount::TypeInfo::Type rxAddressFilteredCount;
        ASSERT_EQ(tester.ReadAttribute(RxAddressFilteredCount::Id, rxAddressFilteredCount), CHIP_NO_ERROR);

        RxDestAddrFilteredCount::TypeInfo::Type rxDestAddrFilteredCount;
        ASSERT_EQ(tester.ReadAttribute(RxDestAddrFilteredCount::Id, rxDestAddrFilteredCount), CHIP_NO_ERROR);

        RxDuplicatedCount::TypeInfo::Type rxDuplicatedCount;
        ASSERT_EQ(tester.ReadAttribute(RxDuplicatedCount::Id, rxDuplicatedCount), CHIP_NO_ERROR);

        RxErrNoFrameCount::TypeInfo::Type rxErrNoFrameCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrNoFrameCount::Id, rxErrNoFrameCount), CHIP_NO_ERROR);

        RxErrUnknownNeighborCount::TypeInfo::Type rxErrUnknownNeighborCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrUnknownNeighborCount::Id, rxErrUnknownNeighborCount), CHIP_NO_ERROR);

        RxErrInvalidSrcAddrCount::TypeInfo::Type rxErrInvalidSrcAddrCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrInvalidSrcAddrCount::Id, rxErrInvalidSrcAddrCount), CHIP_NO_ERROR);

        RxErrSecCount::TypeInfo::Type rxErrSecCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrSecCount::Id, rxErrSecCount), CHIP_NO_ERROR);

        RxErrFcsCount::TypeInfo::Type rxErrFcsCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrFcsCount::Id, rxErrFcsCount), CHIP_NO_ERROR);

        RxErrOtherCount::TypeInfo::Type rxErrOtherCount;
        ASSERT_EQ(tester.ReadAttribute(RxErrOtherCount::Id, rxErrOtherCount), CHIP_NO_ERROR);

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kErrorCounts };
        ThreadNetworkDiagnosticsCluster threadNetworkDiagnostics(kRootEndpointId, features,
                                                                 ThreadNetworkDiagnosticsCluster::StartupConfiguration{});
        ASSERT_EQ(threadNetworkDiagnostics.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(threadNetworkDiagnostics);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        Attributes::Channel::TypeInfo::Type channel;
        ASSERT_EQ(tester.ReadAttribute(Attributes::Channel::Id, channel), CHIP_NO_ERROR);

        RoutingRole::TypeInfo::Type routingRole;
        ASSERT_EQ(tester.ReadAttribute(RoutingRole::Id, routingRole), CHIP_NO_ERROR);

        NetworkName::TypeInfo::Type networkName;
        ASSERT_EQ(tester.ReadAttribute(NetworkName::Id, networkName), CHIP_NO_ERROR);

        PanId::TypeInfo::Type panId;
        ASSERT_EQ(tester.ReadAttribute(PanId::Id, panId), CHIP_NO_ERROR);

        ExtendedPanId::TypeInfo::Type extendedPanId;
        ASSERT_EQ(tester.ReadAttribute(ExtendedPanId::Id, extendedPanId), CHIP_NO_ERROR);

        MeshLocalPrefix::TypeInfo::Type meshLocalPrefix;
        ASSERT_EQ(tester.ReadAttribute(MeshLocalPrefix::Id, meshLocalPrefix), CHIP_NO_ERROR);

        NeighborTable::TypeInfo::DecodableType neighborTable;
        ASSERT_EQ(tester.ReadAttribute(NeighborTable::Id, neighborTable), CHIP_NO_ERROR);

        RouteTable::TypeInfo::DecodableType routeTable;
        ASSERT_EQ(tester.ReadAttribute(RouteTable::Id, routeTable), CHIP_NO_ERROR);

        PartitionId::TypeInfo::Type partitionId;
        ASSERT_EQ(tester.ReadAttribute(PartitionId::Id, partitionId), CHIP_NO_ERROR);

        Weighting::TypeInfo::Type weighting;
        ASSERT_EQ(tester.ReadAttribute(Weighting::Id, weighting), CHIP_NO_ERROR);

        Attributes::DataVersion::TypeInfo::Type dataVersion;
        ASSERT_EQ(tester.ReadAttribute(Attributes::DataVersion::Id, dataVersion), CHIP_NO_ERROR);

        StableDataVersion::TypeInfo::Type stableDataVersion;
        ASSERT_EQ(tester.ReadAttribute(StableDataVersion::Id, stableDataVersion), CHIP_NO_ERROR);

        LeaderRouterId::TypeInfo::Type leaderRouterId;
        ASSERT_EQ(tester.ReadAttribute(LeaderRouterId::Id, leaderRouterId), CHIP_NO_ERROR);

        SecurityPolicy::TypeInfo::DecodableType securityPolicy;
        ASSERT_EQ(tester.ReadAttribute(SecurityPolicy::Id, securityPolicy), CHIP_NO_ERROR);

        ChannelPage0Mask::TypeInfo::Type channelPage0Mask;
        ASSERT_EQ(tester.ReadAttribute(ChannelPage0Mask::Id, channelPage0Mask), CHIP_NO_ERROR);

        OperationalDatasetComponents::TypeInfo::DecodableType operationalDatasetComponents;
        ASSERT_EQ(tester.ReadAttribute(OperationalDatasetComponents::Id, operationalDatasetComponents), CHIP_NO_ERROR);

        ActiveNetworkFaultsList::TypeInfo::DecodableType activeNetworkFaultsList;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkFaultsList::Id, activeNetworkFaultsList), CHIP_NO_ERROR);

        ExtAddress::TypeInfo::Type extAddress;
        ASSERT_EQ(tester.ReadAttribute(ExtAddress::Id, extAddress), CHIP_NO_ERROR);

        Rloc16::TypeInfo::Type rloc16;
        ASSERT_EQ(tester.ReadAttribute(Rloc16::Id, rloc16), CHIP_NO_ERROR);

        OverrunCount::TypeInfo::Type overrunCount;
        ASSERT_EQ(tester.ReadAttribute(OverrunCount::Id, overrunCount), CHIP_NO_ERROR);

        threadNetworkDiagnostics.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
