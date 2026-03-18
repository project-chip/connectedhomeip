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

#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AirQuality/Attributes.h>
#include <clusters/AirQuality/Enums.h>
#include <clusters/AirQuality/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

struct TestAirQualityCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(airQuality.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { airQuality.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestAirQualityCluster() :
        airQuality(kRootEndpointId,
                   BitFlags<Feature>(Feature::kFair, Feature::kModerate, Feature::kVeryPoor, Feature::kExtremelyPoor))
    {}

    TestServerClusterContext testContext;
    AirQualityCluster airQuality;
};

struct TestAirQualityClusterNoFeatures : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(airQuality.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { airQuality.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestAirQualityClusterNoFeatures() : airQuality(kRootEndpointId, BitFlags<Feature>()) {}

    TestServerClusterContext testContext;
    AirQualityCluster airQuality;
};

} // namespace

TEST_F(TestAirQualityCluster, AttributeList)
{
    ASSERT_TRUE(IsAttributesListEqualTo(airQuality,
                                        {
                                            Clusters::AirQuality::Attributes::AirQuality::kMetadataEntry,
                                        }));
}

TEST_F(TestAirQualityCluster, ReadAttributes)
{
    ClusterTester tester(airQuality);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, AirQuality::kRevision);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features,
              to_underlying(Feature::kFair) | to_underlying(Feature::kModerate) | to_underlying(Feature::kVeryPoor) |
                  to_underlying(Feature::kExtremelyPoor));

    uint8_t airQualityVal{};
    ASSERT_EQ(tester.ReadAttribute(Clusters::AirQuality::Attributes::AirQuality::Id, airQualityVal), CHIP_NO_ERROR);
    EXPECT_EQ(airQualityVal, to_underlying(AirQualityEnum::kUnknown));
}

TEST_F(TestAirQualityCluster, DefaultValue)
{
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kUnknown);
}

TEST_F(TestAirQualityCluster, SetAndGetAirQuality)
{
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kGood), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kGood);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kFair), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kFair);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kModerate), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kModerate);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kPoor), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kPoor);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kVeryPoor), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kVeryPoor);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kExtremelyPoor), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kExtremelyPoor);

    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kUnknown), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kUnknown);
}

TEST_F(TestAirQualityCluster, SetSameValueNoChange)
{
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kGood), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kGood);

    // Setting the same value again should still succeed
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kGood), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kGood);
}

TEST_F(TestAirQualityCluster, InvalidValueRejected)
{
    EXPECT_EQ(airQuality.SetAirQuality(static_cast<AirQualityEnum>(0xFF)), Protocols::InteractionModel::Status::InvalidValue);
    // Value should remain unchanged
    EXPECT_EQ(airQuality.GetAirQuality(), AirQualityEnum::kUnknown);
}

TEST_F(TestAirQualityCluster, HasFeature)
{
    EXPECT_TRUE(airQuality.HasFeature(Feature::kFair));
    EXPECT_TRUE(airQuality.HasFeature(Feature::kModerate));
    EXPECT_TRUE(airQuality.HasFeature(Feature::kVeryPoor));
    EXPECT_TRUE(airQuality.HasFeature(Feature::kExtremelyPoor));
}

TEST_F(TestAirQualityClusterNoFeatures, FeatureConstrainedValuesRejected)
{
    // With no features enabled, Fair/Moderate/VeryPoor/ExtremelyPoor should be rejected
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kFair), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kModerate), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kVeryPoor), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kExtremelyPoor), Protocols::InteractionModel::Status::ConstraintError);

    // Unknown, Good, Poor should still work (no feature required)
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kUnknown), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kGood), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kPoor), Protocols::InteractionModel::Status::Success);
}

TEST_F(TestAirQualityClusterNoFeatures, HasFeatureReturnsFalse)
{
    EXPECT_FALSE(airQuality.HasFeature(Feature::kFair));
    EXPECT_FALSE(airQuality.HasFeature(Feature::kModerate));
    EXPECT_FALSE(airQuality.HasFeature(Feature::kVeryPoor));
    EXPECT_FALSE(airQuality.HasFeature(Feature::kExtremelyPoor));
}

TEST_F(TestAirQualityCluster, PartialFeaturesFairOnly)
{
    // Teardown the all-features instance and create one with only kFair
    airQuality.Shutdown(ClusterShutdownType::kClusterShutdown);

    AirQualityCluster fairOnly(kRootEndpointId, BitFlags<Feature>(Feature::kFair));
    ASSERT_EQ(fairOnly.Startup(testContext.Get()), CHIP_NO_ERROR);

    // Fair should be accepted
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kFair), Protocols::InteractionModel::Status::Success);

    // Other feature-gated values should be rejected
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kModerate), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kVeryPoor), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kExtremelyPoor), Protocols::InteractionModel::Status::ConstraintError);

    // Non-feature-gated values should still work
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kGood), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kPoor), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(fairOnly.SetAirQuality(AirQualityEnum::kUnknown), Protocols::InteractionModel::Status::Success);

    fairOnly.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAirQualityCluster, ReadAttributeAfterSet)
{
    ClusterTester tester(airQuality);

    // Set via API
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kPoor), Protocols::InteractionModel::Status::Success);

    // Read via ClusterTester to verify ReadAttribute returns the updated value
    uint8_t airQualityVal{};
    ASSERT_EQ(tester.ReadAttribute(Clusters::AirQuality::Attributes::AirQuality::Id, airQualityVal), CHIP_NO_ERROR);
    EXPECT_EQ(airQualityVal, to_underlying(AirQualityEnum::kPoor));

    // Change again and verify
    EXPECT_EQ(airQuality.SetAirQuality(AirQualityEnum::kExtremelyPoor), Protocols::InteractionModel::Status::Success);
    ASSERT_EQ(tester.ReadAttribute(Clusters::AirQuality::Attributes::AirQuality::Id, airQualityVal), CHIP_NO_ERROR);
    EXPECT_EQ(airQualityVal, to_underlying(AirQualityEnum::kExtremelyPoor));
}

TEST_F(TestAirQualityClusterNoFeatures, FeatureMapReadsZero)
{
    ClusterTester tester(airQuality);

    // No features set — FeatureMap should be 0
    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features, 0u);
}
