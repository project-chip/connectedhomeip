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

#include <app/clusters/electrical-distribution-server/ElectricalDistributionCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ElectricalDistribution/Attributes.h>
#include <clusters/ElectricalDistribution/Enums.h>
#include <clusters/ElectricalDistribution/Metadata.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalDistribution;
using namespace chip::app::Clusters::ElectricalDistribution::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kEndpointWithElectricalDistribution = 1;

const ElectricalDistributionCluster::StartupConfiguration kDefaultConfig = {
    .maxContinuousCurrent = DataModel::MakeNullable<int64_t>(100000), // 100 A, in mA
    .maxVoltage           = DataModel::MakeNullable<int64_t>(240000), // 240 V, in mV
    .numberOfPoles        = DataModel::MakeNullable<uint16_t>(2),
    .endOfLife            = DataModel::MakeNullable(EndOfLifeEnum::kNone),
    .serviceEntranceRated = DataModel::MakeNullable(true),
};

class ElectricalDistributionClusterLocal : public ElectricalDistributionCluster
{
public:
    using ElectricalDistributionCluster::ElectricalDistributionCluster;

    MaxContinuousCurrent::TypeInfo::Type GetMaxContinuousCurrent() const { return mMaxContinuousCurrent; }
    MaxVoltage::TypeInfo::Type GetMaxVoltage() const { return mMaxVoltage; }
    NumberOfPoles::TypeInfo::Type GetNumberOfPoles() const { return mNumberOfPoles; }
    EndOfLife::TypeInfo::Type GetEndOfLife() const { return mEndOfLife; }
    ServiceEntranceRated::TypeInfo::Type GetServiceEntranceRated() const { return mServiceEntranceRated; }
};

struct TestElectricalDistributionCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(electricalDistribution.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { electricalDistribution.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestElectricalDistributionCluster() : electricalDistribution(kEndpointWithElectricalDistribution, kDefaultConfig) {}

    TestServerClusterContext testContext;
    ElectricalDistributionClusterLocal electricalDistribution;
};

} // namespace

TEST_F(TestElectricalDistributionCluster, AttributeTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(electricalDistribution,
                                        {
                                            ElectricalDistribution::Attributes::MaxContinuousCurrent::kMetadataEntry,
                                            ElectricalDistribution::Attributes::MaxVoltage::kMetadataEntry,
                                            ElectricalDistribution::Attributes::NumberOfPoles::kMetadataEntry,
                                            ElectricalDistribution::Attributes::EndOfLife::kMetadataEntry,
                                            ElectricalDistribution::Attributes::ServiceEntranceRated::kMetadataEntry,
                                        }));
}

TEST_F(TestElectricalDistributionCluster, ReadAttributeTest)
{
    ClusterTester tester(electricalDistribution);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    ASSERT_EQ(revision, ElectricalDistribution::kRevision);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
    ASSERT_EQ(features, 0u);

    // Assert the decoded values, not just that decoding succeeded: the configured values reach a
    // client only through the encode path, so a misrouted attribute would otherwise go unnoticed.
    MaxContinuousCurrent::TypeInfo::DecodableType maxContinuousCurrent;
    ASSERT_EQ(tester.ReadAttribute(MaxContinuousCurrent::Id, maxContinuousCurrent), CHIP_NO_ERROR);
    ASSERT_FALSE(maxContinuousCurrent.IsNull());
    EXPECT_EQ(maxContinuousCurrent.Value(), kDefaultConfig.maxContinuousCurrent.Value());

    MaxVoltage::TypeInfo::DecodableType maxVoltage;
    ASSERT_EQ(tester.ReadAttribute(MaxVoltage::Id, maxVoltage), CHIP_NO_ERROR);
    ASSERT_FALSE(maxVoltage.IsNull());
    EXPECT_EQ(maxVoltage.Value(), kDefaultConfig.maxVoltage.Value());

    NumberOfPoles::TypeInfo::DecodableType numberOfPoles;
    ASSERT_EQ(tester.ReadAttribute(NumberOfPoles::Id, numberOfPoles), CHIP_NO_ERROR);
    ASSERT_FALSE(numberOfPoles.IsNull());
    EXPECT_EQ(numberOfPoles.Value(), kDefaultConfig.numberOfPoles.Value());

    EndOfLife::TypeInfo::DecodableType endOfLife;
    ASSERT_EQ(tester.ReadAttribute(EndOfLife::Id, endOfLife), CHIP_NO_ERROR);
    ASSERT_FALSE(endOfLife.IsNull());
    EXPECT_EQ(endOfLife.Value(), kDefaultConfig.endOfLife.Value());

    ServiceEntranceRated::TypeInfo::DecodableType serviceEntranceRated;
    ASSERT_EQ(tester.ReadAttribute(ServiceEntranceRated::Id, serviceEntranceRated), CHIP_NO_ERROR);
    ASSERT_FALSE(serviceEntranceRated.IsNull());
    EXPECT_EQ(serviceEntranceRated.Value(), kDefaultConfig.serviceEntranceRated.Value());
}

TEST_F(TestElectricalDistributionCluster, ConfiguredValues)
{
    ASSERT_EQ(electricalDistribution.GetMaxContinuousCurrent(), kDefaultConfig.maxContinuousCurrent);
    ASSERT_EQ(electricalDistribution.GetMaxVoltage(), kDefaultConfig.maxVoltage);
    ASSERT_EQ(electricalDistribution.GetNumberOfPoles(), kDefaultConfig.numberOfPoles);
    ASSERT_EQ(electricalDistribution.GetEndOfLife(), kDefaultConfig.endOfLife);
    ASSERT_EQ(electricalDistribution.GetServiceEntranceRated(), kDefaultConfig.serviceEntranceRated);
}

TEST_F(TestElectricalDistributionCluster, SetEndOfLife)
{
    ASSERT_EQ(electricalDistribution.GetEndOfLife(), DataModel::MakeNullable(EndOfLifeEnum::kNone));

    EndOfLife::TypeInfo::Type endOfLife = DataModel::MakeNullable(EndOfLifeEnum::kDamaged);
    ASSERT_EQ(electricalDistribution.SetEndOfLife(endOfLife), CHIP_NO_ERROR);
    ASSERT_EQ(electricalDistribution.GetEndOfLife(), endOfLife);

    endOfLife = DataModel::MakeNullable(EndOfLifeEnum::kExpired);
    ASSERT_EQ(electricalDistribution.SetEndOfLife(endOfLife), CHIP_NO_ERROR);
    ASSERT_EQ(electricalDistribution.GetEndOfLife(), endOfLife);

    // Null is a legal value: the attribute is nullable.
    endOfLife = DataModel::NullNullable;
    ASSERT_EQ(electricalDistribution.SetEndOfLife(endOfLife), CHIP_NO_ERROR);
    ASSERT_TRUE(electricalDistribution.GetEndOfLife().IsNull());
}

TEST_F(TestElectricalDistributionCluster, SetEndOfLifeRejectsUnknownValue)
{
    EndOfLife::TypeInfo::Type known = DataModel::MakeNullable(EndOfLifeEnum::kDegraded);
    ASSERT_EQ(electricalDistribution.SetEndOfLife(known), CHIP_NO_ERROR);

    // kUnknownEnumValue is the decode sentinel and must never be stored, or a later read would
    // transmit an out-of-spec value.
    EndOfLife::TypeInfo::Type unknown = DataModel::MakeNullable(EndOfLifeEnum::kUnknownEnumValue);
    ASSERT_EQ(electricalDistribution.SetEndOfLife(unknown), CHIP_ERROR_INVALID_ARGUMENT);

    // Any value outside the defined range is rejected the same way.
    EndOfLife::TypeInfo::Type outOfRange = DataModel::MakeNullable(static_cast<EndOfLifeEnum>(0xFE));
    ASSERT_EQ(electricalDistribution.SetEndOfLife(outOfRange), CHIP_ERROR_INVALID_ARGUMENT);

    // A rejected write leaves the previous value untouched.
    ASSERT_EQ(electricalDistribution.GetEndOfLife(), known);
}
