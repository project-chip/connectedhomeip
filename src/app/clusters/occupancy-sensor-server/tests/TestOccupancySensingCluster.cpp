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
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

constexpr OccupancySensing::Structs::HoldTimeLimitsStruct::Type kDefaultHoldTimeLimits = {
    .holdTimeMin = 1,
    .holdTimeMax = 10,
    .holdTimeDefault = 1,
};

constexpr BitMask<OccupancySensing::OccupancyBitmap> kOccupancyUnoccupied = 0;

// Helper function to read a numeric attribute and decode its value.
template <typename T>
CHIP_ERROR ReadNumericAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, T & value)
{
    static_assert(std::is_arithmetic<T>::value, "This helper only supports numeric attribute types.");

    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    if (paths.size() != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<chip::app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
}

// Helper function to write a value to an attribute.
template <typename T>
CHIP_ERROR WriteAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, const T & value)
{
    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    VerifyOrReturnError(paths.size() == 1, CHIP_ERROR_INVALID_ARGUMENT);
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::WriteOperation writeOperation(path);
    chip::app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

// Helper function to read a struct attribute and decode its value.
template <typename T>
CHIP_ERROR ReadAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, T & value)
{
    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    if (paths.size() != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<chip::app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
}

struct TestOccupancySensingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOccupancySensingCluster, TestReadClusterRevision)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    uint16_t clusterRevision;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, OccupancySensing::kRevision);
}

TEST_F(TestOccupancySensingCluster, TestReadFeatureMap)
{
    chip::Test::TestServerClusterContext context;
    uint32_t featureMap;

    {
        constexpr uint32_t featureMapPir = 0x1; // PassiveInfrared
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(OccupancySensing::Feature(featureMapPir))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, featureMapPir);
    }

    {
        constexpr uint32_t featureMapUltrasonic = 0x2; // Ultrasonic
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(OccupancySensing::Feature(featureMapUltrasonic))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
        EXPECT_EQ(featureMap, featureMapUltrasonic);
    }
}

TEST_F(TestOccupancySensingCluster, TestSetHoldTime)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 200, .holdTimeDefault = 100 };
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(100, holdTimeLimitsConfig)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Verify that we can set a valid hold time
    EXPECT_EQ(cluster.SetHoldTime(150), CHIP_NO_ERROR);
    uint16_t holdTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 150);

    // Verify that setting the same value returns NoOp
    EXPECT_EQ(cluster.SetHoldTime(150), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    // Verify that we cannot set a hold time less than the minimum
    EXPECT_EQ(cluster.SetHoldTime(5), Protocols::InteractionModel::Status::ConstraintError);

    // Verify that we cannot set a hold time greater than the maximum
    EXPECT_EQ(cluster.SetHoldTime(250), Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestOccupancySensingCluster, TestWriteHoldTimeAttribute)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 200, .holdTimeDefault = 100 };
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(100, holdTimeLimitsConfig)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Verify that we can write a valid hold time
    EXPECT_EQ(WriteAttribute(cluster, Attributes::HoldTime::Id, static_cast<uint16_t>(150)), CHIP_NO_ERROR);
    uint16_t holdTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 150);

    // Verify that writing to an alias also works
    EXPECT_EQ(WriteAttribute(cluster, Attributes::PIROccupiedToUnoccupiedDelay::Id, static_cast<uint16_t>(120)), CHIP_NO_ERROR);
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 120);

    // Verify that we cannot write a hold time less than the minimum
    EXPECT_EQ(WriteAttribute(cluster, Attributes::HoldTime::Id, static_cast<uint16_t>(5)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Verify that we cannot write a hold time greater than the maximum
    EXPECT_EQ(WriteAttribute(cluster, Attributes::HoldTime::Id, static_cast<uint16_t>(250)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestOccupancySensingCluster, TestHoldTimePersistence)
{
    chip::Test::TestServerClusterContext context;

    constexpr uint16_t kDefaultHoldTime = 100;
    constexpr uint16_t kNewHoldTime     = 150;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10,
                                                                                   .holdTimeMax = 200,
                                                                                   .holdTimeDefault = kDefaultHoldTime };

    // 1. Create a cluster. On startup, it should store the default hold time.
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(kDefaultHoldTime, holdTimeLimitsConfig)
        };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        uint16_t holdTime;
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kDefaultHoldTime);
    }

    // 2. Write a new value to the attribute. This should update the value in persistence.
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(kDefaultHoldTime, holdTimeLimitsConfig)
        };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR); // Startup will load the default value again
        EXPECT_EQ(WriteAttribute(cluster, Attributes::HoldTime::Id, kNewHoldTime), CHIP_NO_ERROR);
    }

    // 3. Create a new cluster instance. It should load the new value from persistence on startup.
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{ kTestEndpointId }.WithHoldTime(kDefaultHoldTime, holdTimeLimitsConfig)
        };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        uint16_t holdTime;
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kNewHoldTime);
    }
}



TEST_F(TestOccupancySensingCluster, TestOccupancySensorTypeAttributesFromFeatures)
{
    chip::Test::TestServerClusterContext context;
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

    // Test case: No bits set (000) -> PIR
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(BitFlags<OccupancySensing::Feature>())};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPir);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: PIR
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(OccupancySensing::Feature::kPassiveInfrared)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPir);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: Ultrasonic
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(OccupancySensing::Feature::kUltrasonic)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);
    }

    // Test case: PIR + Ultrasonic
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPassiveInfrared, OccupancySensing::Feature::kUltrasonic))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPir,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);
    }

    // Test case: PhysicalContact
    {
        OccupancySensingCluster cluster{
            OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(OccupancySensing::Feature::kPhysicalContact)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact);
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact);
    }

    // Test case: PhysicalContact + PIR
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPhysicalContact, OccupancySensing::Feature::kPassiveInfrared))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kPir));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPir);
    }

    // Test case: PhysicalContact + Ultrasonic
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(
            BitFlags<OccupancySensing::Feature>(OccupancySensing::Feature::kPhysicalContact, OccupancySensing::Feature::kUltrasonic))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);
    }

    // Test case: PhysicalContact + PIR + Ultrasonic
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeatures(BitFlags<OccupancySensing::Feature>(
            OccupancySensing::Feature::kPhysicalContact, OccupancySensing::Feature::kPassiveInfrared,
            OccupancySensing::Feature::kUltrasonic))};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeBitmap,
                  BitMask<OccupancySensing::OccupancySensorTypeBitmap>(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kPir,
                                                                       OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic));
        OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeEnum;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorTypeEnum), CHIP_NO_ERROR);
        EXPECT_EQ(occupancySensorTypeEnum, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);
    }
}

TEST_F(TestOccupancySensingCluster, TestReadOptionalHoldTimeAttributes)
{
    chip::Test::TestServerClusterContext context;

    // Case 1: WithHoldTime is not called, so attributes should be unsupported.
    {
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        uint16_t holdTime;
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime),
                  CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits),
                  CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    }

    // Case 2: WithHoldTime is called, so attributes should be readable.
    {
        constexpr uint16_t kHoldTime = 100;
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 200, .holdTimeDefault = kHoldTime };
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(kHoldTime, holdTimeLimitsConfig)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        uint16_t holdTime;
        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kHoldTime);

        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PIROccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kHoldTime);

        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kHoldTime);

        EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kHoldTime);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, holdTimeLimitsConfig.holdTimeMin);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, holdTimeLimitsConfig.holdTimeMax);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, holdTimeLimitsConfig.holdTimeDefault);
    }
}

TEST_F(TestOccupancySensingCluster, TestHoldTimeLimitsConstraints)
{
    chip::Test::TestServerClusterContext context;

    // Test case: holdTimeMin is 0, should be coerced to 1.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 0, .holdTimeMax = 20, .holdTimeDefault = 10 };
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(10, holdTimeLimitsConfig)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 1);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 10);
    }

    // Test case: holdTimeMax is less than holdTimeMin, should be coerced to holdTimeMin.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 15, .holdTimeMax = 10, .holdTimeDefault = 15 };
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(15, holdTimeLimitsConfig)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 15);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 15);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 15);
    }

    // Test case: holdTimeDefault is less than holdTimeMin, should be coerced to holdTimeMin.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 20, .holdTimeDefault = 5 };
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(10, holdTimeLimitsConfig)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 10);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 10);
    }

    // Test case: holdTimeDefault is greater than holdTimeMax, should be coerced to holdTimeMax.
    {
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 20, .holdTimeDefault = 25 };
        OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(10, holdTimeLimitsConfig)};
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
        EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
        EXPECT_EQ(holdTimeLimits.holdTimeMin, 10);
        EXPECT_EQ(holdTimeLimits.holdTimeMax, 20);
        EXPECT_EQ(holdTimeLimits.holdTimeDefault, 20);
    }
}

TEST_F(TestOccupancySensingCluster, TestAttributeListMandatoryOnly)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
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
    chip::Test::TestServerClusterContext context;
    constexpr uint16_t kHoldTime = 100;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10, .holdTimeMax = 200, .holdTimeDefault = kHoldTime };
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(kHoldTime, holdTimeLimitsConfig)};
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
        { true, Attributes::PIROccupiedToUnoccupiedDelay::kMetadataEntry },
        { true, Attributes::UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry },
        { true, Attributes::PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry },
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedMandatoryAttributes), Span(expectedOptionalAttributes)), CHIP_NO_ERROR);
    EXPECT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOccupancySensingCluster, TestSetOccupancy)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<OccupancySensing::OccupancyBitmap> occupancy;

    // Verify that the initial state is unoccupied
    EXPECT_EQ(ReadAttribute(cluster, Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, kOccupancyUnoccupied);

    // Set to occupied and verify
    cluster.SetOccupancy(true);
    EXPECT_EQ(ReadAttribute(cluster, Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, OccupancySensing::OccupancyBitmap::kOccupied);

    // Set to unoccupied and verify
    cluster.SetOccupancy(false);
    EXPECT_EQ(ReadAttribute(cluster, Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, kOccupancyUnoccupied);
}

TEST_F(TestOccupancySensingCluster, TestOccupancyChangedEvent)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OccupancySensing::Events::OccupancyChanged::DecodableType decodedEvent;

    // Set to occupied and verify event
    cluster.SetOccupancy(true);
    EXPECT_EQ(context.EventsGenerator().DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.occupancy, OccupancySensing::OccupancyBitmap::kOccupied);

    // Set to unoccupied and verify event
    cluster.SetOccupancy(false);
    EXPECT_EQ(context.EventsGenerator().DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.occupancy, kOccupancyUnoccupied);
}

} // namespace