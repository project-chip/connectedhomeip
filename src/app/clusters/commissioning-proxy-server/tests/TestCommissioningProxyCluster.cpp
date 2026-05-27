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
#include <ble/BleConfig.h> // for CONFIG_NETWORK_LAYER_BLE
#include <platform/CHIPDeviceConfig.h>
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

// Helper: read the Transport attribute (CapabilitiesBitmap) — the runtime
// source of truth for which transports the proxy supports.  A transport bit
// is set here iff the cluster's GetSupportedTransports() returns it for the
// current build flag / feature flag combination.
static chip::BitMask<CapabilitiesBitmap> ReadSupportedTransports(ClusterTester & tester)
{
    chip::BitMask<CapabilitiesBitmap> supported;
    EXPECT_EQ(tester.ReadAttribute(CPAttributes::Transport::Id, supported), CHIP_NO_ERROR);
    return supported;
}

// Convenience: skip a test when the named transport is absent from the Transport
// attribute (e.g. PAF-only test running against a BLE-only build of the cluster).
#define SKIP_IF_TRANSPORT_UNSUPPORTED(tester, transport)                                                                           \
    do                                                                                                                              \
    {                                                                                                                              \
        if (!ReadSupportedTransports(tester).Has(transport))                                                                       \
        {                                                                                                                          \
            GTEST_SKIP() << "Transport " #transport " not advertised by this build of CommissioningProxyCluster";                  \
        }                                                                                                                          \
    } while (0)

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

    uint8_t maxSessions = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::MaxSessions::Id, maxSessions), CHIP_NO_ERROR);
    EXPECT_EQ(maxSessions, mockDelegate.GetMaxSessions());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The Transport attribute SHALL advertise kBle iff the BLE stack is compiled
// into this build (CONFIG_NETWORK_LAYER_BLE — the BLE-wide GN arg
// chip_config_network_layer_ble).  kBle is not gated by any Feature bit per
// spec, so this holds regardless of the constructed Feature bits.
TEST_F(TestCommissioningProxyCluster, TestTransportAttribute_BleGatedByBuildFlag)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto supported = ReadSupportedTransports(tester);
#if CONFIG_NETWORK_LAYER_BLE
    EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kBle));
#else
    EXPECT_FALSE(supported.Has(CapabilitiesBitmap::kBle));
#endif

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The Transport attribute SHALL advertise kWiFiPAF only when WiFi-PAF is
// compiled into this build (CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF) AND the WI
// feature bit is set on this cluster instance.
TEST_F(TestCommissioningProxyCluster, TestTransportAttribute_WiFiPAFGatedByWIFeatureAndBuildFlag)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    // Without WI: kWiFiPAF must not be advertised.
    {
        CommissioningProxyCluster cluster(
            CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        auto supported = ReadSupportedTransports(tester);
        EXPECT_FALSE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With WI and the build flag on: kWiFiPAF must be advertised.
    {
        BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
        CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        auto supported = ReadSupportedTransports(tester);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
#else
        EXPECT_FALSE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
#endif
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// MaxSessions attribute must reflect the delegate's GetMaxSessions() value,
// not a hardcoded constant.
TEST_F(TestCommissioningProxyCluster, TestMaxSessionsAttributeReadsFromDelegate)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Default mMaxSessions == 1.
    uint8_t maxSessions = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::MaxSessions::Id, maxSessions), CHIP_NO_ERROR);
    EXPECT_EQ(maxSessions, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Spec-compliance attribute tests
// =============================================================================
// These tests directly assert spec-mandated attribute defaults and access
// semantics from src/app_clusters/CommissioningProxy.adoc (§ Attributes).

// ClusterRevision SHALL equal the highest value in the Revision History table
// (currently 1).  Source: zzz_generated/.../Metadata.h kRevision.
TEST_F(TestCommissioningProxyCluster, TestClusterRevisionEqualsOne)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t clusterRevision = 0;
    ASSERT_EQ(tester.ReadAttribute(chip::app::Clusters::Globals::Attributes::ClusterRevision::Id, clusterRevision),
              CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// FeatureMap SHALL reflect the feature bits the cluster was constructed with.
TEST_F(TestCommissioningProxyCluster, TestFeatureMapReflectsConfig)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface, Feature::kBackgroundScan);

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(chip::app::Clusters::Globals::Attributes::FeatureMap::Id, featureMap),
              CHIP_NO_ERROR);
    EXPECT_EQ(featureMap,
              static_cast<uint32_t>(Feature::kWiFiNetworkInterface) | static_cast<uint32_t>(Feature::kBackgroundScan));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: Transport access = "R V" (read-only).  Write attempts
// SHALL be rejected with UnsupportedWrite.
TEST_F(TestCommissioningProxyCluster, TestTransportAttribute_WriteRejected)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::Transport::Id,
                                        chip::BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: MaxSessions access = "R V" (read-only).  Write attempts
// SHALL be rejected with UnsupportedWrite.
TEST_F(TestCommissioningProxyCluster, TestMaxSessionsAttribute_WriteRejected)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::MaxSessions::Id, static_cast<uint8_t>(5));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: ScanMaxTime access = "RW VO" (writable).  After writing
// a new value the next read SHALL return it.
TEST_F(TestCommissioningProxyCluster, TestScanMaxTimeAttribute_WritableRoundTrip)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(
        CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto writeStatus = tester.WriteAttribute(CPAttributes::ScanMaxTime::Id, static_cast<uint8_t>(45));
    EXPECT_TRUE(writeStatus.IsSuccess());

    uint8_t scanMaxTime = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::ScanMaxTime::Id, scanMaxTime), CHIP_NO_ERROR);
    EXPECT_EQ(scanMaxTime, 45u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: CacheTimeout (BGS-only) fallback = 120, access = "RW VO".
TEST_F(TestCommissioningProxyCluster, TestCacheTimeoutAttribute_DefaultAndWritable)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, bgs, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Default per spec fallback column.
    uint16_t cacheTimeout = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::CacheTimeout::Id, cacheTimeout), CHIP_NO_ERROR);
    EXPECT_EQ(cacheTimeout, 120u);

    // Writable + reflected in next read.
    auto writeStatus = tester.WriteAttribute(CPAttributes::CacheTimeout::Id, static_cast<uint16_t>(60));
    EXPECT_TRUE(writeStatus.IsSuccess());
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::CacheTimeout::Id, cacheTimeout), CHIP_NO_ERROR);
    EXPECT_EQ(cacheTimeout, 60u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: NumCachedResults (BGS-only) fallback = 0.
TEST_F(TestCommissioningProxyCluster, TestNumCachedResultsAttribute_DefaultZero)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, bgs, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint8_t numCachedResults = 99; // pre-set to non-zero so we know read overwrites
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::NumCachedResults::Id, numCachedResults), CHIP_NO_ERROR);
    EXPECT_EQ(numCachedResults, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: MaxCachedResults (BGS-only) min 1.  Reads from delegate.
TEST_F(TestCommissioningProxyCluster, TestMaxCachedResultsAttribute_ReadsFromDelegate)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, bgs, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint8_t maxCachedResults = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::MaxCachedResults::Id, maxCachedResults), CHIP_NO_ERROR);
    EXPECT_EQ(maxCachedResults, mockDelegate.GetMaxCachedResults());
    EXPECT_GE(maxCachedResults, 1u); // spec: min 1

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: WiFiBand (WI-only) reflects delegate's supported bands.
TEST_F(TestCommissioningProxyCluster, TestWiFiBandAttribute_ReadsFromDelegate)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    chip::BitMask<WiFiBandBitmap> bands;
    bands.Set(WiFiBandBitmap::k2g4);
    bands.Set(WiFiBandBitmap::k5g);
    mockDelegate.SetSupportedWiFiBands(bands);

    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    chip::BitMask<WiFiBandBitmap> readBands;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::WiFiBand::Id, readBands), CHIP_NO_ERROR);
    EXPECT_TRUE(readBands.Has(WiFiBandBitmap::k2g4));
    EXPECT_TRUE(readBands.Has(WiFiBandBitmap::k5g));

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
    cmd.transport     = transport;
    cmd.discriminator = 0;
    cmd.vendorID      = chip::VendorId::Common;
    cmd.productID     = 0;
    cmd.timeout       = timeout;
    return cmd;
}

// Zero transport bits SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ZeroTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.transport.Set(CapabilitiesBitmap::kWiFiPAF); // now two bits set

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A single reserved bit in transport (a bit outside the spec-defined kBle /
// kWiFiPAF set) SHALL be rejected.  The cluster currently rejects this via
// HasExactlyOneBitSet → kValidTransportBits check (returns InvalidCommand or
// InvalidTransportType; either is spec-conforming for a malformed request).
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ReservedTransportBitOnly)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // 0x01 is a reserved bit (kBle=0x02, kWiFiPAF=0x08).
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(static_cast<CapabilitiesBitmap>(0x01));
    auto result                              = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    // Spec §10.5.7.1: invalid Transport → InvalidTransportType
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidTransportType);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport without the WI feature SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Whenever the proxy advertises kBle in the Transport attribute (i.e. BLE was
// compiled into this build of the cluster) ProxyConnectRequest with kBle
// SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_BleCapability)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBand present with a non-WiFiPAF transport SHALL return InvalidCommand
// (the WiFiBand field is only meaningful for the WiFiPAF transport per spec).
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithBleTransport)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    // Enable WI so the cluster has wiFiBand plumbing wired; the rejection here
    // is driven by the transport != kWiFiPAF check, not the WI feature bit.
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport with WI feature enabled — SHALL succeed and return a sessionId.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionId, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBand field present with WI feature enabled and band in supported set — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in wiFiBand SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    // bit 1 (0x02) is reserved.
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02)));

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBand not in the proxy's supported bands SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandNotInSupportedBands)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy only supports 2.4 GHz.
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    // Request 5 GHz — not in supported bands.
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g));

    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful ProxyConnectRequest the cluster state SHALL be kState_CPConnected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_StateTransitionOnSuccess)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
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

// Spec §10.5.7.1: "If MaxSessions are in use, a RESOURCE_EXHAUSTED status
// SHALL be returned."  Enforced generically by the cluster from the delegate's
// GetActiveSessionCount() vs GetMaxSessions(), so every delegate inherits the
// behaviour without having to remember the check itself.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ResourceExhaustedAtMaxSessions)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    // Mock defaults: GetMaxSessions() == 1.  Saturate the count so the next
    // connect request hits the MaxSessions gate.
    mockDelegate.SetActiveSessionCount(mockDelegate.GetMaxSessions());

    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::ResourceExhausted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Below MaxSessions: the cluster-level pre-check does not reject; the
// request is forwarded to the delegate and (for the mock) succeeds.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_BelowMaxSessionsSucceeds)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    mockDelegate.SetActiveSessionCount(0); // explicit; mock default is 0

    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));
    EXPECT_TRUE(result.IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec §10.5.7.1: "If Timeout expires, the connection attempt will be terminated
// and a TIMEOUT status SHALL be returned."  The cluster must propagate.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_DelegateTimeout_Propagated)
{
    struct TimeoutDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status
        ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan>, CapabilitiesBitmap, uint16_t, chip::VendorId, uint16_t, uint16_t,
                            WiFiBandBitmap, app::CommandHandler *, const DataModel::InvokeRequest &) override
        {
            return Protocols::InteractionModel::Status::Timeout;
        }
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, wi, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::Timeout);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful connect, disconnect with the returned sessionId SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_AfterConnect)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
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
        Protocols::InteractionModel::Status ProxyDisconnectRequest(uint16_t, chip::FabricIndex) override
        {
            return Protocols::InteractionModel::Status::Failure;
        }
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
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

// SessionId=0xFFFF with a pending connect SHALL return Success and cancel the connect.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_Success)
{
    struct PendingConnectDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex) override
        {
            cancelCalled    = true;
            lastFabricIndex = fabricIndex;
            return Protocols::InteractionModel::Status::Success;
        }
        bool cancelCalled                 = false;
        chip::FabricIndex lastFabricIndex = chip::kUndefinedFabricIndex;
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    tester.SetFabricIndex(2);
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 0xFFFF;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_TRUE(mockDelegate.cancelCalled);
    EXPECT_EQ(mockDelegate.lastFabricIndex, 2);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// SessionId=0xFFFF when already connected SHALL return InvalidInState.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_AlreadyConnected)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // CancelPendingConnect inherits InvalidInState from Delegate base (no pending connect).
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 0xFFFF;
    auto result   = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidInState);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// SessionId=0xFFFF from the wrong fabric SHALL return NotFound (fabric isolation).
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_WrongFabric)
{
    struct FabricCheckDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex) override
        {
            // Simulate a pending connect owned by fabric 1; reject any other fabric.
            if (fabricIndex != 1)
                return Protocols::InteractionModel::Status::NotFound;
            return Protocols::InteractionModel::Status::Success;
        }
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 0xFFFF;

    // Fabric 2 tries to cancel fabric 1's pending connect — SHALL be rejected.
    tester.SetFabricIndex(2);
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::NotFound);

    // Fabric 1 (the owner) cancels its own pending connect — SHALL succeed.
    tester.SetFabricIndex(1);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

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

// Whenever the proxy advertises kBle in the Transport attribute,
// ProxyScanRequest with kBle
// SHALL succeed regardless of which Feature bits the cluster was constructed
// with.  Skips when the build does not include BLE.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_BleNoFeaturesSucceeds)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kBle;

    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ProxyScanRequest with both kBle and kWiFiPAF in a single transport mask
// (with the WI feature enabled) SHALL succeed.  Skips when either transport
// is missing from the build.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_BleAndWiFiPAFTogether)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    Commands::ProxyScanRequest::Type command;
    command.transport.Set(CapabilitiesBitmap::kBle);
    command.transport.Set(CapabilitiesBitmap::kWiFiPAF);

    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
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
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
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

// Spec §10.5.7.7: "If no transport connection with the specified SessionId
// exists on the proxy, or if the fabric associated with the connection does
// not match the fabric sending the command, the command SHALL be rejected
// with a status of NOT_FOUND."  This is delegate-side state; the cluster
// must propagate.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_DelegateNotFound_Propagated)
{
    struct NotFoundDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status ProxyMessageRequest(uint16_t, chip::Optional<chip::ByteSpan>, uint8_t,
                                                                  app::CommandHandler *,
                                                                  const DataModel::InvokeRequest &) override
        {
            return Protocols::InteractionModel::Status::NotFound;
        }
    } mockDelegate;

    TestServerClusterContext context;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    static const uint8_t kMsg[] = { 0xFF };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionId       = 9999;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::NotFound);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec §10.5.7.7: "If another ProxyMessageRequest referencing the same
// SessionId is still outstanding, the command SHALL be rejected with a status
// of BUSY."  Delegate-side bookkeeping; cluster must propagate.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_DelegateBusy_Propagated)
{
    struct BusyDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status ProxyMessageRequest(uint16_t, chip::Optional<chip::ByteSpan>, uint8_t,
                                                                  app::CommandHandler *,
                                                                  const DataModel::InvokeRequest &) override
        {
            return Protocols::InteractionModel::Status::Busy;
        }
    } mockDelegate;

    TestServerClusterContext context;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    static const uint8_t kMsg[] = { 0xFF };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionId       = 1;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::Busy);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec §10.5.7.2: "If no transport connection with the specified session id
// exists on the proxy, ... the command SHALL be rejected with a status of
// NOT_FOUND."  Delegate returns NotFound; cluster must propagate.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_DelegateNotFound_Propagated)
{
    struct NotFoundDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status ProxyDisconnectRequest(uint16_t, chip::FabricIndex) override
        {
            return Protocols::InteractionModel::Status::NotFound;
        }
    } mockDelegate;

    TestServerClusterContext context;
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, BitMask<Feature>{}, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionId = 1234; // unknown to the delegate

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::NotFound);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyBackgroundScanStartRequest — parameter validation tests
// =============================================================================

// Helper: build a minimal valid ProxyBackgroundScanStartRequest.
static Commands::ProxyBackGroundScanStartRequest::Type
MakeBgScanStartRequest(CapabilitiesBitmap transport, uint16_t timeout = 30,
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

// kBle is not gated by any Feature bit in the spec; whenever the proxy
// advertises kBle in the Transport attribute, ProxyBackGroundScanStartRequest
// with kBle SHALL succeed (only the BGS feature is required for the command
// to be accepted at all).
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_BleNoExtraFeaturesSucceeds)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kBle)).IsSuccess());

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
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
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
    chip::BitMask<WiFiBandBitmap> bothBands;
    bothBands.Set(WiFiBandBitmap::k2g4);
    bothBands.Set(WiFiBandBitmap::k5g);
    mockDelegate.SetSupportedWiFiBands(bothBands);
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30, chip::MakeOptional(bothBands));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with a WiFiBand bit not in GetSupportedWiFiBands() SHALL return INVALID_TRANSPORT_TYPE.
// Proxy only supports 2.4 GHz; requesting 5 GHz must be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_UnsupportedWiFiBand)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    mockDelegate.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
                                      chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g)));
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyBackgroundScanStopRequest — parameter validation tests
// =============================================================================

// Helper: build a ProxyBackgroundScanStopRequest.
static Commands::ProxyBackGroundScanStopRequest::Type
MakeBgScanStopRequest(CapabilitiesBitmap transport, chip::Optional<chip::BitMask<WiFiBandBitmap>> wiFiBands = chip::NullOptional)
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

// kBle is not gated by any Feature bit in the spec; whenever the proxy
// advertises kBle in the Transport attribute, ProxyBackGroundScanStopRequest
// with kBle passes the cluster-level transport validation and is forwarded to
// the delegate.  The mock delegate returns Success, mirroring the WiFiPAF
// positive case below.  (NotFound for an unrecognised fabric is a
// delegate-level concern and is covered by the platform delegate's own tests.)
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_BleValid)
{
    TestServerClusterContext context;
    CommissioningProxyMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle)).IsSuccess());

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
    auto cmd =
        MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF, chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec §10.5.7.8: "If the NodeId and FabricId of the client do not match
// those used in a previous ProxyBackGroundScanStartRequest, the proxy SHALL
// take no action and the command SHALL be rejected with a status of NOT_FOUND."
// This is delegate-side state; the cluster must propagate the delegate's
// NotFound status.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_DelegateNotFound_Propagated)
{
    struct NotFoundDelegate : public CommissioningProxyMockDelegate
    {
        Protocols::InteractionModel::Status ProxyBackgroundScanStopRequest(CapabilitiesBitmap, WiFiBandBitmap, chip::FabricIndex,
                                                                            chip::NodeId) override
        {
            return Protocols::InteractionModel::Status::NotFound;
        }
    } mockDelegate;

    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(CommissioningProxyCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::NotFound);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
