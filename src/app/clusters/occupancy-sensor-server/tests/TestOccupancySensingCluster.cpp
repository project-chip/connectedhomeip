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

#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/persistence/AttributePersistence.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Metadata.h>
#include <lib/support/TimerDelegateMock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

constexpr OccupancySensing::Structs::HoldTimeLimitsStruct::Type kDefaultHoldTimeLimits = {
    .holdTimeMin     = 1,
    .holdTimeMax     = 10,
    .holdTimeDefault = 1,
};

constexpr BitMask<OccupancySensing::OccupancyBitmap> kOccupancyUnoccupied = 0;

bool onOccupancyChangedCalled = false;
bool onHoldTimeChangedCalled  = false;

class TestOccupancySensingDelegate : public OccupancySensingDelegate
{
public:
    void OnOccupancyChanged(bool occupied) override { onOccupancyChangedCalled = true; }
    void OnHoldTimeChanged(uint16_t holdTime) override { onHoldTimeChangedCalled = true; }
};

TestOccupancySensingDelegate gTestOccupancySensingDelegate;

struct TestOccupancySensingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        onOccupancyChangedCalled = false;
        onHoldTimeChangedCalled  = false;
    }

    TimerDelegateMock mMockTimerDelegate;
};

TEST_F(TestOccupancySensingCluster, TestReadClusterRevision)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    uint16_t clusterRevision;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, OccupancySensing::kRevision);
}

TEST_F(TestOccupancySensingCluster, TestReadFeatureMap)
{
    chip::Testing::TestServerClusterContext context;
    uint32_t featureMap = 0;

    {
        constexpr uint32_t featureMapPir = 0x1; // PassiveInfrared
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            OccupancySensing::Feature(featureMapPir)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, featureMapPir);
        EXPECT_EQ(cluster.GetFeatureMap(), BitMask<OccupancySensing::Feature>(featureMapPir));
    }

    {
        constexpr uint32_t featureMapUltrasonic = 0x2; // Ultrasonic
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            OccupancySensing::Feature(featureMapUltrasonic)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, featureMapUltrasonic);
        EXPECT_EQ(cluster.GetFeatureMap(), BitMask<OccupancySensing::Feature>(featureMapUltrasonic));
    }
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeAttribute)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = 100 };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(100, holdTimeLimitsConfig,
                                                                                                     mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.IsHoldTimeEnabled());
    chip::Testing::ClusterTester tester(cluster);

    // 1. Verify that we can set a valid hold time via direct method call
    EXPECT_EQ(cluster.SetHoldTime(150), CHIP_NO_ERROR);
    uint16_t holdTime;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 150);
    EXPECT_EQ(cluster.GetHoldTime(), 150);

    // 2. Verify that setting the same value returns NoOp
    EXPECT_EQ(cluster.SetHoldTime(150), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    // 3. Verify that we cannot set a hold time less than the minimum
    EXPECT_EQ(cluster.SetHoldTime(5), Protocols::InteractionModel::Status::ConstraintError);

    // 4. Verify that we cannot set a hold time greater than the maximum
    EXPECT_EQ(cluster.SetHoldTime(250), Protocols::InteractionModel::Status::ConstraintError);

    // 5. Verify that we can write a valid hold time via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(180)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 180);
    EXPECT_EQ(cluster.GetHoldTime(), 180);

    // 6. Verify that PIROccupiedToUnoccupiedDelay is updated when HoldTime is written
    uint16_t pirOccupiedToUnoccupiedDelay;
    EXPECT_EQ(tester.ReadAttribute(Attributes::PIROccupiedToUnoccupiedDelay::Id, pirOccupiedToUnoccupiedDelay), CHIP_NO_ERROR);
    EXPECT_EQ(pirOccupiedToUnoccupiedDelay, 180);

    // 7. Verify that UltrasonicOccupiedToUnoccupiedDelay is updated when HoldTime is written
    uint16_t ultrasonicOccupiedToUnoccupiedDelay;
    EXPECT_EQ(tester.ReadAttribute(Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id, ultrasonicOccupiedToUnoccupiedDelay),
              CHIP_NO_ERROR);
    EXPECT_EQ(ultrasonicOccupiedToUnoccupiedDelay, 180);

    // 8. Verify that PhysicalContactOccupiedToUnoccupiedDelay is updated when HoldTime is written
    uint16_t physicalContactOccupiedToUnoccupiedDelay;
    EXPECT_EQ(
        tester.ReadAttribute(Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id, physicalContactOccupiedToUnoccupiedDelay),
        CHIP_NO_ERROR);
    EXPECT_EQ(physicalContactOccupiedToUnoccupiedDelay, 180);

    // 9. Verify that writing to an alias also works
    EXPECT_EQ(tester.WriteAttribute(Attributes::PIROccupiedToUnoccupiedDelay::Id, static_cast<uint16_t>(120)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 120);

    // 10. Verify that we cannot write a hold time less than the minimum via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(5)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // 11. Verify that we cannot write a hold time greater than the maximum via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(250)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // 12. Verify that SetHoldTime returns UnsupportedAttribute if mHoldTimeDelegate is null (i.e. hold time not configured)
    OccupancySensingCluster clusterNoHoldTime{ OccupancySensingCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(clusterNoHoldTime.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_FALSE(clusterNoHoldTime.IsHoldTimeEnabled());
    EXPECT_EQ(clusterNoHoldTime.SetHoldTime(10), Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestOccupancySensingCluster, TestHoldTimePersistence)
{
    chip::Testing::TestServerClusterContext context;

    constexpr uint16_t kDefaultHoldTime                                        = 100;
    constexpr uint16_t kNewHoldTime                                            = 150;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kDefaultHoldTime };

    // 1. Create a cluster. On startup, it should store the default hold time.
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        uint16_t holdTime = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kDefaultHoldTime);
        EXPECT_EQ(cluster.GetHoldTime(), kDefaultHoldTime);
    }

    // 2. Write a new value to the attribute. This should update the value in persistence.
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR); // Startup will load the default value again
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, kNewHoldTime), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetHoldTime(), kNewHoldTime);
    }

    // 3. Create a new cluster instance. It should load the new value from persistence on startup.
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        uint16_t holdTime = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kNewHoldTime);
        EXPECT_EQ(cluster.GetHoldTime(), kNewHoldTime);
    }
}

TEST_F(TestOccupancySensingCluster, TestOccupancySensorTypeAttributesFromFeatures)
{
    chip::Testing::TestServerClusterContext context;
    // Mapping table from spec:
    //
    //  | Feature Flag Value    | Value of OccupancySensorTypeBitmap    | Value of OccupancySensorType
    //  | PIR | US | PHY        | ===================================== | ============================
    //  | 0   | 0  | 0          | PIR ^*^                               | PIR
    //  | 1   | 0  | 0          | PIR                                   | PIR
    //  | 0   | 1  | 0          | Ultrasonic                            | Ultrasonic
    //  | 1   | 1  | 0          | PIR + Ultrasonic                      | PIRAndUltrasonic
    //  | 0   | 0  | 1          | PhysicalContact                       | PhysicalContact
    //  | 1   | 0  | 1          | PhysicalContact + PIR                 | PIR
    //  | 0   | 1  | 1          | PhysicalContact + Ultrasonic          | Ultrasonic
    //  | 1   | 1  | 1          | PhysicalContact + PIR + Ultrasonic    | PIRAndUltrasonic

    // Test case: No bits set (000) -> Empty Bitmap, PIR Type (as per spec table default)
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            BitFlags<OccupancySensing::Feature>()) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, static_cast<BitMask<OccupancySensing::OccupancySensorTypeBitmap>>(0));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: PIR
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            OccupancySensing::Feature::kPassiveInfrared) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPir);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: Ultrasonic
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            OccupancySensing::Feature::kUltrasonic) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);
    }

    // Test case: PIR + Ultrasonic
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPassiveInfrared,
                                                OccupancySensing::Feature::kUltrasonic)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPir,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);
    }

    // Test case: PhysicalContact
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            OccupancySensing::Feature::kPhysicalContact) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact);
    }

    // Test case: PhysicalContact + PIR
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPhysicalContact,
                                                OccupancySensing::Feature::kPassiveInfrared)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(
            occupancySensorTypeBitmap,
            BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                                                 OccupancySensing::OccupancySensorTypeBitmap::kPir));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: PhysicalContact + Ultrasonic
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPhysicalContact,
                                                OccupancySensing::Feature::kUltrasonic)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(
            occupancySensorTypeBitmap,
            BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                                                 OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);
    }

    // Test case: PhysicalContact + PIR + Ultrasonic
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPhysicalContact,
                                                OccupancySensing::Feature::kPassiveInfrared,
                                                OccupancySensing::Feature::kUltrasonic)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(
                      OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                      OccupancySensing::OccupancySensorTypeBitmap::kPir, OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);
    }
}

TEST_F(TestOccupancySensingCluster, TestReadOptionalHoldTimeAttributes)
{
    chip::Testing::TestServerClusterContext context;
    // Case 2: WithHoldTime is called, so attributes should be readable.
    constexpr uint16_t kHoldTime                                               = 100;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kHoldTime };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
        kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    uint16_t holdTime = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, kHoldTime);

    EXPECT_EQ(tester.ReadAttribute(Attributes::PIROccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, kHoldTime);

    EXPECT_EQ(tester.ReadAttribute(Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, kHoldTime);

    EXPECT_EQ(tester.ReadAttribute(Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, kHoldTime);

    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
    EXPECT_EQ(holdTimeLimits.holdTimeMin, holdTimeLimitsConfig.holdTimeMin);
    EXPECT_EQ(holdTimeLimits.holdTimeMax, holdTimeLimitsConfig.holdTimeMax);
    EXPECT_EQ(holdTimeLimits.holdTimeDefault, holdTimeLimitsConfig.holdTimeDefault);
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeLimitsConstraints)
{
    chip::Testing::TestServerClusterContext context;

    // Test case: holdTimeMin is 0, should be coerced to 1.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 0,
                                                                                       .holdTimeMax     = 20,
                                                                                       .holdTimeDefault = 10 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(10, holdTimeLimitsConfig,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 1);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 10);
    }

    // Test case: holdTimeMax is less than holdTimeMin, should be coerced to holdTimeMin.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 15,
                                                                                       .holdTimeMax     = 10,
                                                                                       .holdTimeDefault = 15 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(15, holdTimeLimitsConfig,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 15);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 15);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 15);
    }

    // Test case: holdTimeDefault is less than holdTimeMin, should be coerced to holdTimeMin.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                       .holdTimeMax     = 20,
                                                                                       .holdTimeDefault = 5 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(10, holdTimeLimitsConfig,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 10);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 10);
    }

    // Test case: holdTimeDefault is greater than holdTimeMax, should be coerced to holdTimeMax.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                       .holdTimeMax     = 20,
                                                                                       .holdTimeDefault = 25 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(10, holdTimeLimitsConfig,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 10);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 20);
    }
}

TEST_F(TestOccupancySensingCluster, TestAttributeListMandatoryOnly)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

    const DataModel::AttributeEntry expectedAttributes[] = {
        Attributes::Occupancy::kMetadataEntry,
        Attributes::OccupancySensorType::kMetadataEntry,
        Attributes::OccupancySensorTypeBitmap::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedAttributes), {}), CHIP_NO_ERROR);
    EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOccupancySensingCluster, TestAttributeListWithOptionalAttributes)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kHoldTime                                               = 100;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kHoldTime };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
        kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

    const DataModel::AttributeEntry expectedMandatoryAttributes[] = {
        Attributes::Occupancy::kMetadataEntry,
        Attributes::OccupancySensorType::kMetadataEntry,
        Attributes::OccupancySensorTypeBitmap::kMetadataEntry,
    };

    const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
        { true, Attributes::HoldTime::kMetadataEntry },
        { true, Attributes::HoldTimeLimits::kMetadataEntry },
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
    EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOccupancySensingCluster, TestAttributeListWithFeatureSpecificOptionalAttributes)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kHoldTime                                               = 100;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kHoldTime };

    const DataModel::AttributeEntry expectedMandatoryAttributes[] = {
        Attributes::Occupancy::kMetadataEntry,
        Attributes::OccupancySensorType::kMetadataEntry,
        Attributes::OccupancySensorTypeBitmap::kMetadataEntry,
    };

    // Test case 1: Only PassiveInfrared feature enabled
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(Feature::kPassiveInfrared) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
            { true, Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }

    // Test case 2: Only Ultrasonic feature enabled
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(Feature::kUltrasonic) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
            { true, Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }

    // Test case 3: Only PhysicalContact feature enabled
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(Feature::kPhysicalContact) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
            { true, Attributes::PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }

    // Test case 4: PassiveInfrared and Ultrasonic features enabled
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(BitFlags<Feature>(Feature::kPassiveInfrared, Feature::kUltrasonic)) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
            { true, Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
            { true, Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }
}

TEST_F(TestOccupancySensingCluster, TestSetOccupancy)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    BitMask<OccupancySensing::OccupancyBitmap> occupancy;

    // Verify that the initial state is unoccupied
    EXPECT_EQ(tester.ReadAttribute(Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, kOccupancyUnoccupied);
    EXPECT_FALSE(cluster.IsOccupied());

    // Set to occupied and verify
    cluster.SetOccupancy(true);
    EXPECT_EQ(tester.ReadAttribute(Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, OccupancySensing::OccupancyBitmap::kOccupied);
    EXPECT_TRUE(cluster.IsOccupied());

    // Set to unoccupied and verify
    cluster.SetOccupancy(false);
    EXPECT_EQ(tester.ReadAttribute(Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, kOccupancyUnoccupied);
    EXPECT_FALSE(cluster.IsOccupied());
}

TEST_F(TestOccupancySensingCluster, TestOccupancyChangedEvent)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OccupancySensing::Events::OccupancyChanged::DecodableType decodedEvent;

    // Set to occupied and verify event
    cluster.SetOccupancy(true);
    auto eventInfo = context.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    if (eventInfo.has_value()) // Redundant check to avoid clang tidy "error: unchecked access to optional value"
    {
        EXPECT_EQ(eventInfo->GetEventData(decodedEvent), CHIP_NO_ERROR);
        EXPECT_EQ(decodedEvent.occupancy, OccupancySensing::OccupancyBitmap::kOccupied);
    }

    // Set to unoccupied and verify event
    cluster.SetOccupancy(false);
    eventInfo = context.EventsGenerator().GetNextEvent();
    ASSERT_NE(eventInfo, std::nullopt);
    if (eventInfo.has_value()) // Redundant check to avoid clang tidy "error: unchecked access to optional value"
    {
        EXPECT_EQ(eventInfo->GetEventData(decodedEvent), CHIP_NO_ERROR);
        EXPECT_EQ(decodedEvent.occupancy, kOccupancyUnoccupied);
    }
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeLimitsAttribute)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(10, kDefaultHoldTimeLimits,
                                                                                                     mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // 1. Verify setting valid limits
    {
        const OccupancySensing::Structs::HoldTimeLimitsStruct::Type newHoldTimeLimits = {
            .holdTimeMin     = 5,
            .holdTimeMax     = 15,
            .holdTimeDefault = 10,
        };

        cluster.SetHoldTimeLimits(newHoldTimeLimits);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type readHoldTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, readHoldTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(readHoldTimeLimits.holdTimeMin, newHoldTimeLimits.holdTimeMin);
        EXPECT_EQ(readHoldTimeLimits.holdTimeMax, newHoldTimeLimits.holdTimeMax);
        EXPECT_EQ(readHoldTimeLimits.holdTimeDefault, newHoldTimeLimits.holdTimeDefault);
        EXPECT_EQ(cluster.GetHoldTimeLimits().holdTimeMin, newHoldTimeLimits.holdTimeMin);
        EXPECT_EQ(cluster.GetHoldTimeLimits().holdTimeMax, newHoldTimeLimits.holdTimeMax);
        EXPECT_EQ(cluster.GetHoldTimeLimits().holdTimeDefault, newHoldTimeLimits.holdTimeDefault);
    }

    // 2. Verify sanitization of invalid input (Max < Min, Default < Min)
    {
        const OccupancySensing::Structs::HoldTimeLimitsStruct::Type invalidHoldTimeLimits = {
            .holdTimeMin     = 20,
            .holdTimeMax     = 10,
            .holdTimeDefault = 5,
        };

        cluster.SetHoldTimeLimits(invalidHoldTimeLimits);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type readHoldTimeLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, readHoldTimeLimits), CHIP_NO_ERROR);

        // Expect the values to be sanitized
        EXPECT_EQ(readHoldTimeLimits.holdTimeMin, 20);
        EXPECT_EQ(readHoldTimeLimits.holdTimeMax, 20);     // Should be coerced to holdTimeMin
        EXPECT_EQ(readHoldTimeLimits.holdTimeDefault, 20); // Should be clamped to the new min/max
    }

    // 3. Verify constraint: HoldTimeMax must be at least 10 (unless min is higher)
    //    Here, we set max=5 (and min=1), so max should be coerced to 10.
    {
        const OccupancySensing::Structs::HoldTimeLimitsStruct::Type limitsMaxTooSmall = {
            .holdTimeMin     = 1,
            .holdTimeMax     = 5,
            .holdTimeDefault = 5,
        };
        cluster.SetHoldTimeLimits(limitsMaxTooSmall);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type readLimits;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, readLimits), CHIP_NO_ERROR);
        EXPECT_EQ(readLimits.holdTimeMin, 1);
        EXPECT_EQ(readLimits.holdTimeMax, 10);
        EXPECT_EQ(readLimits.holdTimeDefault, 5);
    }
}

TEST_F(TestOccupancySensingCluster, TestOccupancyHoldTime)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kHoldTime                                               = 5;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 1,
                                                                                   .holdTimeMax     = 10,
                                                                                   .holdTimeDefault = 1 };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
        kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 1. Set Occupancy to occupied
    cluster.SetOccupancy(true);
    EXPECT_TRUE(cluster.IsOccupied());

    // 2. Set Occupancy to unoccupied, timer should start
    cluster.SetOccupancy(false);
    EXPECT_TRUE(cluster.IsOccupied()); // Should still be occupied
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 3. Advance clock by less than hold time, should still be occupied
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(kHoldTime - 1));
    EXPECT_TRUE(cluster.IsOccupied());

    // 4. Advance clock by the remaining time, should be unoccupied
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_FALSE(cluster.IsOccupied());
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestOccupancySensingCluster, TestOccupancyHoldTimeRetrigger)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kHoldTime                                               = 5;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 1,
                                                                                   .holdTimeMax     = 10,
                                                                                   .holdTimeDefault = 1 };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(
        kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 1. Set Occupancy to occupied
    cluster.SetOccupancy(true);
    EXPECT_TRUE(cluster.IsOccupied());

    // 2. Set Occupancy to unoccupied, timer should start
    cluster.SetOccupancy(false);
    EXPECT_TRUE(cluster.IsOccupied()); // Should still be occupied
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 3. Advance clock by less than hold time
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(kHoldTime - 2));
    EXPECT_TRUE(cluster.IsOccupied());

    // 4. Set Occupancy to occupied again, timer should be cancelled
    cluster.SetOccupancy(true);
    EXPECT_TRUE(cluster.IsOccupied());
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 5. Set Occupancy to unoccupied again, timer should start
    cluster.SetOccupancy(false);
    EXPECT_TRUE(cluster.IsOccupied());
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 6. Advance clock by the full hold time, should be unoccupied
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(kHoldTime));
    EXPECT_FALSE(cluster.IsOccupied());
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestOccupancySensingCluster, TestClusterDelegate)
{
    chip::Testing::TestServerClusterContext context;
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                         .WithHoldTime(10, kDefaultHoldTimeLimits, mMockTimerDelegate)
                                         .WithDelegate(&gTestOccupancySensingDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 1. Verify OnOccupancyChanged callback
    onOccupancyChangedCalled = false;
    cluster.SetOccupancy(true);
    EXPECT_TRUE(onOccupancyChangedCalled);

    onOccupancyChangedCalled = false;
    cluster.SetOccupancy(false);
    // With hold time enabled, the transition to unoccupied happens after the timer fires.
    EXPECT_FALSE(onOccupancyChangedCalled);

    // Advance the timer to trigger the transition
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(10));
    EXPECT_TRUE(onOccupancyChangedCalled);

    // 2. Verify OnHoldTimeChanged callback
    onHoldTimeChangedCalled = false;
    EXPECT_EQ(cluster.SetHoldTime(5), CHIP_NO_ERROR);
    EXPECT_TRUE(onHoldTimeChangedCalled);
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeDecreaseWithActiveTimer)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kInitialHoldTime                                                  = 20;
    constexpr OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 1,
                                                                                             .holdTimeMax     = 100,
                                                                                             .holdTimeDefault = 1 };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                         .WithHoldTime(kInitialHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                         .WithDelegate(&gTestOccupancySensingDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 1. Set occupancy to true, then false to start the timer.
    cluster.SetOccupancy(true);
    cluster.SetOccupancy(false);
    EXPECT_TRUE(cluster.IsOccupied());
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 2. Advance the clock by 10 seconds.
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(10));
    EXPECT_TRUE(cluster.IsOccupied());

    // 3. Decrease the hold time to a value less than the elapsed time.
    // This should cause an immediate transition to the unoccupied state.
    constexpr uint16_t kDecreasedHoldTime = 5;
    EXPECT_EQ(cluster.SetHoldTime(kDecreasedHoldTime), CHIP_NO_ERROR);
    EXPECT_FALSE(cluster.IsOccupied());
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeIncreaseWithActiveTimer)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kInitialHoldTime                                                  = 20;
    constexpr OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 1,
                                                                                             .holdTimeMax     = 100,
                                                                                             .holdTimeDefault = kInitialHoldTime };
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                         .WithHoldTime(kInitialHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                         .WithDelegate(&gTestOccupancySensingDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 1. Set occupancy to true, then false to start the timer.
    cluster.SetOccupancy(true);
    cluster.SetOccupancy(false);
    EXPECT_TRUE(cluster.IsOccupied());

    // 2. Advance the clock by 10 seconds.
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(10));
    EXPECT_TRUE(cluster.IsOccupied());

    // 3. Increase the hold time. The timer should be restarted for the remaining duration.
    constexpr uint16_t kIncreasedHoldTime = 30;
    EXPECT_EQ(cluster.SetHoldTime(kIncreasedHoldTime), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.IsOccupied());
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // 4. Advance the clock by another 19 seconds. The state should still be occupied,
    // as the total elapsed time is 29s, which is less than the new hold time of 30s.
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(19));
    EXPECT_TRUE(cluster.IsOccupied());

    // 5. Advance the clock by 1 more second. The total elapsed time is now 30 seconds,
    // so the state should transition to unoccupied.
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_FALSE(cluster.IsOccupied());
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestOccupancySensingCluster, TestConstructorClampsInitialHoldTime)
{
    // holdTime is below the min, it should be clamped to min.
    {
        chip::Testing::TestServerClusterContext context;
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type limits = { .holdTimeMin     = 10,
                                                                         .holdTimeMax     = 20,
                                                                         .holdTimeDefault = 15 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(5, limits,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetHoldTime(), 10);
    }

    // holdTime is above the max, it should be clamped to max.
    {
        chip::Testing::TestServerClusterContext context;
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type limits = { .holdTimeMin     = 10,
                                                                         .holdTimeMax     = 20,
                                                                         .holdTimeDefault = 15 };
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(25, limits,
                                                                                                         mMockTimerDelegate) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetHoldTime(), 20);
    }
}

TEST_F(TestOccupancySensingCluster, TestStartupWithInvalidPersistedHoldTime)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kDefaultHoldTime                          = 15;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type limits = { .holdTimeMin     = 10,
                                                                     .holdTimeMax     = 20,
                                                                     .holdTimeDefault = kDefaultHoldTime };

    // 1. Manually write an invalid hold time to storage.
    const uint16_t invalidHoldTime = 5; // Below the minimum of 10
    EXPECT_EQ(context.AttributePersistenceProvider().WriteValue(
                  { kTestEndpointId, OccupancySensing::Id, Attributes::HoldTime::Id },
                  { reinterpret_cast<const uint8_t *>(&invalidHoldTime), sizeof(invalidHoldTime) }),
              CHIP_NO_ERROR);

    // 2. Create a new cluster instance. On startup, it should load the invalid value,
    //    detect that it's out of bounds, and coerce it to the default.
    OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(kDefaultHoldTime, limits,
                                                                                                     mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // 3. Verify that the hold time is now the default, not the invalid value.
    EXPECT_EQ(cluster.GetHoldTime(), kDefaultHoldTime);
}

TEST_F(TestOccupancySensingCluster, TestDeprecatedAttributesVisibility)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kHoldTime                                               = 100;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kHoldTime };

    const DataModel::AttributeEntry expectedMandatoryAttributes[] = {
        Attributes::Occupancy::kMetadataEntry,
        Attributes::OccupancySensorType::kMetadataEntry,
        Attributes::OccupancySensorTypeBitmap::kMetadataEntry,
    };

    // Test Case 1: Deprecated attributes enabled (default)
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(BitFlags<Feature>(Feature::kPassiveInfrared, Feature::kUltrasonic,
                                                                             Feature::kPhysicalContact))
                                             .WithDeprecatedAttributes(true) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
            { true, Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
            { true, Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
            { true, Attributes::PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }

    // Test Case 2: Deprecated attributes disabled
    {
        OccupancySensingCluster cluster{ OccupancySensingCluster::Config{ kTestEndpointId }
                                             .WithHoldTime(kHoldTime, holdTimeLimitsConfig, mMockTimerDelegate)
                                             .WithFeatures(BitFlags<Feature>(Feature::kPassiveInfrared, Feature::kUltrasonic,
                                                                             Feature::kPhysicalContact))
                                             .WithDeprecatedAttributes(false) };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OccupancySensing::Id), attributes), CHIP_NO_ERROR);

        const AttributeListBuilder::OptionalAttributeEntry expectedOptionalAttributes[] = {
            { true, Attributes::HoldTime::kMetadataEntry },
            { true, Attributes::HoldTimeLimits::kMetadataEntry },
        };

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
        EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
    }
}

} // namespace
