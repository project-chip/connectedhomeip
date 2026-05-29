/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>
#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ProximityRanging/Attributes.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Events.h>
#include <clusters/ProximityRanging/Metadata.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ProximityRanging;
using chip::Testing::ClusterTester;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;
using chip::Testing::TestServerClusterContext;

constexpr EndpointId kTestEndpointId = 1;

class MockRangingAdapter : public RangingAdapter
{
public:
    explicit MockRangingAdapter(RangingTechEnum technology) : mTechnology(technology) {}

    RangingTechEnum GetTechnology() const override { return mTechnology; }

    Structs::RangingCapabilitiesStruct::Type GetCapabilities() const override
    {
        Structs::RangingCapabilitiesStruct::Type cap;
        cap.technology = mTechnology;
        return cap;
    }

    ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) override
    {
        mLastStartSessionId = sessionId;
        mStartCalls++;
        if (mStartResult == ResultCodeEnum::kAccepted)
        {
            mActiveIds.push_back(sessionId);
        }
        return mStartResult;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        mLastStopSessionId = sessionId;
        VerifyOrReturnError(mStopError == CHIP_NO_ERROR, mStopError);
        for (auto it = mActiveIds.begin(); it != mActiveIds.end(); ++it)
        {
            if (*it == sessionId)
            {
                mActiveIds.erase(it);
                if (mCallback != nullptr)
                {
                    mCallback->OnRangingSessionStopped(sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
                }
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    void StopAllSessions() override
    {
        while (!mActiveIds.empty())
        {
            uint8_t id = mActiveIds.back();
            mActiveIds.pop_back();
            if (mCallback != nullptr)
            {
                mCallback->OnRangingSessionStopped(id, RangingSessionStatusEnum::kSessionEndTimeReached);
            }
        }
    }

    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & out) override
    {
        VerifyOrReturnError(out.size() >= mActiveIds.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        for (size_t i = 0; i < mActiveIds.size(); i++)
        {
            out[i] = mActiveIds[i];
        }
        out.reduce_size(mActiveIds.size());
        return CHIP_NO_ERROR;
    }

    std::optional<uint64_t> GetDeviceId() override { return mDeviceId; }
    std::optional<WiFiUsdConfig> GetWiFiUsdConfig() override { return mWiFiUsdConfig; }
    std::optional<BltcsConfig> GetBltcsConfig() override { return mBltcsConfig; }

    Callback * GetCallback() const { return mCallback; }

    // Test control
    ResultCodeEnum mStartResult = ResultCodeEnum::kAccepted;
    CHIP_ERROR mStopError       = CHIP_NO_ERROR;
    uint8_t mLastStartSessionId = 0;
    uint8_t mLastStopSessionId  = 0;
    int mStartCalls             = 0;
    std::vector<uint8_t> mActiveIds;
    std::optional<uint64_t> mDeviceId;
    std::optional<WiFiUsdConfig> mWiFiUsdConfig;
    std::optional<BltcsConfig> mBltcsConfig;

private:
    RangingTechEnum mTechnology;
};

struct TestProximityRangingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestProximityRangingCluster, TestAttributeListMandatoryOnly)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithAllFeatures)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config(driver).WithFeatures(
            BitMask<Feature>{ Feature::kWiFiUsdProximityDetection, Feature::kBleBeaconRssi, Feature::kBluetoothChannelSounding }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::WiFiDevIK::kMetadataEntry,
                                            Attributes::BLEDeviceID::kMetadataEntry,
                                            Attributes::BLTDevIK::kMetadataEntry,
                                            Attributes::BLTCSSecurityLevel::kMetadataEntry,
                                            Attributes::BLTCSModeCapability::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithBleBeaconRssi)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::BLEDeviceID::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithBluetoothChannelSounding)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBluetoothChannelSounding }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::BLTDevIK::kMetadataEntry,
                                            Attributes::BLTCSSecurityLevel::kMetadataEntry,
                                            Attributes::BLTCSModeCapability::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithWiFiUSDProximityDetection)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::WiFiDevIK::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithUWBRanging)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kUwbRanging }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestFeatureMapMultipleFeatures)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config(driver).WithFeatures(
                                        BitMask<Feature>{ Feature::kBleBeaconRssi, Feature::kBluetoothChannelSounding,
                                                          Feature::kWiFiUsdProximityDetection, Feature::kUwbRanging }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint32_t featureMap = 0;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap,
              static_cast<uint32_t>(Feature::kBleBeaconRssi) | static_cast<uint32_t>(Feature::kBluetoothChannelSounding) |
                  static_cast<uint32_t>(Feature::kWiFiUsdProximityDetection) | static_cast<uint32_t>(Feature::kUwbRanging));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadAttributeNotInFeatureMap)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    // Only WiFi feature enabled - BLEDeviceID should not be readable
    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestAcceptedCommands)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::StartRangingRequest::kMetadataEntry,
                                                  Commands::StopRangingRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestProximityRangingCluster, TestGeneratedCommands)
{
    ProximityRangingDriver driver{ {} };
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };

    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::StartRangingResponse::Id }));
}

TEST_F(TestProximityRangingCluster, TestStartupShutdownLifecycle)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    // Re-Startup must succeed after Shutdown; the driver should accept a new callback.
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadFeatureMap)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint32_t featureMap = 0;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kBleBeaconRssi));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadClusterRevision)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t revision = 0;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, ProximityRanging::kRevision);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadRangingCapabilities)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Attributes::RangingCapabilities::TypeInfo::DecodableType capList;
    EXPECT_EQ(tester.ReadAttribute(Attributes::RangingCapabilities::Id, capList), CHIP_NO_ERROR);

    auto iter = capList.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().technology, RangingTechEnum::kBLEBeaconRSSIRanging);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadSessionIdListEmpty)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Attributes::SessionIDList::TypeInfo::DecodableType sessionList;
    EXPECT_EQ(tester.ReadAttribute(Attributes::SessionIDList::Id, sessionList), CHIP_NO_ERROR);
    EXPECT_FALSE(sessionList.IsNull());

    size_t count = 0;
    EXPECT_EQ(sessionList.Value().ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadBleDeviceIdSupported)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    bleAdapter.mDeviceId        = 0x1234;
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_NO_ERROR);
    EXPECT_EQ(bleId, static_cast<uint64_t>(0x1234));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadBleDeviceIdNoAdapterRegistered)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    // Feature claimed but no BLE adapter registered → driver returns nullopt → cluster surfaces UnsupportedAttribute.
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadWiFiDevIKSupported)
{
    TestServerClusterContext context;
    MockRangingAdapter wifiAdapter(RangingTechEnum::kWiFiRoundTripTimeRanging);
    WiFiUsdConfig wifiConfig{};
    memset(wifiConfig.deviceIdentityKey, 0xAB, sizeof(wifiConfig.deviceIdentityKey));
    wifiAdapter.mWiFiUsdConfig  = wifiConfig;
    RangingAdapter * adapters[] = { &wifiAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    ByteSpan wifiKey;
    EXPECT_EQ(tester.ReadAttribute(Attributes::WiFiDevIK::Id, wifiKey), CHIP_NO_ERROR);
    EXPECT_EQ(wifiKey.size(), 16u);
    EXPECT_EQ(wifiKey.data()[0], 0xAB);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadOptionalAttributeNoAdapter)
{
    TestServerClusterContext context;
    ProximityRangingDriver driver{ {} };

    // Features claimed but no adapter registered → all driver-backed reads return UnsupportedAttribute.
    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{
                                        Feature::kWiFiUsdProximityDetection, Feature::kBluetoothChannelSounding }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    ByteSpan span;
    EXPECT_EQ(tester.ReadAttribute(Attributes::WiFiDevIK::Id, span), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLTDevIK::Id, span), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    uint8_t enumVal = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLTCSSecurityLevel::Id, enumVal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLTCSModeCapability::Id, enumVal), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Build a minimal valid StartRangingRequest for BLE Beacon RSSI ranging, used by tests
// that exercise paths after preflight validation succeeds.
static Commands::StartRangingRequest::Type MakeValidBleBeaconRequest()
{
    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;
    Structs::BLERangingDeviceRoleConfigStruct::Type role;
    role.role            = RangingRoleEnum::kBLEScanningRole;
    role.peerBLEDeviceID = 0xABCD'1234'5678'9ABC;
    request.BLERangingDeviceRoleConfig.SetValue(role);
    request.trigger.startTime = 0;
    request.trigger.endTime   = 60;
    return request;
}

TEST_F(TestProximityRangingCluster, TestStartRangingAccepted)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto request = MakeValidBleBeaconRequest();

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    if (result.response.has_value())
    {
        auto & response = result.response.value();
        EXPECT_EQ(response.resultCode, ResultCodeEnum::kAccepted);
        EXPECT_FALSE(response.sessionID.IsNull());
        EXPECT_NE(response.sessionID.Value(), 0);
        EXPECT_EQ(bleAdapter.mStartCalls, 1);
    }
    else
    {
        FAIL();
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingAdapterRejects)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    bleAdapter.mStartResult     = ResultCodeEnum::kRejectedInfeasibleRanging;
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto request = MakeValidBleBeaconRequest();

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRanging);
    }
    else
    {
        FAIL();
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingTechnologyNotInFeatureMap)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    // Only BLE Beacon RSSI is enabled - request BLT Channel Sounding to trigger preflight.
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBluetoothChannelSounding;
    Structs::BLTChannelSoundingDeviceRoleConfigStruct::Type role;
    role.role = RangingRoleEnum::kBLTInitiatorRole;
    request.BLTChannelSoundingDeviceRoleConfig.SetValue(role);

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRanging);
    EXPECT_TRUE(result.response.value().sessionID.IsNull());
    EXPECT_EQ(bleAdapter.mStartCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingMissingMatchingRoleConfig)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Technology is supported, but BLERangingDeviceRoleConfig is missing.
    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRanging);
    EXPECT_TRUE(result.response.value().sessionID.IsNull());
    EXPECT_EQ(bleAdapter.mStartCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingMismatchedRoleConfig)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    MockRangingAdapter bltAdapter(RangingTechEnum::kBluetoothChannelSounding);
    RangingAdapter * adapters[] = { &bleAdapter, &bltAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config(driver).WithFeatures(
                                        BitMask<Feature>{ Feature::kBleBeaconRssi, Feature::kBluetoothChannelSounding }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Technology=BLE Beacon RSSI but only the BLT config is set - inconsistent.
    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;
    Structs::BLTChannelSoundingDeviceRoleConfigStruct::Type role;
    role.role = RangingRoleEnum::kBLTInitiatorRole;
    request.BLTChannelSoundingDeviceRoleConfig.SetValue(role);

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRanging);
    EXPECT_TRUE(result.response.value().sessionID.IsNull());
    EXPECT_EQ(bleAdapter.mStartCalls, 0);
    EXPECT_EQ(bltAdapter.mStartCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingTriggerEndTimeNotAfterStart)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto request              = MakeValidBleBeaconRequest();
    request.trigger.startTime = 10;
    request.trigger.endTime   = 10;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRangingTriggers);
    EXPECT_EQ(bleAdapter.mStartCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingTriggerIntervalZero)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto request = MakeValidBleBeaconRequest();
    request.trigger.rangingInstanceInterval.SetValue(0);

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kRejectedInfeasibleRangingTriggers);
    EXPECT_EQ(bleAdapter.mStartCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStopRangingSuccess)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config(driver).WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // First start a session so there's something to stop.
    {
        auto startResult = tester.Invoke(MakeValidBleBeaconRequest());
        ASSERT_TRUE(startResult.IsSuccess());
        ASSERT_TRUE(startResult.response.has_value());
        EXPECT_EQ(startResult.response.value().resultCode, ResultCodeEnum::kAccepted);
    }

    Commands::StopRangingRequest::Type stopReq;
    stopReq.sessionID = bleAdapter.mLastStartSessionId;

    auto result = tester.Invoke(stopReq);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(bleAdapter.mLastStopSessionId, stopReq.sessionID);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStopRangingNotFound)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StopRangingRequest::Type stopReq;
    stopReq.sessionID = 99;

    auto result = tester.Invoke(stopReq);
    EXPECT_FALSE(result.IsSuccess());
    if (result.GetStatusCode().has_value())
    {
        EXPECT_EQ(result.GetStatusCode().value().GetStatus(), Protocols::InteractionModel::Status::InvalidInState);
    }
    else
    {
        FAIL();
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestOnMeasurementDataEvent)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Register session 42 with the driver so the OnMeasurementData callback is
    // not filtered as a stale notification.
    Commands::StartRangingRequest::DecodableType startRequest{};
    startRequest.technology = RangingTechEnum::kBLEBeaconRSSIRanging;
    ASSERT_EQ(driver.HandleStartRanging(42, startRequest), ResultCodeEnum::kAccepted);

    Structs::RangingMeasurementDataStruct::Type measurement;
    measurement.distance.SetNonNull(static_cast<uint16_t>(500));

    // Adapter fires measurement → driver routes to cluster → cluster generates event.
    ASSERT_NE(bleAdapter.GetCallback(), nullptr);
    bleAdapter.GetCallback()->OnMeasurementData(42, measurement);

    auto eventInfo = context.EventsGenerator().GetNextEvent();

    Events::RangingResult::DecodableType decodedEvent;
    if (eventInfo.has_value())
    {
        EXPECT_EQ(eventInfo.value().GetEventData(decodedEvent), CHIP_NO_ERROR);
    }
    else
    {
        FAIL();
    }
    EXPECT_EQ(decodedEvent.sessionID, 42);
    EXPECT_FALSE(decodedEvent.rangingResultData.distance.IsNull());
    EXPECT_EQ(decodedEvent.rangingResultData.distance.Value(), 500);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestOnSessionStoppedEvent)
{
    TestServerClusterContext context;
    MockRangingAdapter bleAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    RangingAdapter * adapters[] = { &bleAdapter };
    ProximityRangingDriver driver{ Span<RangingAdapter * const>(adapters) };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config(driver) };
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Register session 7 with the driver so the OnRangingSessionStopped
    // callback is not filtered as a stale notification.
    Commands::StartRangingRequest::DecodableType startRequest{};
    startRequest.technology = RangingTechEnum::kBLEBeaconRSSIRanging;
    ASSERT_EQ(driver.HandleStartRanging(7, startRequest), ResultCodeEnum::kAccepted);

    ASSERT_NE(bleAdapter.GetCallback(), nullptr);
    bleAdapter.GetCallback()->OnRangingSessionStopped(7, RangingSessionStatusEnum::kSessionEndTimeReached);

    auto eventInfo = context.EventsGenerator().GetNextEvent();

    Events::RangingSessionStatus::DecodableType decodedEvent;
    if (eventInfo.has_value())
    {
        EXPECT_EQ(eventInfo.value().GetEventData(decodedEvent), CHIP_NO_ERROR);
    }
    else
    {
        FAIL();
    }
    EXPECT_EQ(decodedEvent.sessionID, 7);
    EXPECT_EQ(decodedEvent.status, RangingSessionStatusEnum::kSessionEndTimeReached);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
