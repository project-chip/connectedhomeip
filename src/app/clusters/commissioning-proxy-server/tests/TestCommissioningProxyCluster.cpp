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

#include "CommissioningProxyMockDelegate.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/clusters/commissioning-proxy-server/tests/CommissioningProxyMockDelegate.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CommissioningProxy/Attributes.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <clusters/CommissioningProxy/Metadata.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissioningProxy;
using namespace chip::app::Clusters::CommissioningProxy::Attributes;
using namespace chip::app::Clusters::CommissioningProxy::Commands;
using namespace chip::Testing;

namespace {
constexpr EndpointId kTestEndpointId = 1;
struct TestCommissioningProxyCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

namespace CPAttributes = chip::app::Clusters::CommissioningProxy::Attributes;

// =============================================================================
// Feature Tests
// =============================================================================
TEST_F(TestCommissioningProxyCluster, TestFeatures)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    // No features - only mandatory attributes
    {
        BitMask<Feature> noFeatures;
        CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CPAttributes::Transport::kMetadataEntry,
                                                CPAttributes::ScanMaxTime::kMetadataEntry,
                                                CPAttributes::MaxSessions::kMetadataEntry,
                                            }));

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedCommandsBuilder;
        EXPECT_EQ(expectedCommandsBuilder.AppendElements({
                      ProxyConnectRequest::kMetadataEntry,
                      ProxyDisconnectRequest::kMetadataEntry,
                      ProxyScanRequest::kMetadataEntry,
                      ProxyMessageRequest::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedCommandsBuilder.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Background Scan(BGS) Feature - BGS and mandatory attributes
    {
        BitMask<Feature> features(Feature::kBackgroundScan);
        CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CPAttributes::Transport::kMetadataEntry,
                                                CPAttributes::ScanMaxTime::kMetadataEntry,
                                                CPAttributes::MaxSessions::kMetadataEntry,
                                                CPAttributes::MaxCachedResults::kMetadataEntry,
                                                CPAttributes::NumCachedResults::kMetadataEntry,
                                                CPAttributes::CacheTimeout::kMetadataEntry,
                                                CPAttributes::CachedResults::kMetadataEntry,
                                            }));

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedCommandsBuilder;
        EXPECT_EQ(expectedCommandsBuilder.AppendElements({
                      ProxyConnectRequest::kMetadataEntry,
                      ProxyDisconnectRequest::kMetadataEntry,
                      ProxyScanRequest::kMetadataEntry,
                      ProxyBackGroundScanStartRequest::kMetadataEntry,
                      ProxyBackGroundScanStopRequest::kMetadataEntry,
                      ProxyMessageRequest::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedCommandsBuilder.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // WiFi Feature - Wi-Fi and mandatory attributes
    {
        BitMask<Feature> features(Feature::kWiFiNetworkInterface);
        CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CPAttributes::Transport::kMetadataEntry,
                                                CPAttributes::ScanMaxTime::kMetadataEntry,
                                                CPAttributes::MaxSessions::kMetadataEntry,
                                                CPAttributes::WiFiBand::kMetadataEntry,
                                            }));

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedCommandsBuilder;
        EXPECT_EQ(expectedCommandsBuilder.AppendElements({
                      ProxyConnectRequest::kMetadataEntry,
                      ProxyDisconnectRequest::kMetadataEntry,
                      ProxyScanRequest::kMetadataEntry,
                      ProxyMessageRequest::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedCommandsBuilder.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // All Features - All attributes
    {
        BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
        CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CPAttributes::Transport::kMetadataEntry,
                                                CPAttributes::ScanMaxTime::kMetadataEntry,
                                                CPAttributes::MaxSessions::kMetadataEntry,
                                                CPAttributes::MaxCachedResults::kMetadataEntry,
                                                CPAttributes::NumCachedResults::kMetadataEntry,
                                                CPAttributes::CacheTimeout::kMetadataEntry,
                                                CPAttributes::CachedResults::kMetadataEntry,
                                                CPAttributes::WiFiBand::kMetadataEntry,
                                            }));

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id), commandsBuilder),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedCommandsBuilder;
        EXPECT_EQ(expectedCommandsBuilder.AppendElements({
                      ProxyConnectRequest::kMetadataEntry,
                      ProxyDisconnectRequest::kMetadataEntry,
                      ProxyScanRequest::kMetadataEntry,
                      ProxyBackGroundScanStartRequest::kMetadataEntry,
                      ProxyBackGroundScanStopRequest::kMetadataEntry,
                      ProxyMessageRequest::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedCommandsBuilder.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// =============================================================================
// Startup Tests
// =============================================================================
TEST_F(TestCommissioningProxyCluster, TestStartupFailsWithMismatchedEndpointId)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    constexpr EndpointId kClusterEndpointId  = 1;
    constexpr EndpointId kDelegateEndpointId = 2;

    // Create cluster with one endpoint ID
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kClusterEndpointId, noFeatures, mockDelegate));

    // Set delegate to a different endpoint ID
    mockDelegate.SetEndpointId(kDelegateEndpointId);

    // Startup should fail because endpoint IDs don't match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCommissioningProxyCluster, TestStartupSucceedsWithMatchingEndpointId)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    constexpr EndpointId kEndpointId = 1;

    // Create cluster with endpoint ID
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kEndpointId, noFeatures, mockDelegate));

    // Delegate endpoint ID is set in constructor, so they should match
    EXPECT_EQ(mockDelegate.GetEndpointId(), kEndpointId);

    // Startup should succeed because endpoint IDs match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Attribute Tests
// =============================================================================
TEST_F(TestCommissioningProxyCluster, TestMandatoryAttributes)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint8_t scanMaxTime = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::ScanMaxTime::Id, scanMaxTime), CHIP_NO_ERROR);
    EXPECT_EQ(scanMaxTime, 120);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyConnectRequest Command Tests
// =============================================================================

// Helper: build a minimal valid ProxyConnectRequest for a single transport.
static Commands::ProxyConnectRequest::Type MakeConnectRequest(CapabilitiesBitmap transport, uint16_t timeout = 30)
{
    Commands::ProxyConnectRequest::Type cmd;
    cmd.address.SetNull();
    cmd.transport      = transport;
    cmd.discriminator  = 0;
    cmd.vendorId       = chip::VendorId::Common;
    cmd.productId      = 0;
    cmd.timeout        = timeout;
    return cmd;
}

// Zero transport bits SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ZeroTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(static_cast<CapabilitiesBitmap>(0));

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Multiple transport bits set SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_MultipleTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.transport.Set(CapabilitiesBitmap::kWiFiPAF); // now two bits set

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport without the WI feature SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle without a BLE feature flag SHALL return InvalidTransportType.
// kBle is a valid spec-defined transport; once Feature::kBleInterface is added
// to the cluster, a positive test should be added alongside this one.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_BleWithoutBleFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport with WI feature enabled — SHALL succeed and return a sessionId.
// (kBle would also succeed here once Feature::kBleInterface is defined; transport
// acceptance is driven by GetSupportedTransports(), not per-transport hardcoding.)
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBand field present with WI feature enabled — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful ProxyConnectRequest the cluster state SHALL be kState_CPConnected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_StateTransitionOnSuccess)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    ClusterTester tester(cluster);
    EXPECT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A failed ProxyConnectRequest SHALL NOT change the cluster state.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_StateUnchangedOnFailure)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // WiFiPAF without WI feature → failure
    EXPECT_FALSE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyDisconnectRequest Command Tests
// =============================================================================

// After a successful connect, disconnect with the returned sessionId SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_AfterConnect)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Establish a proxy session.
    auto connectResult = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));
    ASSERT_TRUE(connectResult.IsSuccess());
    ASSERT_TRUE(connectResult.response.has_value());

    // Disconnect the session using the sessionId from the response.
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = connectResult.response->sessionId;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful disconnect, the cluster state SHALL revert to kState_CPDisconnected.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_StateTransitionToDisconnected)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ASSERT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 1;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    // State SHALL be Disconnected after a successful disconnect.
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// When the delegate fails to disconnect, the command SHALL fail and the cluster
// state SHALL remain Connected (session not cleaned up).
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_DelegateFailurePreservesState)
{
    // Subclass that rejects all disconnect requests.
    struct FailDisconnectDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status ProxyDisconnectRequest(uint16_t) override
        {
            return Protocols::InteractionModel::Status::Failure;
        }
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ASSERT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 1;
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    // State SHALL remain Connected since the delegate rejected the disconnect.
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyScanRequest Command Tests
// =============================================================================

// Zero transport bits SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ZeroTransport)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = static_cast<CapabilitiesBitmap>(0);

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ReservedTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // kWiFiPAF(0x08) | reserved(0x01) = 0x09 contains a reserved bit.
    Commands::ProxyScanRequest::Type command;
    command.transport = static_cast<CapabilitiesBitmap>(0x09);

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle without a BLE feature flag SHALL return InvalidTransportType.
// kBle is a valid spec-defined transport; once Feature::kBleInterface is added
// to the cluster, a positive test should be added alongside this one.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_BleWithoutBleFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kBle;

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF transport without the WI feature SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBands field present without the WI feature SHALL be rejected.
// The kWiFiPAF transport check fires first (InvalidTransportType) since
// kWiFiPAF itself requires WI; the wiFiBands guard is defence-in-depth.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiBandWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in wiFiBands SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // k2g4=0x01, k5g=0x04 are valid; bit 1 (0x02) is reserved.
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02)));

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands containing bits not in the proxy's supported bands SHALL return
// InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiBandNotInSupportedBands)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy only supports 2.4 GHz.
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Request 5 GHz — not in supported bands.
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g));

    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with WI feature enabled and no wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiPAFWithWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    // No wiFiBands — skips the band validation entirely.

    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with WI feature and a wiFiBands value within supported bands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ValidWiFiBandInSupportedBands)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy supports both 2.4 GHz and 5 GHz.
    chip::BitMask<WiFiBandBitmap> bothBands;
    bothBands.Set(WiFiBandBitmap::k2g4);
    bothBands.Set(WiFiBandBitmap::k5g);
    mockDelegate.SetSupportedWiFiBands(bothBands);

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyMessageRequest Command Tests
// =============================================================================

// ProxyMessageRequest with a non-null message SHALL succeed and return a
// ProxyMessageResponse carrying the same sessionId.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_WithMessage)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    static const uint8_t kMsg[] = { 0x01, 0x02, 0x03, 0x04 };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionId       = 1;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A null ProxyMessageRequest.message SHALL succeed and
// return a ProxyMessageResponse with a null message (no data from commissionee).
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_NullMessage_Poll)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionId       = 1;
    cmd.responseTimeout = 5;
    cmd.message.SetNull();

    auto result = tester.Invoke(cmd);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);
    // Null response message signals no pending data from commissionee.
    EXPECT_TRUE(result.response->message.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The ProxyMessageResponse sessionId SHALL match the sessionId from the request.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_SessionIdEchoed)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    static const uint8_t kMsg[] = { 0xAA, 0xBB };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionId       = 42;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 42u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyBackgroundScanStartRequest — parameter validation tests
// =============================================================================

// Helper: build a minimal valid ProxyBackgroundScanStartRequest.
static Commands::ProxyBackGroundScanStartRequest::Type MakeBgScanStartRequest(
    CapabilitiesBitmap transport, uint16_t timeout = 30,
    chip::Optional<chip::BitMask<WiFiBandBitmap>> wiFiBands = chip::NullOptional)
{
    Commands::ProxyBackGroundScanStartRequest::Type cmd;
    cmd.transport = transport;
    cmd.timeout   = timeout;
    cmd.wiFiBands = wiFiBands;
    return cmd;
}

// transport=0 SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ZeroTransport)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStartRequest(static_cast<CapabilitiesBitmap>(0))).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ReservedTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // CapabilitiesBitmap: kBle=0x02, kWiFiPAF=0x08; all other bits are reserved.
    // 0x09 = kWiFiPAF(0x08) | reserved(0x01) → contains a reserved bit.
    EXPECT_FALSE(tester.Invoke(MakeBgScanStartRequest(static_cast<CapabilitiesBitmap>(0x09))).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle without a BLE feature flag SHALL return InvalidTransportType.
// kBle is a valid spec-defined transport; once Feature::kBleInterface is added
// to the cluster, a positive test should be added alongside this one.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_BleWithoutBleFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF without WI feature SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands with reserved bits SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // k2g4=0x01, k5g=0x04 are valid; bit 1 (0x02) is reserved.
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02))));
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with wiFiBands but no WI feature SHALL return InvalidTransportType.
// Note: the kWiFiPAF transport check fires before the wiFiBands check because
// kWiFiPAF itself requires the WI feature.  There is no transport that is both
// valid without WI and also permits a wiFiBands field, so the "wiFiBands requires
// WI feature" guard in the cluster code is defence-in-depth rather than a
// reachable path for BackgroundScanStart.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_WiFiPAFAndBandWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF with WI feature, no wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ValidWiFiPAF)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_TRUE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF with k2g4 wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ValidWiFiPAF_2g4Band)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF with k2g4|k5g wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ValidWiFiPAF_BothBands)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    chip::BitMask<WiFiBandBitmap> bothBands;
    bothBands.Set(WiFiBandBitmap::k2g4);
    bothBands.Set(WiFiBandBitmap::k5g);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30, chip::MakeOptional(bothBands));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyBackgroundScanStopRequest — parameter validation tests
// =============================================================================

// Helper: build a ProxyBackgroundScanStopRequest.
static Commands::ProxyBackGroundScanStopRequest::Type MakeBgScanStopRequest(
    CapabilitiesBitmap transport,
    chip::Optional<chip::BitMask<WiFiBandBitmap>> wiFiBands = chip::NullOptional)
{
    Commands::ProxyBackGroundScanStopRequest::Type cmd;
    cmd.transport = transport;
    cmd.wiFiBands = wiFiBands;
    return cmd;
}

// transport=0 and no wiFiBands SHALL return InvalidCommand (nothing to stop).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ZeroTransportNoWiFiBands)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0))).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ReservedTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // CapabilitiesBitmap: kBle=0x02, kWiFiPAF=0x08; all other bits are reserved.
    // 0x09 = kWiFiPAF(0x08) | reserved(0x01) → contains a reserved bit.
    EXPECT_FALSE(tester.Invoke(MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0x09))).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle without a BLE feature flag SHALL return InvalidTransportType.
// kBle is a valid spec-defined transport; once Feature::kBleInterface is added
// to the cluster, a positive test should be added alongside this one.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_BleWithoutBleFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF without WI feature SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_FALSE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands with reserved bits SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Bit 1 (0x02) is reserved in WiFiBandBitmap.
    auto cmd = MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF,
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02))));
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands without WI feature SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_WiFiBandWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // transport=0 alone is invalid, so provide kWiFiPAF (which also fails,
    // but for InvalidTransportType). Separately test wiFiBands-only path
    // with transport=0 and a wiFiBands value without WI feature.
    auto cmd = MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0),
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// transport=0 with valid wiFiBands and WI feature — SHALL succeed
// (band-only stop is valid per spec §10.5.7.7).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_BandOnlyStop_Valid)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0),
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF stop without wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ValidWiFiPAF)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    EXPECT_TRUE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF stop with k5g wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ValidWiFiPAF_5gBand)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF,
        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
