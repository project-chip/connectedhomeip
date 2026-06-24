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

#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <clusters/WaterHeaterManagement/Events.h>
#include <clusters/WaterHeaterManagement/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockWHMDelegate : public WaterHeaterManagement::Delegate
{
public:
    // Configurable return values for Get* methods
    BitMask<WaterHeaterHeatSourceBitmap> heaterTypes{ WaterHeaterHeatSourceBitmap::kImmersionElement1 };
    BitMask<WaterHeaterHeatSourceBitmap> heatDemand{ WaterHeaterHeatSourceBitmap::kImmersionElement1 };
    uint16_t tankVolume              = 100;
    Energy_mWh estimatedHeatRequired = 5000;
    Percent tankPercentage           = 80;
    BoostStateEnum boostState        = BoostStateEnum::kInactive;

    // HandleBoost call tracking
    bool handleBoostCalled     = false;
    uint32_t lastBoostDuration = 0;
    Optional<bool> lastBoostOneShot;
    Optional<bool> lastBoostEmergencyBoost;
    Optional<int16_t> lastBoostTemporarySetpoint;
    Optional<Percent> lastBoostTargetPercentage;
    Optional<Percent> lastBoostTargetReheat;
    Protocols::InteractionModel::Status handleBoostStatus = Protocols::InteractionModel::Status::Success;

    // HandleCancelBoost call tracking
    bool handleCancelBoostCalled                                = false;
    Protocols::InteractionModel::Status handleCancelBoostStatus = Protocols::InteractionModel::Status::Success;

    // Delegate interface
    BitMask<WaterHeaterHeatSourceBitmap> GetHeaterTypes() override { return heaterTypes; }
    BitMask<WaterHeaterHeatSourceBitmap> GetHeatDemand() override { return heatDemand; }
    uint16_t GetTankVolume() override { return tankVolume; }
    Energy_mWh GetEstimatedHeatRequired() override { return estimatedHeatRequired; }
    Percent GetTankPercentage() override { return tankPercentage; }
    BoostStateEnum GetBoostState() override { return boostState; }

    Protocols::InteractionModel::Status HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                    Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                                    Optional<Percent> targetReheat) override
    {
        handleBoostCalled          = true;
        lastBoostDuration          = duration;
        lastBoostOneShot           = oneShot;
        lastBoostEmergencyBoost    = emergencyBoost;
        lastBoostTemporarySetpoint = temporarySetpoint;
        lastBoostTargetPercentage  = targetPercentage;
        lastBoostTargetReheat      = targetReheat;
        return handleBoostStatus;
    }

    Protocols::InteractionModel::Status HandleCancelBoost() override
    {
        handleCancelBoostCalled = true;
        return handleCancelBoostStatus;
    }
};

// Default fixture: cluster with no feature flags
struct TestWaterHeaterManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override {}

    MockWHMDelegate mDelegate;
    WaterHeaterManagementCluster mCluster{ kTestEndpointId, mDelegate, BitMask<Feature>(0) };
    ClusterTester mClusterTester{ mCluster };
};

// ---------------------------------------------------------------------------
// Metadata tests
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestAttributeListMandatoryOnly)
{
    // No features: only mandatory attributes (HeaterTypes, HeatDemand, BoostState)
    std::vector<DataModel::AttributeEntry> expected(WaterHeaterManagement::Attributes::kMandatoryMetadata.begin(),
                                                    WaterHeaterManagement::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, expected));
}

TEST_F(TestWaterHeaterManagementCluster, TestAttributeListWithEnergyManagement)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kEnergyManagement));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    std::vector<DataModel::AttributeEntry> expected(WaterHeaterManagement::Attributes::kMandatoryMetadata.begin(),
                                                    WaterHeaterManagement::Attributes::kMandatoryMetadata.end());
    expected.push_back(TankVolume::kMetadataEntry);
    expected.push_back(EstimatedHeatRequired::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected));
}

TEST_F(TestWaterHeaterManagementCluster, TestAttributeListWithTankPercent)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    std::vector<DataModel::AttributeEntry> expected(WaterHeaterManagement::Attributes::kMandatoryMetadata.begin(),
                                                    WaterHeaterManagement::Attributes::kMandatoryMetadata.end());
    expected.push_back(TankPercentage::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected));
}

TEST_F(TestWaterHeaterManagementCluster, TestAttributeListAllFeatures)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate,
                                         BitMask<Feature>(Feature::kEnergyManagement, Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    std::vector<DataModel::AttributeEntry> expected(WaterHeaterManagement::Attributes::kMandatoryMetadata.begin(),
                                                    WaterHeaterManagement::Attributes::kMandatoryMetadata.end());
    expected.push_back(TankVolume::kMetadataEntry);
    expected.push_back(EstimatedHeatRequired::kMetadataEntry);
    expected.push_back(TankPercentage::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected));
}

TEST_F(TestWaterHeaterManagementCluster, TestAcceptedCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              {
                                                  Commands::Boost::kMetadataEntry,
                                                  Commands::CancelBoost::kMetadataEntry,
                                              }));
}

// ---------------------------------------------------------------------------
// Attribute read tests
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, WaterHeaterManagement::kRevision);
}

TEST_F(TestWaterHeaterManagementCluster, TestReadFeatureMap)
{
    MockWHMDelegate delegate;
    auto features = BitMask<Feature>(Feature::kEnergyManagement, Feature::kTankPercent);
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, features);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t featureMap = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, features.Raw());
}

TEST_F(TestWaterHeaterManagementCluster, TestReadMandatoryAttributes)
{
    // HeaterTypes
    mDelegate.heaterTypes = BitMask<WaterHeaterHeatSourceBitmap>(WaterHeaterHeatSourceBitmap::kHeatPump);
    BitMask<WaterHeaterHeatSourceBitmap> heaterTypes;
    EXPECT_EQ(mClusterTester.ReadAttribute(HeaterTypes::Id, heaterTypes), CHIP_NO_ERROR);
    EXPECT_EQ(heaterTypes, mDelegate.heaterTypes);

    // HeatDemand
    mDelegate.heatDemand = BitMask<WaterHeaterHeatSourceBitmap>(WaterHeaterHeatSourceBitmap::kBoiler);
    BitMask<WaterHeaterHeatSourceBitmap> heatDemand;
    EXPECT_EQ(mClusterTester.ReadAttribute(HeatDemand::Id, heatDemand), CHIP_NO_ERROR);
    EXPECT_EQ(heatDemand, mDelegate.heatDemand);

    // BoostState
    mDelegate.boostState = BoostStateEnum::kActive;
    BoostStateEnum boostState;
    EXPECT_EQ(mClusterTester.ReadAttribute(BoostState::Id, boostState), CHIP_NO_ERROR);
    EXPECT_EQ(boostState, BoostStateEnum::kActive);
}

TEST_F(TestWaterHeaterManagementCluster, TestReadOptionalAttributesWithEnergyManagement)
{
    MockWHMDelegate delegate;
    delegate.tankVolume            = 200;
    delegate.estimatedHeatRequired = 12500;

    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kEnergyManagement));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t tankVolume = 0;
    EXPECT_EQ(tester.ReadAttribute(TankVolume::Id, tankVolume), CHIP_NO_ERROR);
    EXPECT_EQ(tankVolume, 200u);

    Energy_mWh estimatedHeat = 0;
    EXPECT_EQ(tester.ReadAttribute(EstimatedHeatRequired::Id, estimatedHeat), CHIP_NO_ERROR);
    EXPECT_EQ(estimatedHeat, static_cast<Energy_mWh>(12500));
}

TEST_F(TestWaterHeaterManagementCluster, TestReadOptionalAttributesWithTankPercent)
{
    MockWHMDelegate delegate;
    delegate.tankPercentage = 65;

    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Percent tankPercentage = 0;
    EXPECT_EQ(tester.ReadAttribute(TankPercentage::Id, tankPercentage), CHIP_NO_ERROR);
    EXPECT_EQ(tankPercentage, static_cast<Percent>(65));
}

TEST_F(TestWaterHeaterManagementCluster, TestOptionalAttributesUnsupportedWithoutFeature)
{
    // Without kEnergyManagement, TankVolume and EstimatedHeatRequired must be absent
    uint16_t tankVolume = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(TankVolume::Id, tankVolume), Protocols::InteractionModel::Status::UnsupportedAttribute);

    Energy_mWh estimatedHeat = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(EstimatedHeatRequired::Id, estimatedHeat),
              Protocols::InteractionModel::Status::UnsupportedAttribute);

    // Without kTankPercent, TankPercentage must be absent
    Percent tankPercentage = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(TankPercentage::Id, tankPercentage),
              Protocols::InteractionModel::Status::UnsupportedAttribute);
}

// ---------------------------------------------------------------------------
// Boost command validation tests (require kTankPercent feature)
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestBoostValidation_PercentageOutOfRange)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Boost::Type req;
    req.boostInfo.duration         = 3600;
    req.boostInfo.targetPercentage = MakeOptional(static_cast<Percent>(101));

    auto result = tester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
    else
    {
        FAIL();
    }
    EXPECT_FALSE(delegate.handleBoostCalled);
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostValidation_ReheatOutOfRange)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Boost::Type req;
    req.boostInfo.duration         = 3600;
    req.boostInfo.targetPercentage = MakeOptional(static_cast<Percent>(80));
    req.boostInfo.targetReheat     = MakeOptional(static_cast<Percent>(101));

    auto result = tester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
    else
    {
        FAIL();
    }
    EXPECT_FALSE(delegate.handleBoostCalled);
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostValidation_ReheatWithoutPercentage)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Boost::Type req;
    req.boostInfo.duration     = 3600;
    req.boostInfo.targetReheat = MakeOptional(static_cast<Percent>(50));
    // targetPercentage intentionally omitted

    auto result = tester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
    else
    {
        FAIL();
    }
    EXPECT_FALSE(delegate.handleBoostCalled);
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostValidation_ReheatAndOneShotMutuallyExclusive)
{
    MockWHMDelegate delegate;
    WaterHeaterManagementCluster cluster(kTestEndpointId, delegate, BitMask<Feature>(Feature::kTankPercent));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Boost::Type req;
    req.boostInfo.duration         = 3600;
    req.boostInfo.targetPercentage = MakeOptional(static_cast<Percent>(80));
    req.boostInfo.targetReheat     = MakeOptional(static_cast<Percent>(50));
    req.boostInfo.oneShot          = MakeOptional(true);

    auto result = tester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
    else
    {
        FAIL();
    }
    EXPECT_FALSE(delegate.handleBoostCalled);
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostValidation_PercentFieldsRequireFeature)
{
    // Default fixture has no features — targetPercentage must be rejected
    Commands::Boost::Type req;
    req.boostInfo.duration         = 3600;
    req.boostInfo.targetPercentage = MakeOptional(static_cast<Percent>(80));

    auto result = mClusterTester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);
    }
    else
    {
        FAIL();
    }
    EXPECT_FALSE(mDelegate.handleBoostCalled);

    // Also test targetReheat alone
    Commands::Boost::Type req2;
    req2.boostInfo.duration     = 3600;
    req2.boostInfo.targetReheat = MakeOptional(static_cast<Percent>(50));

    auto result2 = mClusterTester.Invoke<Commands::Boost::Type>(req2);
    EXPECT_FALSE(result2.IsSuccess());
}

// ---------------------------------------------------------------------------
// Boost command success + delegate forwarding
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestBoostCommandValid)
{
    Commands::Boost::Type req;
    req.boostInfo.duration       = 7200;
    req.boostInfo.oneShot        = MakeOptional(true);
    req.boostInfo.emergencyBoost = MakeOptional(false);

    auto result = mClusterTester.Invoke<Commands::Boost::Type>(req);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(mDelegate.handleBoostCalled);
    EXPECT_EQ(mDelegate.lastBoostDuration, 7200u);
    ASSERT_TRUE(mDelegate.lastBoostOneShot.HasValue());
    EXPECT_EQ(mDelegate.lastBoostOneShot.Value(), true);
    ASSERT_TRUE(mDelegate.lastBoostEmergencyBoost.HasValue());
    EXPECT_EQ(mDelegate.lastBoostEmergencyBoost.Value(), false);
    EXPECT_FALSE(mDelegate.lastBoostTemporarySetpoint.HasValue());
    EXPECT_FALSE(mDelegate.lastBoostTargetPercentage.HasValue());
    EXPECT_FALSE(mDelegate.lastBoostTargetReheat.HasValue());
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostCommandDelegateFailure)
{
    mDelegate.handleBoostStatus = Protocols::InteractionModel::Status::Failure;

    Commands::Boost::Type req;
    req.boostInfo.duration = 3600;

    auto result = mClusterTester.Invoke<Commands::Boost::Type>(req);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_TRUE(mDelegate.handleBoostCalled);
}

// ---------------------------------------------------------------------------
// CancelBoost command
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestCancelBoostCommandSuccess)
{
    auto result = mClusterTester.Invoke<Commands::CancelBoost::Type>(Commands::CancelBoost::Type());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(mDelegate.handleCancelBoostCalled);
}

TEST_F(TestWaterHeaterManagementCluster, TestCancelBoostCommandDelegateFailure)
{
    mDelegate.handleCancelBoostStatus = Protocols::InteractionModel::Status::Busy;

    auto result = mClusterTester.Invoke<Commands::CancelBoost::Type>(Commands::CancelBoost::Type());
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_TRUE(mDelegate.handleCancelBoostCalled);
    auto code = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (code.has_value())
    {
        EXPECT_EQ(code->GetStatus(), Protocols::InteractionModel::Status::Busy);
    }
    else
    {
        FAIL();
    }
}

// ---------------------------------------------------------------------------
// Event generation tests
// ---------------------------------------------------------------------------

TEST_F(TestWaterHeaterManagementCluster, TestBoostStartedEvent)
{
    constexpr uint32_t kDuration = 3600;
    auto oneShot                 = MakeOptional(true);
    auto emergencyBoost          = MakeOptional(false);
    auto temporarySetpoint       = MakeOptional(static_cast<int16_t>(6000));
    Optional<Percent> noPercent;
    Optional<Percent> noReheat;

    EXPECT_EQ(mCluster.GenerateBoostStartedEvent(kDuration, oneShot, emergencyBoost, temporarySetpoint, noPercent, noReheat),
              CHIP_NO_ERROR);

    auto evt = mClusterTester.GetNextGeneratedEvent();
    if (evt.has_value())
    {
        Events::BoostStarted::DecodableType decoded;
        EXPECT_EQ(evt->GetEventData(decoded), CHIP_NO_ERROR);
        EXPECT_EQ(decoded.boostInfo.duration, kDuration);
        ASSERT_TRUE(decoded.boostInfo.oneShot.HasValue());
        EXPECT_EQ(decoded.boostInfo.oneShot.Value(), true);
        ASSERT_TRUE(decoded.boostInfo.emergencyBoost.HasValue());
        EXPECT_EQ(decoded.boostInfo.emergencyBoost.Value(), false);
        ASSERT_TRUE(decoded.boostInfo.temporarySetpoint.HasValue());
        EXPECT_EQ(decoded.boostInfo.temporarySetpoint.Value(), static_cast<int16_t>(6000));
        EXPECT_FALSE(decoded.boostInfo.targetPercentage.HasValue());
        EXPECT_FALSE(decoded.boostInfo.targetReheat.HasValue());
    }
    else
    {
        FAIL();
    }

    // Queue is now empty
    EXPECT_FALSE(mClusterTester.GetNextGeneratedEvent().has_value());
}

TEST_F(TestWaterHeaterManagementCluster, TestBoostEndedEvent)
{
    EXPECT_EQ(mCluster.GenerateBoostEndedEvent(), CHIP_NO_ERROR);

    auto evt = mClusterTester.GetNextGeneratedEvent();
    if (evt.has_value())
    {
        Events::BoostEnded::DecodableType decoded;
        EXPECT_EQ(evt->GetEventData(decoded), CHIP_NO_ERROR);
    }
    else
    {
        FAIL();
    }

    // Queue is now empty
    EXPECT_FALSE(mClusterTester.GetNextGeneratedEvent().has_value());
}

} // namespace
