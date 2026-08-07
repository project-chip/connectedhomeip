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

constexpr BitMask<OptionalCommands> kAllOptionalCommands{ OptionalCommands::kSupportsGetDetailedPriceRequest,
                                                         OptionalCommands::kSupportsGetDetailedForecastRequest };

struct TestCommodityPriceCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mCluster.Startup(mTester.GetServerClusterContext()), CHIP_NO_ERROR); }

    CommodityPriceCluster mCluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), kAllOptionalCommands };
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
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        std::vector<DataModel::AttributeEntry>{
                                            TariffUnit::kMetadataEntry,
                                            Currency::kMetadataEntry,
                                            CurrentPrice::kMetadataEntry,
                                        }));
}

TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithAllOptionalCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedPriceRequest::kMetadataEntry,
                                                  Commands::GetDetailedForecastRequest::kMetadataEntry,
                                              }));
}

// Both commands are optionally conformant, so opting into neither is a valid configuration.
TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithNoOptionalCommands)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), BitMask<OptionalCommands>() };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, std::vector<DataModel::AcceptedCommandEntry>{}));
}

// GetDetailedForecastRequest requires FORE in addition to the opt-in, so it must be absent here even
// though the application asked for it.
TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithoutForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedPriceRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestCommodityPriceCluster, AcceptedCommandsWithForecastRequestOnly)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting),
                                   BitMask<OptionalCommands>(OptionalCommands::kSupportsGetDetailedForecastRequest) };

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              std::vector<DataModel::AcceptedCommandEntry>{
                                                  Commands::GetDetailedForecastRequest::kMetadataEntry,
                                              }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithAllOptionalCommands)
{
    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(mCluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedPriceResponse::Id,
                                                   Commands::GetDetailedForecastResponse::Id,
                                               }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithNoOptionalCommands)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting), BitMask<OptionalCommands>() };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster, std::vector<CommandId>{}));
}

// GetDetailedForecastResponse follows its request, which requires FORE on top of the opt-in.
TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithoutForecastingFeature)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(), kAllOptionalCommands };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedPriceResponse::Id,
                                               }));
}

TEST_F(TestCommodityPriceCluster, GeneratedCommandsWithForecastRequestOnly)
{
    CommodityPriceCluster cluster{ kTestEndpointId, BitMask<Feature>(Feature::kForecasting),
                                   BitMask<OptionalCommands>(OptionalCommands::kSupportsGetDetailedForecastRequest) };

    EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               std::vector<CommandId>{
                                                   Commands::GetDetailedForecastResponse::Id,
                                               }));
}

// TODO: replace with an actual value check once ReadAttribute is implemented. TariffUnit is now in
// the attribute list, so this exercises the ReadAttribute stub's default branch.
TEST_F(TestCommodityPriceCluster, ReadUnimplementedAttribute)
{
    TariffUnit::TypeInfo::DecodableType tariffUnit;
    EXPECT_EQ(mTester.ReadAttribute(TariffUnit::Id, tariffUnit), Status::UnsupportedAttribute);
}

// TODO: replace with a response check once InvokeCommand is implemented. GetDetailedPriceRequest is
// now in the accepted commands list, so this exercises the InvokeCommand stub.
TEST_F(TestCommodityPriceCluster, InvokeUnimplementedCommand)
{
    auto result = mTester.Invoke(Commands::GetDetailedPriceRequest::Type{});
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedCommand));
}

} // namespace
