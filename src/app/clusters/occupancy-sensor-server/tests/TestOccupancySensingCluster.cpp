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

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr uint32_t kDefaultFeatureMap = 0;

constexpr OccupancySensing::Structs::HoldTimeLimitsStruct::Type kDefaultHoldTimeLimits = {
    .holdTimeMin = 0,
    .holdTimeMax = 0,
    .holdTimeDefault = 0,
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
    EXPECT_EQ(ReadNumericAttribute(cluster, Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, OccupancySensing::kRevision);
}

TEST_F(TestOccupancySensingCluster, TestReadFeatureMap)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithFeature(kDefaultFeatureMap)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    uint32_t featureMap;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, kDefaultFeatureMap);
}

TEST_F(TestOccupancySensingCluster, TestReadHoldTime)
{
    chip::Test::TestServerClusterContext context;
    constexpr uint16_t kHoldTime = 100;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTime(kHoldTime)};
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
}

TEST_F(TestOccupancySensingCluster, TestReadHoldTimeLimits)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithHoldTimeLimits(kDefaultHoldTimeLimits)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits;
    EXPECT_EQ(ReadAttribute(cluster, Attributes::HoldTimeLimits::Id, holdTimeLimits), CHIP_NO_ERROR);
    EXPECT_EQ(holdTimeLimits.holdTimeMin, kDefaultHoldTimeLimits.holdTimeMin);
    EXPECT_EQ(holdTimeLimits.holdTimeMax, kDefaultHoldTimeLimits.holdTimeMax);
    EXPECT_EQ(holdTimeLimits.holdTimeDefault, kDefaultHoldTimeLimits.holdTimeDefault);
}

TEST_F(TestOccupancySensingCluster, TestReadOccupancy)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<OccupancySensing::OccupancyBitmap> occupancy;
    EXPECT_EQ(ReadAttribute(cluster, Attributes::Occupancy::Id, occupancy), CHIP_NO_ERROR);
    EXPECT_EQ(occupancy, kOccupancyUnoccupied);
}

TEST_F(TestOccupancySensingCluster, TestReadOccupancySensorType)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(OccupancySensing::OccupancySensorTypeBitmap::kPir)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OccupancySensing::OccupancySensorTypeEnum occupancySensorType;
    EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorType::Id, occupancySensorType), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorType, OccupancySensing::OccupancySensorTypeEnum::kPir);
}

TEST_F(TestOccupancySensingCluster, TestReadOccupancySensorTypeBitmap)
{
    chip::Test::TestServerClusterContext context;
    OccupancySensingCluster cluster{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(OccupancySensing::OccupancySensorTypeBitmap::kPir)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap;
    EXPECT_EQ(ReadAttribute(cluster, Attributes::OccupancySensorTypeBitmap::Id, occupancySensorTypeBitmap), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypeBitmap, OccupancySensing::OccupancySensorTypeBitmap::kPir);
}

TEST_F(TestOccupancySensingCluster, TestOccupancySensorTypeEnumFromBitmap)
{
    chip::Test::TestServerClusterContext context;

    // Test case: PIR
    OccupancySensingCluster clusterPIR{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(OccupancySensing::OccupancySensorTypeBitmap::kPir)};
    EXPECT_EQ(clusterPIR.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypePIR;
    EXPECT_EQ(ReadAttribute(clusterPIR, Attributes::OccupancySensorType::Id, occupancySensorTypePIR), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypePIR, OccupancySensing::OccupancySensorTypeEnum::kPir);

    // Test case: Ultrasonic
    OccupancySensingCluster clusterUltrasonic{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic)};
    EXPECT_EQ(clusterUltrasonic.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeUltrasonic;
    EXPECT_EQ(ReadAttribute(clusterUltrasonic, Attributes::OccupancySensorType::Id, occupancySensorTypeUltrasonic), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypeUltrasonic, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);

    // Test case: PhysicalContact
    OccupancySensingCluster clusterPhysicalContact{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact)};
    EXPECT_EQ(clusterPhysicalContact.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypePhysicalContact;
    EXPECT_EQ(ReadAttribute(clusterPhysicalContact, Attributes::OccupancySensorType::Id, occupancySensorTypePhysicalContact), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypePhysicalContact, OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact);

    // Test case: PIR + Ultrasonic
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> pirAndUltrasonicBitmap;
    pirAndUltrasonicBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    pirAndUltrasonicBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic);
    OccupancySensingCluster clusterPIRAndUltrasonic{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(pirAndUltrasonicBitmap)};
    EXPECT_EQ(clusterPIRAndUltrasonic.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypePIRAndUltrasonic;
    EXPECT_EQ(ReadAttribute(clusterPIRAndUltrasonic, Attributes::OccupancySensorType::Id, occupancySensorTypePIRAndUltrasonic), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypePIRAndUltrasonic, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);

    // Test case: PhysicalContact + PIR
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> physicalContactAndPirBitmap;
    physicalContactAndPirBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact);
    physicalContactAndPirBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    OccupancySensingCluster clusterPhysicalContactAndPir{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(physicalContactAndPirBitmap)};
    EXPECT_EQ(clusterPhysicalContactAndPir.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypePhysicalContactAndPir;
    EXPECT_EQ(ReadAttribute(clusterPhysicalContactAndPir, Attributes::OccupancySensorType::Id, occupancySensorTypePhysicalContactAndPir), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypePhysicalContactAndPir, OccupancySensing::OccupancySensorTypeEnum::kPir);

    // Test case: PhysicalContact + Ultrasonic
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> physicalContactAndUltrasonicBitmap;
    physicalContactAndUltrasonicBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact);
    physicalContactAndUltrasonicBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic);
    OccupancySensingCluster clusterPhysicalContactAndUltrasonic{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(physicalContactAndUltrasonicBitmap)};
    EXPECT_EQ(clusterPhysicalContactAndUltrasonic.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypePhysicalContactAndUltrasonic;
    EXPECT_EQ(ReadAttribute(clusterPhysicalContactAndUltrasonic, Attributes::OccupancySensorType::Id, occupancySensorTypePhysicalContactAndUltrasonic), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypePhysicalContactAndUltrasonic, OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);

    // Test case: PhysicalContact + PIR + Ultrasonic
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> allBitmap;
    allBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact);
    allBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    allBitmap.Set(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic);
    OccupancySensingCluster clusterAll{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(allBitmap)};
    EXPECT_EQ(clusterAll.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeAll;
    EXPECT_EQ(ReadAttribute(clusterAll, Attributes::OccupancySensorType::Id, occupancySensorTypeAll), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypeAll, OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic);

    // Test case: No bits set (000) -> PIR
    OccupancySensingCluster clusterNone{OccupancySensingCluster::Config{kTestEndpointId}.WithOccupancySensorTypeBitmap(0)};
    EXPECT_EQ(clusterNone.Startup(context.Get()), CHIP_NO_ERROR);
    OccupancySensing::OccupancySensorTypeEnum occupancySensorTypeNone;
    EXPECT_EQ(ReadAttribute(clusterNone, Attributes::OccupancySensorType::Id, occupancySensorTypeNone), CHIP_NO_ERROR);
    EXPECT_EQ(occupancySensorTypeNone, OccupancySensing::OccupancySensorTypeEnum::kPir);
}

TEST_F(TestOccupancySensingCluster, TestReadUnoccupiedToOccupiedAttributes)
{
    chip::Test::TestServerClusterContext context;
    constexpr uint16_t kPirUnoccupiedToOccupiedDelay = 10;
    constexpr uint8_t kPirUnoccupiedToOccupiedThreshold = 20;
    constexpr uint16_t kUltrasonicUnoccupiedToOccupiedDelay = 30;
    constexpr uint8_t kUltrasonicUnoccupiedToOccupiedThreshold = 40;
    constexpr uint16_t kPhysicalContactUnoccupiedToOccupiedDelay = 50;
    constexpr uint8_t kPhysicalContactUnoccupiedToOccupiedThreshold = 60;

    OccupancySensingCluster cluster{
        OccupancySensingCluster::Config(kTestEndpointId)
            .WithPIRUnoccupiedToOccupiedDelay(kPirUnoccupiedToOccupiedDelay)
            .WithPIRUnoccupiedToOccupiedThreshold(kPirUnoccupiedToOccupiedThreshold)
            .WithUltrasonicUnoccupiedToOccupiedDelay(kUltrasonicUnoccupiedToOccupiedDelay)
            .WithUltrasonicUnoccupiedToOccupiedThreshold(kUltrasonicUnoccupiedToOccupiedThreshold)
            .WithPhysicalContactUnoccupiedToOccupiedDelay(kPhysicalContactUnoccupiedToOccupiedDelay)
            .WithPhysicalContactUnoccupiedToOccupiedThreshold(kPhysicalContactUnoccupiedToOccupiedThreshold)};
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    uint16_t pirUnoccupiedToOccupiedDelay;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PIRUnoccupiedToOccupiedDelay::Id, pirUnoccupiedToOccupiedDelay), CHIP_NO_ERROR);
    EXPECT_EQ(pirUnoccupiedToOccupiedDelay, kPirUnoccupiedToOccupiedDelay);

    uint8_t pirUnoccupiedToOccupiedThreshold;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PIRUnoccupiedToOccupiedThreshold::Id, pirUnoccupiedToOccupiedThreshold), CHIP_NO_ERROR);
    EXPECT_EQ(pirUnoccupiedToOccupiedThreshold, kPirUnoccupiedToOccupiedThreshold);

    uint16_t ultrasonicUnoccupiedToOccupiedDelay;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id, ultrasonicUnoccupiedToOccupiedDelay), CHIP_NO_ERROR);
    EXPECT_EQ(ultrasonicUnoccupiedToOccupiedDelay, kUltrasonicUnoccupiedToOccupiedDelay);

    uint8_t ultrasonicUnoccupiedToOccupiedThreshold;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id, ultrasonicUnoccupiedToOccupiedThreshold), CHIP_NO_ERROR);
    EXPECT_EQ(ultrasonicUnoccupiedToOccupiedThreshold, kUltrasonicUnoccupiedToOccupiedThreshold);

    uint16_t physicalContactUnoccupiedToOccupiedDelay;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id, physicalContactUnoccupiedToOccupiedDelay), CHIP_NO_ERROR);
    EXPECT_EQ(physicalContactUnoccupiedToOccupiedDelay, kPhysicalContactUnoccupiedToOccupiedDelay);

    uint8_t physicalContactUnoccupiedToOccupiedThreshold;
    EXPECT_EQ(ReadNumericAttribute(cluster, Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id, physicalContactUnoccupiedToOccupiedThreshold), CHIP_NO_ERROR);
    EXPECT_EQ(physicalContactUnoccupiedToOccupiedThreshold, kPhysicalContactUnoccupiedToOccupiedThreshold);
}

} // namespace