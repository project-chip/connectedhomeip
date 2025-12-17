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

#include <app/ConcreteCommandPath.h>
#include <app/clusters/wifi-network-diagnostics-server/WiFiNetworkDiagnosticsCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WiFiNetworkDiagnostics/Enums.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

#include <cmath>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using namespace chip::app::DataModel;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

struct TestWiFiNetworkDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestWiFiNetworkDiagnosticsCluster, CompileTest)
{
    class NullProvider : public DeviceLayer::DiagnosticDataProvider
    {
    };

    NullProvider nullProvider;
    WiFiDiagnosticsServerCluster cluster(kRootEndpointId, nullProvider, WiFiDiagnosticsServerCluster::OptionalAttributeSet(),
                                         BitFlags<WiFiNetworkDiagnostics::Feature>(0));

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, WiFiNetworkDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestWiFiNetworkDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };

        NullProvider nullProvider;
        WiFiDiagnosticsServerCluster cluster(kRootEndpointId, nullProvider, WiFiDiagnosticsServerCluster::OptionalAttributeSet(),
                                             BitFlags<WiFiNetworkDiagnostics::Feature>(0));

        // without any enabled attributes, no commands are accepted
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));

        // Everything is unimplemented, so attributes are the mandatory and global ones.
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                WiFiNetworkDiagnostics::Attributes::Bssid::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::SecurityType::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::WiFiVersion::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::ChannelNumber::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::Rssi::kMetadataEntry,
                                            }));
    }

    {
        class ErrorCountsProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiOverrunCount(uint64_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }
        };

        ErrorCountsProvider errorCountsProvider;
        WiFiDiagnosticsServerCluster cluster(
            kRootEndpointId, errorCountsProvider, WiFiDiagnosticsServerCluster::OptionalAttributeSet(),
            BitFlags<WiFiNetworkDiagnostics::Feature>(WiFiNetworkDiagnostics::Feature::kErrorCounts));

        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      WiFiNetworkDiagnostics::Commands::ResetCounts::kMetadataEntry,
                                                  }));

        DataModel::InvokeRequest request2;
        request2.path =
            ConcreteCommandPath(kRootEndpointId, WiFiNetworkDiagnostics::Id, WiFiNetworkDiagnostics::Commands::ResetCounts::Id);
        TLV::TLVReader tlvReader2;
        ASSERT_EQ(cluster.InvokeCommand(request2, tlvReader2, nullptr), Protocols::InteractionModel::Status::Success);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                WiFiNetworkDiagnostics::Attributes::Bssid::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::SecurityType::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::WiFiVersion::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::ChannelNumber::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::Rssi::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::BeaconLostCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::OverrunCount::kMetadataEntry,
                                            }));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetWiFiBssId(MutableByteSpan & value) override
            {
                static constexpr uint8_t kBssId[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
                return CopySpanToMutableSpan(ByteSpan(kBssId), value);
            }
            CHIP_ERROR GetWiFiSecurityType(WiFiNetworkDiagnostics::SecurityTypeEnum & v) override
            {
                v = WiFiNetworkDiagnostics::SecurityTypeEnum::kWpa2;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiChannelNumber(uint16_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiRssi(int8_t & v) override
            {
                v = -123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiBeaconRxCount(uint32_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & v) override
            {
                v = 345;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & v) override
            {
                v = 456;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiPacketUnicastRxCount(uint32_t & v) override
            {
                v = 567;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & v) override
            {
                v = 678;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiCurrentMaxRate(uint64_t & v) override
            {
                v = 789;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetWiFiOverrunCount(uint64_t & v) override
            {
                v = 890;
                return CHIP_NO_ERROR;
            }
        };

        AllProvider allProvider;
        BitFlags<WiFiNetworkDiagnostics::Feature> features;
        features.Set(WiFiNetworkDiagnostics::Feature::kErrorCounts);
        features.Set(WiFiNetworkDiagnostics::Feature::kPacketCounts);
        WiFiDiagnosticsServerCluster cluster(
            kRootEndpointId, allProvider, WiFiDiagnosticsServerCluster::OptionalAttributeSet().Set<CurrentMaxRate::Id>(), features);

        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      WiFiNetworkDiagnostics::Commands::ResetCounts::kMetadataEntry,
                                                  }));

        // Test all WiFi-specific attributes
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                WiFiNetworkDiagnostics::Attributes::Bssid::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::SecurityType::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::WiFiVersion::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::ChannelNumber::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::Rssi::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::BeaconLostCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::BeaconRxCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::PacketMulticastRxCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::PacketMulticastTxCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::PacketUnicastRxCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::PacketUnicastTxCount::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::CurrentMaxRate::kMetadataEntry,
                                                WiFiNetworkDiagnostics::Attributes::OverrunCount::kMetadataEntry,
                                            }));

        // Test that the provider methods are working correctly by directly accessing the provider
        uint8_t bssIdBuffer[6];
        MutableByteSpan bssId(bssIdBuffer);
        EXPECT_EQ(allProvider.GetWiFiBssId(bssId), CHIP_NO_ERROR);
        EXPECT_EQ(bssId.size(), 6u);
        static constexpr uint8_t kExpectedBssId[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
        EXPECT_EQ(memcmp(bssId.data(), kExpectedBssId, bssId.size()), 0);

        WiFiNetworkDiagnostics::SecurityTypeEnum securityType;
        EXPECT_EQ(allProvider.GetWiFiSecurityType(securityType), CHIP_NO_ERROR);
        EXPECT_EQ(securityType, WiFiNetworkDiagnostics::SecurityTypeEnum::kWpa2);

        uint16_t channelNumber;
        EXPECT_EQ(allProvider.GetWiFiChannelNumber(channelNumber), CHIP_NO_ERROR);
        EXPECT_EQ(channelNumber, 123u);

        int8_t rssi;
        EXPECT_EQ(allProvider.GetWiFiRssi(rssi), CHIP_NO_ERROR);
        EXPECT_EQ(rssi, -123);

        uint32_t beaconRxCount;
        EXPECT_EQ(allProvider.GetWiFiBeaconRxCount(beaconRxCount), CHIP_NO_ERROR);
        EXPECT_EQ(beaconRxCount, 123u);

        uint32_t beaconLostCount;
        EXPECT_EQ(allProvider.GetWiFiBeaconLostCount(beaconLostCount), CHIP_NO_ERROR);
        EXPECT_EQ(beaconLostCount, 234u);

        uint32_t packetMulticastRxCount;
        EXPECT_EQ(allProvider.GetWiFiPacketMulticastRxCount(packetMulticastRxCount), CHIP_NO_ERROR);
        EXPECT_EQ(packetMulticastRxCount, 345u);

        uint32_t packetMulticastTxCount;
        EXPECT_EQ(allProvider.GetWiFiPacketMulticastTxCount(packetMulticastTxCount), CHIP_NO_ERROR);
        EXPECT_EQ(packetMulticastTxCount, 456u);

        uint32_t packetUnicastRxCount;
        EXPECT_EQ(allProvider.GetWiFiPacketUnicastRxCount(packetUnicastRxCount), CHIP_NO_ERROR);
        EXPECT_EQ(packetUnicastRxCount, 567u);

        uint32_t packetUnicastTxCount;
        EXPECT_EQ(allProvider.GetWiFiPacketUnicastTxCount(packetUnicastTxCount), CHIP_NO_ERROR);
        EXPECT_EQ(packetUnicastTxCount, 678u);

        uint64_t currentMaxRate;
        EXPECT_EQ(allProvider.GetWiFiCurrentMaxRate(currentMaxRate), CHIP_NO_ERROR);
        EXPECT_EQ(currentMaxRate, 789u);

        uint64_t overrunCount;
        EXPECT_EQ(allProvider.GetWiFiOverrunCount(overrunCount), CHIP_NO_ERROR);
        EXPECT_EQ(overrunCount, 890u);
    }
}

} // namespace
