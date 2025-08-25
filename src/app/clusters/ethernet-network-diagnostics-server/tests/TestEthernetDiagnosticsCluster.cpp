/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/EthernetNetworkDiagnostics/Enums.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

#include <cmath>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

struct TestEthernetDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestEthernetDiagnosticsCluster, CompileTest)
{
    class NullProvider : public DeviceLayer::DiagnosticDataProvider
    {
    };

    const EthernetDiagnosticsServerCluster::OptionalAttributeSet optionalAttributeSet;

    NullProvider nullProvider;
    EthernetDiagnosticsServerCluster cluster(nullProvider, {}, optionalAttributeSet);

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, EthernetNetworkDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestEthernetDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };

        const EthernetDiagnosticsServerCluster::OptionalAttributeSet optionalAttributeSet;

        NullProvider nullProvider;
        EthernetDiagnosticsServerCluster cluster(nullProvider, {}, optionalAttributeSet);

        // without any enabled attributes, no commands are accepted
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), commandsBuilder),
                  CHIP_NO_ERROR);
        ASSERT_EQ(commandsBuilder.TakeBuffer().size(), 0u);

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), attributesBuilder),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), DefaultServerCluster::GlobalAttributes()));
    }

    {
        class ResetCountsProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetEthPacketRxCount(uint64_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }

            CHIP_ERROR GetEthPacketTxCount(uint64_t & v) override
            {
                v = 345;
                return CHIP_NO_ERROR;
            }
        };

        const EthernetDiagnosticsServerCluster::OptionalAttributeSet optionalAttributeSet =
            EthernetDiagnosticsServerCluster::OptionalAttributeSet();

        ResetCountsProvider resetCountsProvider;
        EthernetDiagnosticsServerCluster cluster(
            resetCountsProvider,
            BitFlags<EthernetNetworkDiagnostics::Feature>{ EthernetNetworkDiagnostics::Feature::kPacketCounts },
            optionalAttributeSet);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, EthernetNetworkDiagnostics::Commands::ResetCounts::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  EthernetNetworkDiagnostics::Commands::ResetCounts::kMetadataEntry.GetInvokePrivilege());

        // Test with PacketCount enabled
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), attributesBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({ EthernetNetworkDiagnostics::Attributes::PacketRxCount::kMetadataEntry,
                                                   EthernetNetworkDiagnostics::Attributes::PacketTxCount::kMetadataEntry }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetEthPHYRate(app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & v) override
            {
                v = EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthFullDuplex(bool & v) override
            {
                v = true;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthCarrierDetect(bool & v) override
            {
                v = true;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthTimeSinceReset(uint64_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthPacketRxCount(uint64_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthPacketTxCount(uint64_t & v) override
            {
                v = 345;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthTxErrCount(uint64_t & v) override
            {
                v = 456;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthCollisionCount(uint64_t & v) override
            {
                v = 567;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetEthOverrunCount(uint64_t & v) override
            {
                v = 678;
                return CHIP_NO_ERROR;
            }
        };

        const EthernetDiagnosticsServerCluster::OptionalAttributeSet optionalAttributeSet =
            EthernetDiagnosticsServerCluster::OptionalAttributeSet()
                .Set<EthernetNetworkDiagnostics::Attributes::CarrierDetect::Id>()
                .Set<EthernetNetworkDiagnostics::Attributes::FullDuplex::Id>()
                .Set<EthernetNetworkDiagnostics::Attributes::PHYRate::Id>()
                .Set<EthernetNetworkDiagnostics::Attributes::TimeSinceReset::Id>();

        AllProvider allProvider;
        EthernetDiagnosticsServerCluster cluster(
            allProvider, { EthernetNetworkDiagnostics::Feature::kPacketCounts, EthernetNetworkDiagnostics::Feature::kErrorCounts },
            optionalAttributeSet);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, EthernetNetworkDiagnostics::Commands::ResetCounts::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  EthernetNetworkDiagnostics::Commands::ResetCounts::kMetadataEntry.GetInvokePrivilege());

        // Test all ethernet-specific attributes
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, EthernetNetworkDiagnostics::Id), attributesBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      EthernetNetworkDiagnostics::Attributes::CarrierDetect::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::CollisionCount::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::FullDuplex::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::OverrunCount::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::PacketRxCount::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::PacketTxCount::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::PHYRate::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::TimeSinceReset::kMetadataEntry,
                      EthernetNetworkDiagnostics::Attributes::TxErrCount::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

        // Test that the provider methods are working correctly by directly accessing the provider
        uint64_t value;
        EXPECT_EQ(allProvider.GetEthTimeSinceReset(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 123u);

        EXPECT_EQ(allProvider.GetEthPacketRxCount(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 234u);

        EXPECT_EQ(allProvider.GetEthPacketTxCount(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 345u);

        EXPECT_EQ(allProvider.GetEthTxErrCount(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 456u);

        EXPECT_EQ(allProvider.GetEthCollisionCount(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 567u);

        EXPECT_EQ(allProvider.GetEthOverrunCount(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 678u);

        EthernetNetworkDiagnostics::PHYRateEnum phyRate;
        EXPECT_EQ(allProvider.GetEthPHYRate(phyRate), CHIP_NO_ERROR);
        EXPECT_EQ(phyRate, EthernetNetworkDiagnostics::PHYRateEnum::kRate10M);

        bool boolValue;
        EXPECT_EQ(allProvider.GetEthFullDuplex(boolValue), CHIP_NO_ERROR);
        EXPECT_EQ(boolValue, true);

        EXPECT_EQ(allProvider.GetEthCarrierDetect(boolValue), CHIP_NO_ERROR);
        EXPECT_EQ(boolValue, true);
    }
}

} // namespace
