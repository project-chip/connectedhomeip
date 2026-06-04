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

class MockProximityRangingDriver : public ProximityRangingDriver
{
public:
    CHIP_ERROR Init(Callback & callback) override
    {
        mCallback = &callback;
        return mInitError;
    }
    void Shutdown() override { mShutdownCalled = true; }
    ResultCodeEnum HandleStartRanging(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) override
    {
        mLastStartSessionId = sessionId;
        return mStartRangingResult;
    }
    CHIP_ERROR HandleStopRanging(uint8_t sessionId) override
    {
        mLastStopSessionId = sessionId;
        return mStopRangingError;
    }
    CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder) override
    {
        return encoder.EncodeList([](const auto & listEncoder) -> CHIP_ERROR {
            Structs::RangingCapabilitiesStruct::Type cap;
            cap.technology = RangingTechEnum::kBLEBeaconRSSIRanging;
            return listEncoder.Encode(cap);
        });
    }

    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) override
    {
        VerifyOrReturnError(mGetActiveSessionIdsError == CHIP_NO_ERROR, mGetActiveSessionIdsError);
        size_t count = std::min(mActiveSessionIds.size(), sessionIds.size());
        if (count == 0)
        {
            sessionIds = Span<uint8_t>();
            return CHIP_NO_ERROR;
        }
        memcpy(sessionIds.data(), mActiveSessionIds.data(), count);
        sessionIds.reduce_size(count);
        return CHIP_NO_ERROR;
    }
    size_t GetNumActiveSessionIds() override { return mActiveSessionIds.size(); }
    std::optional<BleRbcConfig> GetBleRbcConfig() override { return mBleRbcConfig; }
    std::optional<WiFiUsdConfig> GetWiFiUsdConfig() override { return mWiFiUsdConfig; }
    std::optional<BltcsConfig> GetBltcsConfig() override { return mBltcsConfig; }

    // Test control
    CHIP_ERROR mInitError                = CHIP_NO_ERROR;
    bool mShutdownCalled                 = false;
    ResultCodeEnum mStartRangingResult   = ResultCodeEnum::kAccepted;
    CHIP_ERROR mStopRangingError         = CHIP_NO_ERROR;
    CHIP_ERROR mGetActiveSessionIdsError = CHIP_NO_ERROR;
    std::vector<uint8_t> mActiveSessionIds;
    uint8_t mLastStartSessionId = 0;
    uint8_t mLastStopSessionId  = 0;
    std::optional<BleRbcConfig> mBleRbcConfig;
    std::optional<WiFiUsdConfig> mWiFiUsdConfig;
    std::optional<BltcsConfig> mBltcsConfig;
    Callback * mCallback = nullptr;
};

struct TestProximityRangingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestProximityRangingCluster, TestAttributeListMandatoryOnly)
{
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithAllFeatures)
{
    ProximityRangingCluster cluster(
        kTestEndpointId,
        ProximityRangingCluster::Config().WithFeatures(
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
    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::BLEDeviceID::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithBluetoothChannelSounding)
{
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kBluetoothChannelSounding }));

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
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::WiFiDevIK::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestAttributeListWithUWBRanging)
{
    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kUwbRanging }));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::RangingCapabilities::kMetadataEntry,
                                            Attributes::SessionIDList::kMetadataEntry,
                                        }));
}

TEST_F(TestProximityRangingCluster, TestFeatureMapMultipleFeatures)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(
                                        BitMask<Feature>{ Feature::kBleBeaconRssi, Feature::kBluetoothChannelSounding,
                                                          Feature::kWiFiUsdProximityDetection, Feature::kUwbRanging }));
    cluster.SetDriver(&driver);
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
    MockProximityRangingDriver driver;

    // Only WiFi feature enabled — BLEDeviceID should not be readable
    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestAcceptedCommands)
{
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::StartRangingRequest::kMetadataEntry,
                                                  Commands::StopRangingRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestProximityRangingCluster, TestGeneratedCommands)
{
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };

    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::StartRangingResponse::Id }));
}

TEST_F(TestProximityRangingCluster, TestStartupWithoutDriver)
{
    TestServerClusterContext context;
    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestProximityRangingCluster, TestStartupDriverInitFailure)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mInitError = CHIP_ERROR_INTERNAL;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INTERNAL);
}

TEST_F(TestProximityRangingCluster, TestStartupShutdownLifecycle)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_FALSE(driver.mShutdownCalled);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    EXPECT_TRUE(driver.mShutdownCalled);
}

TEST_F(TestProximityRangingCluster, TestReadFeatureMap)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    cluster.SetDriver(&driver);
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
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
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
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
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
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
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

TEST_F(TestProximityRangingCluster, TestReadSessionIdListNonEmpty)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mActiveSessionIds = { 5, 10, 15 };

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Attributes::SessionIDList::TypeInfo::DecodableType sessionList;
    EXPECT_EQ(tester.ReadAttribute(Attributes::SessionIDList::Id, sessionList), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionList.IsNull());

    auto iter = sessionList.Value().begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 5);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 10);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 15);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadBleDeviceIdSupported)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mBleRbcConfig = BleRbcConfig{ 0x1234 };

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_NO_ERROR);
    EXPECT_EQ(bleId, static_cast<uint64_t>(0x1234));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadBleDeviceIdUnsupported)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kBleBeaconRssi }));
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint64_t bleId = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::BLEDeviceID::Id, bleId), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadWiFiDevIKSupported)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    WiFiUsdConfig wifiConfig{};
    memset(wifiConfig.deviceIdentityKey, 0xAB, sizeof(wifiConfig.deviceIdentityKey));
    driver.mWiFiUsdConfig = wifiConfig;

    ProximityRangingCluster cluster(
        kTestEndpointId, ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{ Feature::kWiFiUsdProximityDetection }));
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    ByteSpan wifiKey;
    EXPECT_EQ(tester.ReadAttribute(Attributes::WiFiDevIK::Id, wifiKey), CHIP_NO_ERROR);
    EXPECT_EQ(wifiKey.size(), 16u);
    EXPECT_EQ(wifiKey.data()[0], 0xAB);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestReadOptionalAttributeUnsupportedByDriver)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster(kTestEndpointId,
                                    ProximityRangingCluster::Config().WithFeatures(BitMask<Feature>{
                                        Feature::kWiFiUsdProximityDetection, Feature::kBluetoothChannelSounding }));
    cluster.SetDriver(&driver);
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

TEST_F(TestProximityRangingCluster, TestStartRangingAccepted)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mStartRangingResult = ResultCodeEnum::kAccepted;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    if (result.response.has_value())
    {
        auto & response = result.response.value();
        EXPECT_EQ(response.resultCode, ResultCodeEnum::kAccepted);
        EXPECT_FALSE(response.sessionID.IsNull());
        EXPECT_NE(response.sessionID.Value(), 0);
    }
    else
    {
        FAIL();
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStartRangingRejected)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mStartRangingResult = ResultCodeEnum::kRejectedInfeasibleRanging;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;

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

TEST_F(TestProximityRangingCluster, TestStartRangingCapacityExhausted)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    // GenerateSessionId() short-circuits when numSessions == 0, so at least one
    // active session is needed to reach the GetActiveSessionIds error path.
    driver.mActiveSessionIds         = { 1 };
    driver.mGetActiveSessionIdsError = CHIP_ERROR_INTERNAL;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StartRangingRequest::Type request;
    request.technology = RangingTechEnum::kBLEBeaconRSSIRanging;

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().resultCode, ResultCodeEnum::kBusySessionCapacityReached);
        EXPECT_TRUE(result.response.value().sessionID.IsNull());
    }
    else
    {
        FAIL();
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStopRangingSuccess)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mStopRangingError = CHIP_NO_ERROR;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StopRangingRequest::Type stopReq;
    stopReq.sessionID = 1;

    auto result = tester.Invoke(stopReq);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(driver.mLastStopSessionId, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestProximityRangingCluster, TestStopRangingNotFound)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mStopRangingError = CHIP_ERROR_NOT_FOUND;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
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

TEST_F(TestProximityRangingCluster, TestStopRangingFailure)
{
    TestServerClusterContext context;
    MockProximityRangingDriver driver;
    driver.mStopRangingError = CHIP_ERROR_INTERNAL;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::StopRangingRequest::Type stopReq;
    stopReq.sessionID = 1;

    auto result = tester.Invoke(stopReq);
    EXPECT_FALSE(result.IsSuccess());
    if (result.GetStatusCode().has_value())
    {
        EXPECT_EQ(result.GetStatusCode().value().GetStatus(), Protocols::InteractionModel::Status::Failure);
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
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Structs::RangingMeasurementDataStruct::Type measurement;
    measurement.distance.SetNonNull(static_cast<uint16_t>(500));

    driver.mCallback->OnMeasurementData(42, measurement);

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
    MockProximityRangingDriver driver;

    ProximityRangingCluster cluster{ kTestEndpointId, ProximityRangingCluster::Config() };
    cluster.SetDriver(&driver);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    driver.mCallback->OnSessionStopped(7, RangingSessionStatusEnum::kSessionEndTimeReached);

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
