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

    // Test 1: No features - only mandatory attributes
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

    // Test 2: Background Scan(BGS) Feature - BGS and mandatory attributes
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

    // Test 3: WiFi Feature - Wi-Fi and mandatory attributes
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

        // Test 4: All Features - All attributes
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
//  CPAttributes::Transport::kMetadataEntry,
//  CPAttributes::ScanMaxTime::kMetadataEntry,
//  CPAttributes::MaxSessions::kMetadataEntry,
//  CPAttributes::MaxCachedResults::kMetadataEntry,
//  CPAttributes::NumCachedResults::kMetadataEntry,
//  CPAttributes::CacheTimeout::kMetadataEntry,
//  CPAttributes::CachedResults::kMetadataEntry,
//  CPAttributes::WiFiBand::kMetadataEntry,
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
// ProxyScanRequest Command Tests
// =============================================================================
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(WiFiBandBitmap::k2g4);
    // Wi-Fi Feature not supported, expect to fail
    EXPECT_FALSE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    // BLE is Supported
    command.transport = CapabilitiesBitmap::kBle;
    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

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

// WiFiBand field present without the WI feature SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid BLE transport — SHALL succeed and return a ProxyConnectResponse with a sessionId.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ValidBLETransport)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid BLE transport with a non-zero timeout — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ValidBLEWithTimeout)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle, 60));

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport with WI feature enabled — SHALL succeed.
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
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    ClusterTester tester(cluster);
    EXPECT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());

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

} // namespace
