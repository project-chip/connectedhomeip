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

#include "CommissioningProxyMockTransport.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/clusters/commissioning-proxy-server/tests/CommissioningProxyMockTransport.h>
#include <platform/CommissionableDataProvider.h> // for kMaxDiscriminatorValue
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
using chip::Protocols::InteractionModel::ClusterStatusCode;

namespace {
constexpr EndpointId kTestEndpointId = 1;
struct TestCommissioningProxyCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}

    // Mock transports available to every test. A transport is "supported" (advertised
    // in the Transport attribute) iff it is registered, so RegisterMocks() makes both
    // BLE and Wi-Fi PAF available; tests that need only one register it directly.
    CommissioningProxyMockTransport mockBle{ CapabilitiesBitmap::kBle };
    CommissioningProxyMockTransport mockPaf{ CapabilitiesBitmap::kWiFiPAF };

    void RegisterMocks(CommissioningProxyCluster & cluster)
    {
        cluster.RegisterTransport(mockBle);
        cluster.RegisterTransport(mockPaf);
    }
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
    do                                                                                                                             \
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

    // No features - only mandatory attributes
    {
        BitMask<Feature> noFeatures;
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(noFeatures));
        RegisterMocks(cluster);
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
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
        RegisterMocks(cluster);
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

    // WiFi Feature - mandatory attributes plus WiFiBand. WiFiBand is [WI]
    // (optional under WI); this implementation always exposes it when WI is set.
    {
        BitMask<Feature> features(Feature::kWiFiNetworkInterface);
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
        RegisterMocks(cluster);
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
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
        RegisterMocks(cluster);
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
TEST_F(TestCommissioningProxyCluster, TestStartupSucceeds)
{
    TestServerClusterContext context;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(noFeatures));
    RegisterMocks(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Attribute Tests
// =============================================================================
TEST_F(TestCommissioningProxyCluster, TestMandatoryAttributes)
{
    TestServerClusterContext context;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(noFeatures));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // ScanMaxTime has no spec fallback; the cluster uses 10 s as a practical default.
    uint8_t scanMaxTime = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::ScanMaxTime::Id, scanMaxTime), CHIP_NO_ERROR);
    EXPECT_EQ(scanMaxTime, 10);

    uint8_t maxSessions = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::MaxSessions::Id, maxSessions), CHIP_NO_ERROR);
    EXPECT_EQ(maxSessions, cluster.GetMaxSessions());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The Transport attribute SHALL advertise exactly the transports that have a
// registered driver. kBle is not gated by any Feature bit per spec.
TEST_F(TestCommissioningProxyCluster, TestTransportAttribute_ReflectsRegisteredTransports)
{
    TestServerClusterContext context;

    // Register only BLE: Transport advertises kBle, not kWiFiPAF.
    {
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
        cluster.RegisterTransport(mockBle);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        auto supported = ReadSupportedTransports(tester);
        EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kBle));
        EXPECT_FALSE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Register only Wi-Fi PAF: Transport advertises kWiFiPAF, not kBle. WiFiPAF is
    // independent of the WI feature (WI only gates WiFiBand), so no feature is set.
    {
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
        cluster.RegisterTransport(mockPaf);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        auto supported = ReadSupportedTransports(tester);
        EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
        EXPECT_FALSE(supported.Has(CapabilitiesBitmap::kBle));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Register both: Transport advertises both.
    {
        CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
        RegisterMocks(cluster);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        auto supported = ReadSupportedTransports(tester);
        EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kBle));
        EXPECT_TRUE(supported.Has(CapabilitiesBitmap::kWiFiPAF));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// MaxSessions attribute must reflect the configured GetMaxSessions() value,
// not a hardcoded constant.
TEST_F(TestCommissioningProxyCluster, TestMaxSessionsAttributeReadsFromConfig)
{
    TestServerClusterContext context;
    BitMask<Feature> noFeatures;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(noFeatures));
    RegisterMocks(cluster);
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
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t clusterRevision = 0;
    ASSERT_EQ(tester.ReadAttribute(chip::app::Clusters::Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 1u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// FeatureMap SHALL reflect the feature bits the cluster was constructed with.
TEST_F(TestCommissioningProxyCluster, TestFeatureMapReflectsConfig)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface, Feature::kBackgroundScan);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(chip::app::Clusters::Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kWiFiNetworkInterface) | static_cast<uint32_t>(Feature::kBackgroundScan));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: Transport access = "R V" (read-only).  Write attempts
// SHALL be rejected with UnsupportedWrite.
TEST_F(TestCommissioningProxyCluster, TestTransportAttribute_WriteRejected)
{
    TestServerClusterContext context;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(wi));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::Transport::Id, chip::BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: MaxSessions access = "R V" (read-only).  Write attempts
// SHALL be rejected with UnsupportedWrite.
TEST_F(TestCommissioningProxyCluster, TestMaxSessionsAttribute_WriteRejected)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
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
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto writeStatus = tester.WriteAttribute(CPAttributes::ScanMaxTime::Id, static_cast<uint8_t>(45));
    EXPECT_TRUE(writeStatus.IsSuccess());

    uint8_t scanMaxTime = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::ScanMaxTime::Id, scanMaxTime), CHIP_NO_ERROR);
    EXPECT_EQ(scanMaxTime, 45u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: ScanMaxTime constraint = "min 1".  Writing 0 SHALL be
// rejected with ConstraintError.
TEST_F(TestCommissioningProxyCluster, TestScanMaxTimeAttribute_WriteZeroConstraintError)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto writeStatus = tester.WriteAttribute(CPAttributes::ScanMaxTime::Id, static_cast<uint8_t>(0));
    EXPECT_EQ(writeStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ScanMaxTime storage/reporting is owned by the cluster: a write that changes the
// value SHALL emit a change report, and a write of the unchanged value SHALL NOT.
TEST_F(TestCommissioningProxyCluster, TestScanMaxTimeAttribute_ChangeReporting)
{
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);

    // Start with the tester's context so change notifications land in the dirty
    // list the tester observes.
    ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Writing the current value (default 10) is a no-op: success, no change report.
    EXPECT_TRUE(tester.WriteAttribute(CPAttributes::ScanMaxTime::Id, static_cast<uint8_t>(10)).IsSuccess());
    EXPECT_FALSE(tester.IsAttributeDirty(CPAttributes::ScanMaxTime::Id));

    // Writing a new value reports the change.
    EXPECT_TRUE(tester.WriteAttribute(CPAttributes::ScanMaxTime::Id, static_cast<uint8_t>(45)).IsSuccess());
    EXPECT_TRUE(tester.IsAttributeDirty(CPAttributes::ScanMaxTime::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: CacheTimeout (BGS-only) fallback = 120, access = "RW VO".
TEST_F(TestCommissioningProxyCluster, TestCacheTimeoutAttribute_DefaultAndWritable)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
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

// Spec § Attributes: CacheTimeout constraint = "min 1".  Writing 0 SHALL be
// rejected with ConstraintError.
TEST_F(TestCommissioningProxyCluster, TestCacheTimeoutAttribute_WriteZeroConstraintError)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto writeStatus = tester.WriteAttribute(CPAttributes::CacheTimeout::Id, static_cast<uint16_t>(0));
    EXPECT_EQ(writeStatus.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// CacheTimeout storage/reporting is owned by the cluster: a write that changes the
// value SHALL emit a change report, and a write of the unchanged value SHALL NOT.
TEST_F(TestCommissioningProxyCluster, TestCacheTimeoutAttribute_ChangeReporting)
{
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);

    // Start with the tester's context so change notifications land in the dirty
    // list the tester observes.
    ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Writing the current value (default 120) is a no-op: success, no change report.
    EXPECT_TRUE(tester.WriteAttribute(CPAttributes::CacheTimeout::Id, static_cast<uint16_t>(120)).IsSuccess());
    EXPECT_FALSE(tester.IsAttributeDirty(CPAttributes::CacheTimeout::Id));

    // Writing a new value reports the change.
    EXPECT_TRUE(tester.WriteAttribute(CPAttributes::CacheTimeout::Id, static_cast<uint16_t>(60)).IsSuccess());
    EXPECT_TRUE(tester.IsAttributeDirty(CPAttributes::CacheTimeout::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: NumCachedResults (BGS-only) fallback = 0.
TEST_F(TestCommissioningProxyCluster, TestNumCachedResultsAttribute_DefaultZero)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint8_t numCachedResults = 99; // pre-set to non-zero so we know read overwrites
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::NumCachedResults::Id, numCachedResults), CHIP_NO_ERROR);
    EXPECT_EQ(numCachedResults, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: MaxCachedResults (BGS-only) min 1.  Reads from config.
TEST_F(TestCommissioningProxyCluster, TestMaxCachedResultsAttribute_ReadsFromConfig)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint8_t maxCachedResults = 0;
    ASSERT_EQ(tester.ReadAttribute(CPAttributes::MaxCachedResults::Id, maxCachedResults), CHIP_NO_ERROR);
    EXPECT_EQ(maxCachedResults, cluster.GetMaxCachedResults());
    EXPECT_GE(maxCachedResults, 1u); // spec: min 1

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: WiFiBand (WI-only) reflects the configured supported bands.
TEST_F(TestCommissioningProxyCluster, TestWiFiBandAttribute_ReadsFromConfig)
{
    TestServerClusterContext context;
    chip::BitMask<WiFiBandBitmap> bands;
    bands.Set(WiFiBandBitmap::k2g4);
    bands.Set(WiFiBandBitmap::k5g);

    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(wi));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(bands);
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

// Zero transport bits SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ZeroTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(static_cast<CapabilitiesBitmap>(0));

    // Spec: "Exactly one transport ... SHALL be selected"; zero bits is an invalid
    // field, and the spec mandates INVALID_COMMAND for an invalid field.
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Multiple transport bits set SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_MultipleTransportBits)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.transport.Set(CapabilitiesBitmap::kWiFiPAF); // now two bits set

    // Spec: "Exactly one transport ... SHALL be selected"; >1 bit is an invalid
    // field → INVALID_COMMAND (test plan TC-2.6).
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ProxyConnectRequest: Discriminator constraint is "0 to 4095".  A value
// above 4095 is an invalid field and SHALL return InvalidCommand.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_DiscriminatorOutOfRange)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Exactly one transport bit set so the single-transport check passes; the
    // discriminator check runs before the transport-supported check, so this is
    // independent of which transports are compiled in.
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    cmd.discriminator                       = chip::kMaxDiscriminatorValue + 1; // one past the spec max

    auto result = tester.Invoke(cmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A single reserved bit in transport (a bit outside the spec-defined kBle /
// kWiFiPAF set) SHALL be rejected.  The cluster currently rejects this via
// HasExactlyOneBitSet → kValidTransportBits check (returns InvalidCommand or
// InvalidTransportType; either is spec-conforming for a malformed request).
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ReservedTransportBitOnly)
{
    TestServerClusterContext context;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(wi));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // 0x01 is a reserved bit (kBle=0x02, kWiFiPAF=0x08).
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(static_cast<CapabilitiesBitmap>(0x01));
    auto result                             = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    // Per the ProxyConnectRequest Effect on Receipt: an invalid/unsupported Transport → InvalidTransportType
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport without the WI feature SHALL be accepted: the WiFiPAF
// CapabilitiesBitmap conformance is O.a+, independent of the WI feature. WI
// only gates the WiFiBand field (absent here).
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);

    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Whenever the proxy advertises kBle in the Transport attribute (i.e. BLE was
// compiled into this build of the cluster) ProxyConnectRequest with kBle
// SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_BleCapability)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response->sessionID, 1u);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBand present with a non-WiFiPAF transport SHALL return InvalidCommand
// (the WiFiBand field is only meaningful for the WiFiPAF transport per spec).
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithBleTransport)
{
    TestServerClusterContext context;
    // Enable WI so the cluster has wiFiBand plumbing wired; the rejection here
    // is driven by the transport != kWiFiPAF check, not the WI feature bit.
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kBle);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiPAF transport with WI feature enabled — SHALL succeed and return a sessionId.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiPAFWithWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response->sessionID, 1u);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBand field present with WI feature enabled and band in supported set — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandWithWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in wiFiBand SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    // bit 1 (0x02) is reserved.
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02)));

    // Impl-defined: the spec mandates INVALID_TRANSPORT_TYPE only for an *unsupported*
    // WiFiBand; it does not mandate a specific status for reserved WiFiBand bits. This
    // asserts current behavior (INVALID_COMMAND for a malformed field).
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBand not in the proxy's supported bands SHALL return InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_WiFiBandNotInSupportedBands)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy only supports 2.4 GHz.

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyConnectRequest::Type cmd = MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF);
    // Request 5 GHz — not in supported bands.
    cmd.wiFiBand.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful ProxyConnectRequest the cluster state SHALL be kState_CPConnected.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_StateTransitionOnSuccess)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);
    EXPECT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A failed ProxyConnectRequest SHALL NOT change the cluster state.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_StateUnchangedOnFailure)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // A reserved transport bit (0x01) is never supported → InvalidTransportType,
    // a build-independent connect failure that must not transition state.
    EXPECT_FALSE(tester.Invoke(MakeConnectRequest(static_cast<CapabilitiesBitmap>(0x01))).IsSuccess());

    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyConnectRequest Effect on Receipt: if the number of active sessions
// has reached the value of the MaxSessions attribute, a RESOURCE_EXHAUSTED status
// SHALL be returned.  Enforced generically by the cluster from
// GetActiveSessionCount() vs the configured MaxSessions, so every transport inherits the
// behaviour without having to remember the check itself.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_ResourceExhaustedAtMaxSessions)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // MaxSessions defaults to 1. A pending connect counts toward the active-session
    // total, so saturating it makes the next connect hit the MaxSessions gate.
    mockBle.SetConnectPending(true);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::ResourceExhausted));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Below MaxSessions: the cluster-level pre-check does not reject; the
// request is forwarded to the transport driver and (for the mock) succeeds.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_BelowMaxSessionsSucceeds)
{
    TestServerClusterContext context;
    // MaxSessions == 2: the second concurrent connect is still below the gate.
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}, /*maxSessions=*/2));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    // First connect establishes one session (1 of 2 in use).
    EXPECT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    // Second connect is still below MaxSessions, so the gate passes and it succeeds.
    // (The MaxSessions == 1 exhaustion case is covered by _ResourceExhaustedAtMaxSessions.)
    EXPECT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyConnectRequest Effect on Receipt: if Timeout expires, the connection
// attempt is terminated and a TIMEOUT status SHALL be returned.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_TransportTimeout_Propagated)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // The transport's connect times out; the cluster SHALL surface TIMEOUT.
    mockBle.SetConnectStatus(Protocols::InteractionModel::Status::Timeout);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::Timeout));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyDisconnectRequest Command Tests
// =============================================================================

// After a successful connect, disconnect with the returned sessionId SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_AfterConnect)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    // Establish a proxy session.
    auto connectResult = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(connectResult.IsSuccess());
    ASSERT_TRUE(connectResult.response.has_value());

    // Disconnect the session using the sessionId from the response.
    Commands::ProxyDisconnectRequest::Type cmd;
    if (connectResult.response.has_value())
    {
        cmd.sessionID.SetNonNull(connectResult.response->sessionID);
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// After a successful disconnect, the cluster state SHALL revert to kState_CPDisconnected.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_StateTransitionToDisconnected)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    ASSERT_TRUE(tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle)).IsSuccess());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNonNull(1);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    // State SHALL be Disconnected after a successful disconnect.
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// With MaxSessions > 1, disconnecting one of several active sessions SHALL NOT
// transition the cluster to disconnected; only the final disconnect (no sessions
// remaining) SHALL do so.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_MultiSessionStateTransition)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}, /*maxSessions=*/2));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Open two sessions and capture their (distinct) session IDs.
    auto r1 = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(r1.IsSuccess());
    ASSERT_TRUE(r1.response.has_value());
    auto r2 = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(r2.IsSuccess());
    ASSERT_TRUE(r2.response.has_value());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    Commands::ProxyDisconnectRequest::Type cmd;

    // First disconnect: one session remains, so state SHALL stay Connected.
    cmd.sessionID.SetNonNull(r1.response->sessionID);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    // Second disconnect: no sessions remain, so state SHALL revert to Disconnected.
    cmd.sessionID.SetNonNull(r2.response->sessionID);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPDisconnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// When the transport fails to disconnect, the command SHALL fail and the cluster
// state SHALL remain Connected (session not cleaned up).
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_TransportFailurePreservesState)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto connectResult = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(connectResult.IsSuccess());
    ASSERT_TRUE(connectResult.response.has_value());
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    // The transport rejects the disconnect.
    mockBle.SetDisconnectStatus(Protocols::InteractionModel::Status::Failure);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNonNull(connectResult.response->sessionID);
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    // State SHALL remain Connected since the transport rejected the disconnect.
    EXPECT_EQ(cluster.GetCPState(), CommissioningProxyCluster::kState_CPConnected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A null SessionID with a pending connect SHALL return Success and cancel the connect.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_Success)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // A connect is pending on the BLE transport, owned by fabric 2.
    mockBle.SetPendingConnectFabric(2);

    tester.SetFabricIndex(2);
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNull();
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_TRUE(mockBle.CancelCalled());
    EXPECT_EQ(mockBle.LastCancelFabric(), 2);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A null SessionID with no ProxyConnectRequest in flight (e.g. already
// connected, or never started) has no pending connect to cancel.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_AlreadyConnected)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // CancelPendingConnect inherits InvalidInState from the transport driver (no pending connect).
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNull();
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidInState));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A null SessionID from the wrong fabric SHALL return NotFound (fabric isolation).
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_CancelPending_WrongFabric)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // A connect is pending on the BLE transport, owned by fabric 1.
    mockBle.SetPendingConnectFabric(1);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNull();

    // Fabric 2 tries to cancel fabric 1's pending connect — SHALL be rejected.
    tester.SetFabricIndex(2);
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    // Fabric 1 (the owner) cancels its own pending connect — SHALL succeed.
    tester.SetFabricIndex(1);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyDisconnectRequest Effect on Receipt: if no transport connection
// with the specified SessionId exists (or the connection's fabric does not match
// the sending fabric), the command SHALL be rejected with NOT_FOUND.  The cluster
// returns NotFound; cluster must propagate.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_UnknownSession_NotFound)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNonNull(1234); // no such session registered

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ProxyScanRequest Command Tests
// =============================================================================

// Zero transport bits SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ZeroTransport)
{
    TestServerClusterContext context;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ProxyScanRequest::Type command;
    command.transport = static_cast<CapabilitiesBitmap>(0);

    // Impl-defined: the ProxyScanRequest spec defines only a BUSY status; it mandates
    // no specific status for a zero/malformed transport. This asserts current behavior.
    auto result = tester.Invoke(Commands::ProxyScanRequest::Id, command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ReservedTransportBits)
{
    TestServerClusterContext context;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // kWiFiPAF(0x08) | reserved(0x01) = 0x09 contains a reserved bit.
    Commands::ProxyScanRequest::Type command;
    command.transport = static_cast<CapabilitiesBitmap>(0x09);

    // Impl-defined: the ProxyScanRequest spec defines only a BUSY status; it mandates
    // no specific status for reserved transport bits. This asserts current behavior.
    auto result = tester.Invoke(Commands::ProxyScanRequest::Id, command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Whenever the proxy advertises kBle in the Transport attribute,
// ProxyScanRequest with kBle
// SHALL succeed regardless of which Feature bits the cluster was constructed
// with.  Skips when the build does not include BLE.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_BleNoFeaturesSucceeds)
{
    TestServerClusterContext context;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
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
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    Commands::ProxyScanRequest::Type command;
    command.transport.Set(CapabilitiesBitmap::kBle);
    command.transport.Set(CapabilitiesBitmap::kWiFiPAF);

    // Each mock transport contributes 2 results; the aggregator SHALL combine both
    // sub-scans into a single ProxyScanResponse and report the total count.
    auto result = tester.Invoke(command);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->numberOfResults, 4u);
    size_t listCount = 0;
    auto iter        = result.response->proxyScanResult.begin();
    while (iter.Next())
        ++listCount;
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(listCount, 4u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF transport without the WI feature SHALL be accepted: the WiFiPAF
// transport bit is O.a+, independent of WI. WI only gates the wiFiBands field
// (absent here).
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;

    EXPECT_TRUE(tester.Invoke(Commands::ProxyScanRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WiFiBands field present without the WI feature is rejected. The kWiFiPAF transport
// itself is supported without WI; the rejection is driven solely by the
// wiFiBands-requires-WI guard. Impl-defined status: the spec does not mandate a
// specific status for a WI-conformance field sent without WI; this asserts current
// behavior (INVALID_COMMAND).
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiBandWithoutWIFeature)
{
    TestServerClusterContext context;

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));

    auto result = tester.Invoke(Commands::ProxyScanRequest::Id, command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in wiFiBands SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // k2g4=0x01, k5g=0x04 are valid; bit 1 (0x02) is reserved.
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02)));

    // Impl-defined: the spec mandates INVALID_TRANSPORT_TYPE only for unsupported
    // bands, not for reserved WiFiBand bits. This asserts current behavior.
    auto result = tester.Invoke(Commands::ProxyScanRequest::Id, command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands containing bits not in the proxy's supported bands SHALL return
// InvalidTransportType.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiBandNotInSupportedBands)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy only supports 2.4 GHz.

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Request 5 GHz — not in supported bands.
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    command.wiFiBands.SetValue(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g));

    auto result = tester.Invoke(Commands::ProxyScanRequest::Id, command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with WI feature enabled and no wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_WiFiPAFWithWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
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
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    // Proxy supports both 2.4 GHz and 5 GHz.
    chip::BitMask<WiFiBandBitmap> bothBands;
    bothBands.Set(WiFiBandBitmap::k2g4);
    bothBands.Set(WiFiBandBitmap::k5g);

    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(bothBands);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
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
// Establish a proxy session and return its sessionId (via the mock BLE transport).
static uint16_t OpenSession(ClusterTester & tester)
{
    auto conn = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    EXPECT_TRUE(conn.IsSuccess());
    EXPECT_TRUE(conn.response.has_value());
    return conn.response.has_value() ? conn.response->sessionID : 0;
}

TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_WithMessage)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    static const uint8_t kMsg[] = { 0x01, 0x02, 0x03, 0x04 };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    // The mock transport delivers an immediate (null) commissionee reply.
    auto result = tester.Invoke(cmd);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response->sessionID, sid);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A null ProxyMessageRequest.message SHALL succeed and
// return a ProxyMessageResponse with a null message (no data from commissionee).
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_NullMessage_Poll)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNull();

    auto result = tester.Invoke(cmd);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response->sessionID, sid);
        // Null response message signals no pending data from commissionee.
        EXPECT_TRUE(result.response->message.IsNull());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}


// Per the ProxyMessageRequest Effect on Receipt: if no transport connection with
// the specified SessionID exists (or the fabric does not match), the command SHALL
// be rejected with NOT_FOUND. The cluster's session table enforces this.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_UnknownSession_NotFound)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    static const uint8_t kMsg[] = { 0xFF };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = 9999; // no such session
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyMessageRequest Effect on Receipt: if another ProxyMessageRequest
// referencing the same SessionId is still outstanding, the command SHALL be
// rejected with BUSY. The cluster's session manager enforces this.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_DuplicateRequest_Busy)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    // Leave the first request pending (no commissionee reply), so the second one
    // for the same session hits the BUSY path.
    mockBle.SetAutoRespond(false);

    static const uint8_t kMsg[] = { 0xFF };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    [[maybe_unused]] auto pending = tester.Invoke(cmd); // first request: stays pending
    auto result                   = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::Busy));

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

// transport=0 SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ZeroTransport)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Impl-defined: the spec mandates INVALID_TRANSPORT_TYPE only for an unsupported
    // transport/band on BgScanStart; it does not mandate a status for a zero transport.
    // This asserts current behavior.
    auto result = tester.Invoke(MakeBgScanStartRequest(static_cast<CapabilitiesBitmap>(0)));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ReservedTransportBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // CapabilitiesBitmap: kBle=0x02, kWiFiPAF=0x08; all other bits are reserved.
    // 0x09 = kWiFiPAF(0x08) | reserved(0x01) → contains a reserved bit.
    // Impl-defined: the spec mandates no specific status for reserved transport bits
    // on BgScanStart. This asserts current behavior.
    auto result = tester.Invoke(MakeBgScanStartRequest(static_cast<CapabilitiesBitmap>(0x09)));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle is not gated by any Feature bit in the spec; whenever the proxy
// advertises kBle in the Transport attribute, ProxyBackGroundScanStartRequest
// with kBle SHALL succeed (only the BGS feature is required for the command
// to be accepted at all).
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_BleNoExtraFeaturesSucceeds)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF without WI feature SHALL be accepted: the WiFiPAF transport bit is
// O.a+, independent of WI. WI only gates the wiFiBands field (absent here).
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands with reserved bits SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // k2g4=0x01, k5g=0x04 are valid; bit 1 (0x02) is reserved.
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
                                      chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02))));
    // Impl-defined: the spec mandates INVALID_TRANSPORT_TYPE only for unsupported
    // bands, not reserved WiFiBand bits. This asserts current behavior.
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with a wiFiBands field but no WI feature is rejected. The kWiFiPAF
// transport itself is supported without WI; the rejection is driven solely by the
// wiFiBands-requires-WI guard. Impl-defined status: the spec does not mandate a
// specific status for a WI-conformance field sent without WI; this asserts current
// behavior (INVALID_COMMAND).
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_WiFiPAFAndBandWithoutWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    auto cmd    = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
                                         chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF with k2g4 wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ValidWiFiPAF_2g4Band)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
                                      chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF with k2g4|k5g wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_ValidWiFiPAF_BothBands)
{
    TestServerClusterContext context;
    chip::BitMask<WiFiBandBitmap> bothBands;
    bothBands.Set(WiFiBandBitmap::k2g4);
    bothBands.Set(WiFiBandBitmap::k5g);
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(bothBands);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
    auto cmd = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30, chip::MakeOptional(bothBands));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF with a WiFiBand bit not in GetSupportedWiFiBands() SHALL return INVALID_TRANSPORT_TYPE.
// Proxy only supports 2.4 GHz; requesting 5 GHz must be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_UnsupportedWiFiBand)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd    = MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF, 30,
                                         chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g)));
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

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

// transport=0 and no wiFiBands SHALL be rejected (nothing to stop).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ZeroTransportNoWiFiBands)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Impl-defined: the BgScanStop spec defines only NOT_FOUND and SUCCESS; it mandates
    // no specific status for an empty (transport=0, no bands) request. This asserts
    // current behavior.
    auto result = tester.Invoke(MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0)));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reserved bits in transport SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ReservedTransportBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // CapabilitiesBitmap: kBle=0x02, kWiFiPAF=0x08; all other bits are reserved.
    // 0x09 = kWiFiPAF(0x08) | reserved(0x01) → contains a reserved bit.
    // Impl-defined: the BgScanStop spec mandates no specific status for reserved
    // transport bits. This asserts current behavior.
    auto result = tester.Invoke(MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0x09)));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kBle is not gated by any Feature bit in the spec; whenever the proxy
// advertises kBle in the Transport attribute, ProxyBackGroundScanStopRequest
// with kBle passes the cluster-level transport validation and is forwarded to
// the transport driver.  The mock returns Success, mirroring the WiFiPAF
// positive case below.  (NotFound for an unrecognised fabric is a
// transport-level concern and is covered by the platform transport's own tests.)
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_BleValid)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// kWiFiPAF without WI feature SHALL be accepted: the WiFiPAF transport bit is
// O.a+, independent of WI. WI only gates the wiFiBands field (absent here).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_WiFiPAFWithoutWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    EXPECT_TRUE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands with reserved bits SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ReservedWiFiBandBits)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // Bit 1 (0x02) is reserved in WiFiBandBitmap.
    auto cmd = MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF,
                                     chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(static_cast<WiFiBandBitmap>(0x02))));
    // Impl-defined: the BgScanStop spec mandates no specific status for reserved
    // WiFiBand bits. This asserts current behavior.
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// wiFiBands without WI feature is rejected. Impl-defined status: the spec does not
// mandate a specific status for a WI-conformance field sent without WI; this asserts
// current behavior (INVALID_COMMAND).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_WiFiBandWithoutWIFeature)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan); // no kWiFiNetworkInterface
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    // A band-only stop (transport=0 + wiFiBands) without the WI feature SHALL be
    // rejected with InvalidCommand by the wiFiBands-requires-WI guard.
    auto cmd    = MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0),
                                        chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// transport=0 with valid wiFiBands and WI feature — SHALL succeed
// (a band-only stop is valid per the ProxyBackGroundScanStopRequest behaviour).
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_BandOnlyStop_Valid)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto cmd = MakeBgScanStopRequest(static_cast<CapabilitiesBitmap>(0),
                                     chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Valid kWiFiPAF stop with k5g wiFiBands — SHALL succeed.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_ValidWiFiPAF_5gBand)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);
    auto cmd =
        MakeBgScanStopRequest(CapabilitiesBitmap::kWiFiPAF, chip::MakeOptional(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g)));
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyBackGroundScanStopRequest Effect on Receipt: if the NodeId and
// FabricId of the client do not match those used in a previous
// ProxyBackGroundScanStartRequest, the proxy SHALL take no action and the command
// SHALL be rejected with NOT_FOUND.  This is transport-side state; the cluster
// must propagate the transport's NotFound status.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_TransportNotFound_Propagated)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // No transport has a matching per-fabric background-scan record. The cluster
    // fans the stop to every registered transport and reports NotFound only when
    // none matched, so both mocks must report NotFound.
    mockBle.SetBgScanStopStatus(Protocols::InteractionModel::Status::NotFound);
    mockPaf.SetBgScanStopStatus(Protocols::InteractionModel::Status::NotFound);

    ClusterTester tester(cluster);
    auto result = tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Per the ProxyBackGroundScanStopRequest Effect on Receipt: "If valid Transports and
// WiFiBands are received but were not originally requested, the command SHALL return a
// status of SUCCESS." The cluster fans the stop to every registered transport and
// reports SUCCESS when at least one matched, even if another reports NOT_FOUND.
TEST_F(TestCommissioningProxyCluster, TestBgScanStop_PartialMatch_Success)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // BLE has no matching per-fabric record (NOT_FOUND); PAF reports SUCCESS.
    mockBle.SetBgScanStopStatus(Protocols::InteractionModel::Status::NotFound);
    mockPaf.SetBgScanStopStatus(Protocols::InteractionModel::Status::Success);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);
    EXPECT_TRUE(tester.Invoke(MakeBgScanStopRequest(CapabilitiesBitmap::kBle)).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Additional coverage: read-only writes, unsupported transports, scan BUSY,
// ProxyMessage TIMEOUT, and CachedResults content.
// =============================================================================

// Build a minimal ScanResultStruct for injecting into the ScanCache.
static Structs::ScanResultStruct::Type MakeScanEntry(uint16_t discriminator, CapabilitiesBitmap transport)
{
    Structs::ScanResultStruct::Type e;
    e.transport     = chip::BitMask<CapabilitiesBitmap>(transport);
    e.discriminator = discriminator;
    e.vendorID      = static_cast<chip::VendorId>(0x1234);
    e.productID     = 0x0001;
    e.address.SetNull();
    e.extendedData.SetNull();
    return e;
}

// Spec § Attributes: MaxCachedResults is R V (read-only). A write SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestMaxCachedResultsAttribute_WriteRejected)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::MaxCachedResults::Id, static_cast<uint8_t>(5));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: NumCachedResults is R V (read-only). A write SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestNumCachedResultsAttribute_WriteRejected)
{
    TestServerClusterContext context;
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::NumCachedResults::Id, static_cast<uint8_t>(3));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § Attributes: WiFiBand is F R V (read-only, fixed). A write SHALL be rejected.
TEST_F(TestCommissioningProxyCluster, TestWiFiBandAttribute_WriteRejected)
{
    TestServerClusterContext context;
    BitMask<Feature> wi(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(wi));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto status = tester.WriteAttribute(CPAttributes::WiFiBand::Id, chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k5g));
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A valid transport bit with no registered driver SHALL be rejected with
// INVALID_TRANSPORT_TYPE (test plan TC-2.4 step 11).  Register only BLE, request PAF.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_UnsupportedTransport_InvalidTransportType)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    cluster.RegisterTransport(mockBle); // only BLE
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    auto result = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kWiFiPAF));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ProxyScanRequest for a valid-but-unregistered transport SHALL be rejected with
// INVALID_TRANSPORT_TYPE (test plan TC-2.2 step 9).
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_UnsupportedTransport_InvalidTransportType)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    cluster.RegisterTransport(mockBle); // only BLE
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kWiFiPAF;
    auto result       = tester.Invoke(command);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ProxyBackGroundScanStartRequest for a valid-but-unregistered transport SHALL be
// rejected with INVALID_TRANSPORT_TYPE (spec + test plan TC-2.3 step 19).
TEST_F(TestCommissioningProxyCluster, TestBgScanStart_UnsupportedTransport_InvalidTransportType)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    cluster.RegisterTransport(mockBle); // only BLE
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    auto result = tester.Invoke(MakeBgScanStartRequest(CapabilitiesBitmap::kWiFiPAF));
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::InvalidTransportType));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec: a ProxyScanRequest received while one is in progress MAY be answered with
// BUSY. This implementation chose the BUSY mechanism; verify it.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_ConcurrentBusy)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    // Leave the first scan in-flight (no synchronous contribution) so the aggregator
    // stays busy for the second request.
    mockBle.SetAutoContribute(false);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kBle;
    [[maybe_unused]] auto first = tester.Invoke(command); // stays pending

    auto second = tester.Invoke(command);
    EXPECT_FALSE(second.IsSuccess());
    EXPECT_EQ(second.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::Busy));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § ProxyMessageResponse: "If the ResponseTimeout from the ProxyMessageRequest
// expires the TIMEOUT status SHALL be returned."
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_ResponseTimeout)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    // The commissionee never replies; the response timeout fires and resolves the
    // pending request with TIMEOUT.
    mockBle.SetSendMessageTimeout(true);

    static const uint8_t kMsg[] = { 0xAB };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::Timeout));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// CachedResults / NumCachedResults reflect the ScanCache: null/0 when empty, unique
// per discriminator/VID/PID/transport (spec), and cleared by ClearTransport. Change
// reporting for both attributes is the cluster's responsibility.
TEST_F(TestCommissioningProxyCluster, TestCachedResults_ReportDedupAndClear)
{
    BitMask<Feature> bgs(Feature::kBackgroundScan);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(bgs));
    RegisterMocks(cluster);

    ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto readNum = [&]() {
        uint8_t n = 0xFF;
        EXPECT_EQ(tester.ReadAttribute(CPAttributes::NumCachedResults::Id, n), CHIP_NO_ERROR);
        return n;
    };

    // Empty cache: NumCachedResults 0, CachedResults null.
    EXPECT_EQ(readNum(), 0u);
    {
        Attributes::CachedResults::TypeInfo::DecodableType list;
        ASSERT_EQ(tester.ReadAttribute(CPAttributes::CachedResults::Id, list), CHIP_NO_ERROR);
        EXPECT_TRUE(list.IsNull());
    }

    // Report device A → count 1; CachedResults and NumCachedResults marked dirty.
    cluster.ScanCache().Report(MakeScanEntry(1000, CapabilitiesBitmap::kBle));
    EXPECT_EQ(readNum(), 1u);
    EXPECT_TRUE(tester.IsAttributeDirty(CPAttributes::CachedResults::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(CPAttributes::NumCachedResults::Id));

    // Report device B (different discriminator) → count 2.
    cluster.ScanCache().Report(MakeScanEntry(2000, CapabilitiesBitmap::kBle));
    EXPECT_EQ(readNum(), 2u);

    // Re-report device A (same discriminator/VID/PID/transport) → dedup, count unchanged.
    cluster.ScanCache().Report(MakeScanEntry(1000, CapabilitiesBitmap::kBle));
    EXPECT_EQ(readNum(), 2u);

    // CachedResults is now a non-null list with 2 entries.
    {
        Attributes::CachedResults::TypeInfo::DecodableType list;
        ASSERT_EQ(tester.ReadAttribute(CPAttributes::CachedResults::Id, list), CHIP_NO_ERROR);
        ASSERT_FALSE(list.IsNull());
        size_t count = 0;
        auto it      = list.Value().begin();
        while (it.Next())
            ++count;
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(count, 2u);
    }

    // Clearing the BLE transport removes all its entries → count 0.
    cluster.ScanCache().ClearTransport(chip::BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle));
    EXPECT_EQ(readNum(), 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Additional coverage: fabric isolation on established sessions, session
// lifecycle, ProxyMessage edge paths, scan sub-scan handling, GeneratedCommands.
// =============================================================================

// Spec: the proxy associates a session with the invoking fabric, and a disconnect
// whose fabric does not match SHALL be rejected with NOT_FOUND. Fabric 1 opens a
// session; fabric 2 cannot disconnect it, but fabric 1 can.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_WrongFabricEstablishedSession_NotFound)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    tester.SetFabricIndex(1);
    uint16_t sid = OpenSession(tester);

    Commands::ProxyDisconnectRequest::Type cmd;
    cmd.sessionID.SetNonNull(sid);

    // Fabric 2 attempts to disconnect fabric 1's session → NOT_FOUND.
    tester.SetFabricIndex(2);
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    // The owning fabric can still disconnect it (session survived the foreign attempt).
    tester.SetFabricIndex(1);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec: a ProxyMessageRequest whose fabric does not match the session's fabric SHALL
// be rejected with NOT_FOUND.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_WrongFabricEstablishedSession_NotFound)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    tester.SetFabricIndex(1);
    uint16_t sid = OpenSession(tester);

    tester.SetFabricIndex(2);
    static const uint8_t kMsg[] = { 0x01 };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec: "The SessionId allows multiple commissioning sessions to be run in parallel."
// With MaxSessions >= 2, two connects SHALL each get a distinct, non-zero SessionId.
TEST_F(TestCommissioningProxyCluster, TestProxyConnectRequest_MultipleSessionsHaveDistinctSessionIds)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}, /*maxSessions=*/2));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    auto first = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(first.IsSuccess());
    ASSERT_TRUE(first.response.has_value());
    auto second = tester.Invoke(MakeConnectRequest(CapabilitiesBitmap::kBle));
    ASSERT_TRUE(second.IsSuccess());
    ASSERT_TRUE(second.response.has_value());

    EXPECT_NE(first.response->sessionID, 0u);
    EXPECT_NE(second.response->sessionID, 0u);
    EXPECT_NE(first.response->sessionID, second.response->sessionID);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A ProxyDisconnectRequest SHALL remove the session: a subsequent ProxyMessageRequest
// referencing the same SessionId SHALL be rejected with NOT_FOUND.
TEST_F(TestCommissioningProxyCluster, TestProxyDisconnectRequest_RemovesSession)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    uint16_t sid = OpenSession(tester);

    Commands::ProxyDisconnectRequest::Type dc;
    dc.sessionID.SetNonNull(sid);
    EXPECT_TRUE(tester.Invoke(dc).IsSuccess());

    static const uint8_t kMsg[] = { 0x01 };
    Commands::ProxyMessageRequest::Type msg;
    msg.sessionID       = sid;
    msg.responseTimeout = 5;
    msg.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));
    auto result = tester.Invoke(msg);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Spec § ProxyMessageRequest ResponseTimeout: "A value of zero indicates no response
// is expected and the proxy should send ProxyMessageResponse immediately indicating
// success." The response message SHALL be null.
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_ResponseTimeoutZero_ImmediateSuccess)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    static const uint8_t kMsg[] = { 0x01, 0x02 };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 0;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    auto result = tester.Invoke(cmd);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->sessionID, sid);
    EXPECT_TRUE(result.response->message.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// If the transport SendMessage fails, the command SHALL fail and the pending state
// SHALL be rolled back (the session is not left BUSY for the next request).
TEST_F(TestCommissioningProxyCluster, TestProxyMessageRequest_SendMessageFailure_RollsBackPending)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t sid = OpenSession(tester);

    static const uint8_t kMsg[] = { 0xAA };
    Commands::ProxyMessageRequest::Type cmd;
    cmd.sessionID       = sid;
    cmd.responseTimeout = 5;
    cmd.message.SetNonNull(chip::ByteSpan(kMsg, sizeof(kMsg)));

    // First request: the transport send fails → FAILURE.
    mockBle.SetSendMessageError(CHIP_ERROR_INTERNAL);
    auto first = tester.Invoke(cmd);
    EXPECT_FALSE(first.IsSuccess());
    EXPECT_EQ(first.GetStatusCode(), ClusterStatusCode(Protocols::InteractionModel::Status::Failure));

    // The pending state was rolled back: a follow-up request is not rejected as BUSY.
    mockBle.SetSendMessageError(CHIP_NO_ERROR);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// If no requested sub-scan can be started, the ProxyScanRequest SHALL fail rather
// than hang waiting for a contribution that never comes.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_AllSubScansFailToStart_Error)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);

    mockBle.SetScanStatus(Protocols::InteractionModel::Status::Failure);
    Commands::ProxyScanRequest::Type command;
    command.transport = CapabilitiesBitmap::kBle;
    EXPECT_FALSE(tester.Invoke(command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// If some (but not all) requested sub-scans start, the combined ProxyScanResponse
// SHALL carry the results of the transports that did start.
TEST_F(TestCommissioningProxyCluster, TestProxyScanRequest_PartialStart_ReturnsStartedResults)
{
    TestServerClusterContext context;
    BitMask<Feature> features(Feature::kWiFiNetworkInterface);
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(features));
    RegisterMocks(cluster);
    cluster.SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap>(WiFiBandBitmap::k2g4));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kBle);
    SKIP_IF_TRANSPORT_UNSUPPORTED(tester, CapabilitiesBitmap::kWiFiPAF);

    // PAF fails to start; BLE succeeds and contributes 2 results.
    mockPaf.SetScanStatus(Protocols::InteractionModel::Status::Failure);
    Commands::ProxyScanRequest::Type command;
    command.transport.Set(CapabilitiesBitmap::kBle);
    command.transport.Set(CapabilitiesBitmap::kWiFiPAF);

    auto result = tester.Invoke(command);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->numberOfResults, 2u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// GeneratedCommandList SHALL advertise the three response commands the server
// generates (regression guard for the GeneratedCommands override).
TEST_F(TestCommissioningProxyCluster, TestGeneratedCommandsAdvertisesResponses)
{
    TestServerClusterContext context;
    CommissioningProxyCluster cluster(kTestEndpointId, CommissioningProxyCluster::Config(BitMask<Feature>{}));
    RegisterMocks(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<CommandId> generated;
    EXPECT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id), generated), CHIP_NO_ERROR);
    auto buffer = generated.TakeBuffer();

    bool hasConnect = false, hasScan = false, hasMessage = false;
    for (const CommandId id : buffer)
    {
        hasConnect |= (id == Commands::ProxyConnectResponse::Id);
        hasScan |= (id == Commands::ProxyScanResponse::Id);
        hasMessage |= (id == Commands::ProxyMessageResponse::Id);
    }
    EXPECT_TRUE(hasConnect);
    EXPECT_TRUE(hasScan);
    EXPECT_TRUE(hasMessage);
    EXPECT_EQ(buffer.size(), 3u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
