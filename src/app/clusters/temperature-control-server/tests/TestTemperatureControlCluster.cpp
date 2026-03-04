/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/temperature-control-server/TemperatureControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;
using namespace chip::Testing;

struct TestTemperatureControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTemperatureControlCluster() {}

    TestServerClusterContext testContext;
};

constexpr CharSpan kTemperatureLevelOptions[] = { "Low"_span, "Medium"_span, "High"_span };

class AppSupportedTemperatureLevelsDelegate : public SupportedTemperatureLevelsIteratorDelegate
{
public:
    uint8_t Size() override { return 3; }

    CHIP_ERROR Next(MutableCharSpan & item) override
    {
        return CopyCharSpanToMutableCharSpan(kTemperatureLevelOptions[mIndex++], item);
    }

    ~AppSupportedTemperatureLevelsDelegate() {}
};

AppSupportedTemperatureLevelsDelegate gDelegate;

} // namespace

TEST_F(TestTemperatureControlCluster, AttributeTest)
{
    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber };
        TemperatureControlCluster cluster(
            kRootEndpointId, features,
            TemperatureControlCluster::StartupConfiguration{ .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3 });

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::TemperatureSetpoint::kMetadataEntry,
                                                Attributes::MinTemperature::kMetadataEntry,
                                                Attributes::MaxTemperature::kMetadataEntry,
                                            }));
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber, Feature::kTemperatureStep };
        TemperatureControlCluster cluster(kRootEndpointId, features,
                                          TemperatureControlCluster::StartupConfiguration{
                                              .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3, .step = 1 });

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::TemperatureSetpoint::kMetadataEntry,
                                                Attributes::MinTemperature::kMetadataEntry,
                                                Attributes::MaxTemperature::kMetadataEntry,
                                                Attributes::Step::kMetadataEntry,
                                            }));
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureLevel };
        TemperatureControlCluster cluster(kRootEndpointId, features, TemperatureControlCluster::StartupConfiguration{});

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::SelectedTemperatureLevel::kMetadataEntry,
                                                Attributes::SupportedTemperatureLevels::kMetadataEntry,
                                            }));
    }
}

TEST_F(TestTemperatureControlCluster, ReadAttributeTest)
{
    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber };
        TemperatureControlCluster cluster(
            kRootEndpointId, features,
            TemperatureControlCluster::StartupConfiguration{ .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3 });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        int16_t temperatureSetpoint{};
        ASSERT_EQ(tester.ReadAttribute(TemperatureSetpoint::Id, temperatureSetpoint), CHIP_NO_ERROR);

        int16_t minTemperature{};
        ASSERT_EQ(tester.ReadAttribute(MinTemperature::Id, minTemperature), CHIP_NO_ERROR);

        int16_t maxTemperature{};
        ASSERT_EQ(tester.ReadAttribute(MaxTemperature::Id, maxTemperature), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber, Feature::kTemperatureStep };
        TemperatureControlCluster cluster(kRootEndpointId, features,
                                          TemperatureControlCluster::StartupConfiguration{
                                              .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3, .step = 1 });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        int16_t temperatureSetpoint{};
        ASSERT_EQ(tester.ReadAttribute(TemperatureSetpoint::Id, temperatureSetpoint), CHIP_NO_ERROR);

        int16_t minTemperature{};
        ASSERT_EQ(tester.ReadAttribute(MinTemperature::Id, minTemperature), CHIP_NO_ERROR);

        int16_t maxTemperature{};
        ASSERT_EQ(tester.ReadAttribute(MaxTemperature::Id, maxTemperature), CHIP_NO_ERROR);

        int16_t step{};
        ASSERT_EQ(tester.ReadAttribute(Step::Id, step), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureLevel };
        TemperatureControlCluster cluster(kRootEndpointId, features, TemperatureControlCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        TemperatureControlCluster::SetDelegate(&gDelegate);

        ClusterTester tester(cluster);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        uint8_t selectedTemperatureLevel{};
        ASSERT_EQ(tester.ReadAttribute(SelectedTemperatureLevel::Id, selectedTemperatureLevel), CHIP_NO_ERROR);

        DataModel::DecodableList<chip::CharSpan> supportedTemperatureLevels;
        ASSERT_EQ(tester.ReadAttribute(SupportedTemperatureLevels::Id, supportedTemperatureLevels), CHIP_NO_ERROR);

        auto it = supportedTemperatureLevels.begin();
        ASSERT_TRUE(it.Next());
        auto temperatureLevel = it.GetValue();
        ASSERT_TRUE(temperatureLevel.data_equal("Low"_span));
        ASSERT_TRUE(it.Next());
        temperatureLevel = it.GetValue();
        ASSERT_TRUE(temperatureLevel.data_equal("Medium"_span));
        ASSERT_TRUE(it.Next());
        temperatureLevel = it.GetValue();
        ASSERT_TRUE(temperatureLevel.data_equal("High"_span));
        ASSERT_FALSE(it.Next());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTemperatureControlCluster, TemperatureSetpoint)
{
    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber };
        TemperatureControlCluster cluster(
            kRootEndpointId, features,
            TemperatureControlCluster::StartupConfiguration{ .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3 });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        int16_t temperatureSetpoint = 0;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        temperatureSetpoint = 1;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_NO_ERROR);
        int16_t temperatureVal = cluster.GetTemperatureSetpoint();
        EXPECT_EQ(temperatureVal, temperatureSetpoint);

        temperatureSetpoint = 2;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_NO_ERROR);
        temperatureVal = cluster.GetTemperatureSetpoint();
        EXPECT_EQ(temperatureVal, temperatureSetpoint);

        temperatureSetpoint = 3;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_NO_ERROR);
        temperatureVal = cluster.GetTemperatureSetpoint();
        EXPECT_EQ(temperatureVal, temperatureSetpoint);

        temperatureSetpoint = 4;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber, Feature::kTemperatureStep };
        TemperatureControlCluster cluster(kRootEndpointId, features,
                                          TemperatureControlCluster::StartupConfiguration{
                                              .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3, .step = 2 });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        int16_t temperatureSetpoint = 0;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        temperatureSetpoint = 1;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_NO_ERROR);
        int16_t temperatureVal = cluster.GetTemperatureSetpoint();
        EXPECT_EQ(temperatureVal, temperatureSetpoint);

        temperatureSetpoint = 2;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        temperatureSetpoint = 3;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_NO_ERROR);
        temperatureVal = cluster.GetTemperatureSetpoint();
        EXPECT_EQ(temperatureVal, temperatureSetpoint);

        temperatureSetpoint = 4;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureLevel };
        TemperatureControlCluster cluster(kRootEndpointId, features, TemperatureControlCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        int16_t temperatureSetpoint = 0;
        EXPECT_EQ(cluster.SetTemperatureSetpoint(temperatureSetpoint), CHIP_IM_GLOBAL_STATUS(InvalidInState));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTemperatureControlCluster, SelectedTemperatureLevel)
{
    {
        BitFlags<Feature> features{ Feature::kTemperatureLevel };
        TemperatureControlCluster cluster(kRootEndpointId, features, TemperatureControlCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        TemperatureControlCluster::SetDelegate(&gDelegate);

        uint8_t selectedTemperatureLevel = 0;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_NO_ERROR);
        uint8_t temperatureLevel = cluster.GetSelectedTemperatureLevel();
        EXPECT_EQ(temperatureLevel, selectedTemperatureLevel);

        selectedTemperatureLevel = 1;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_NO_ERROR);
        temperatureLevel = cluster.GetSelectedTemperatureLevel();
        EXPECT_EQ(temperatureLevel, selectedTemperatureLevel);

        selectedTemperatureLevel = 2;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_NO_ERROR);
        temperatureLevel = cluster.GetSelectedTemperatureLevel();
        EXPECT_EQ(temperatureLevel, selectedTemperatureLevel);

        selectedTemperatureLevel = 3;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureLevel };
        TemperatureControlCluster cluster(kRootEndpointId, features, TemperatureControlCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        TemperatureControlCluster::SetDelegate(nullptr);

        uint8_t selectedTemperatureLevel = 0;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_IM_GLOBAL_STATUS(NotFound));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        BitFlags<Feature> features{ Feature::kTemperatureNumber };
        TemperatureControlCluster cluster(
            kRootEndpointId, features,
            TemperatureControlCluster::StartupConfiguration{ .temperatureSetpoint = 1, .minTemperature = 1, .maxTemperature = 3 });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t selectedTemperatureLevel = 0;
        EXPECT_EQ(cluster.SetSelectedTemperatureLevel(selectedTemperatureLevel), CHIP_IM_GLOBAL_STATUS(InvalidInState));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
