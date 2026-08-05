/*
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

#include <pw_unit_test/framework.h>

#include <app/clusters/commodity-price-server/CommodityPriceCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/Commands.h>
#include <clusters/CommodityPrice/Metadata.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;
using namespace chip::app::Clusters::CommodityPrice::Attributes;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

constexpr EndpointId kTestEndpointId = 1;

struct TestCommodityPriceCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mCluster.Startup(mTester.GetServerClusterContext()), CHIP_NO_ERROR); }

    CommodityPriceCluster mCluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting) };
    ClusterTester mTester{ mCluster };
};

TEST_F(TestCommodityPriceCluster, ReadClusterRevision)
{
    ClusterRevision::TypeInfo::DecodableType clusterRevision = 0;
    ASSERT_EQ(mTester.ReadAttribute(ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestCommodityPriceCluster, ReadFeatureMap)
{
    FeatureMap::TypeInfo::DecodableType featureMap = 0;
    ASSERT_EQ(mTester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kForecasting));
}

TEST_F(TestCommodityPriceCluster, AttributeListWithForecasting)
{
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster,
                                        std::vector<DataModel::AttributeEntry>{
                                            TariffUnit::kMetadataEntry,
                                            Currency::kMetadataEntry,
                                            CurrentPrice::kMetadataEntry,
                                            PriceForecast::kMetadataEntry,
                                        }));
}

TEST_F(TestCommodityPriceCluster, AttributeListWithoutForecasting)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>() };

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        std::vector<DataModel::AttributeEntry>{
                                            TariffUnit::kMetadataEntry,
                                            Currency::kMetadataEntry,
                                            CurrentPrice::kMetadataEntry,
                                        }));
}

// TODO: the lists below are empty while the commands are stubs. Once implemented these must assert
// GetDetailedPriceRequest/Response plus the Forecasting (FORE) gated entries.
TEST_F(TestCommodityPriceCluster, AcceptedCommandsIsEmpty)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster, std::vector<DataModel::AcceptedCommandEntry>{}));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsIsEmpty)
{
    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(mCluster, std::vector<CommandId>{}));
}

// TODO: replace with an actual value check once ReadAttribute is implemented. TariffUnit is now in
// the attribute list, so this exercises the ReadAttribute stub's default branch.
TEST_F(TestCommodityPriceCluster, ReadUnimplementedAttribute)
{
    TariffUnit::TypeInfo::DecodableType tariffUnit;
    EXPECT_EQ(mTester.ReadAttribute(TariffUnit::Id, tariffUnit), Status::UnsupportedAttribute);
}

// TODO: replace with a response check once InvokeCommand is implemented.
TEST_F(TestCommodityPriceCluster, InvokeUnimplementedCommand)
{
    auto result = mTester.Invoke(Commands::GetDetailedPriceRequest::Type{});
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedCommand));
}

} // namespace
