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

#include <app/clusters/switch-server/SwitchCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Switch/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::app::Clusters::Switch::Attributes;
using namespace chip::Testing;

struct TestSwitchCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestSwitchCluster() {}

    TestServerClusterContext testContext;
};

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    uint8_t numberOfPositions{};
    ASSERT_EQ(tester.ReadAttribute(NumberOfPositions::Id, numberOfPositions), CHIP_NO_ERROR);

    uint8_t currentPosition{};
    ASSERT_EQ(tester.ReadAttribute(CurrentPosition::Id, currentPosition), CHIP_NO_ERROR);
}

} // namespace

TEST_F(TestSwitchCluster, AttributeTest)
{
    {
        const BitFlags<Feature> features{};
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, Switch::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMomentarySwitchMultiPress };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, Switch::Id), attributes), CHIP_NO_ERROR);

        const DataModel::AttributeEntry optionalAttributes[] = { MultiPressMax::kMetadataEntry };
        app::OptionalAttributeSet<MultiPressMax::Id> optionalAttributeSet;
        optionalAttributeSet.Set<MultiPressMax::Id>();

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestSwitchCluster, ReadAttributeTest)
{
    {
        const BitFlags<Feature> features{};
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMomentarySwitchMultiPress };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        uint8_t multiPressMax{};
        ASSERT_EQ(tester.ReadAttribute(MultiPressMax::Id, multiPressMax), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestSwitchCluster, CurrentPosition)
{
    {
        const BitFlags<Feature> features{};
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 4,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 0;
        EXPECT_EQ(cluster.SetCurrentPosition(currentPosition), CHIP_NO_ERROR);
        uint8_t currentPos = cluster.GetCurrentPosition();
        EXPECT_EQ(currentPos, currentPosition);

        currentPosition = 1;
        EXPECT_EQ(cluster.SetCurrentPosition(currentPosition), CHIP_NO_ERROR);
        currentPos = cluster.GetCurrentPosition();
        EXPECT_EQ(currentPos, currentPosition);

        currentPosition = 2;
        EXPECT_EQ(cluster.SetCurrentPosition(currentPosition), CHIP_NO_ERROR);
        currentPos = cluster.GetCurrentPosition();
        EXPECT_EQ(currentPos, currentPosition);

        currentPosition = 3;
        EXPECT_EQ(cluster.SetCurrentPosition(currentPosition), CHIP_NO_ERROR);
        currentPos = cluster.GetCurrentPosition();
        EXPECT_EQ(currentPos, currentPosition);

        currentPosition = 4;
        EXPECT_EQ(cluster.SetCurrentPosition(currentPosition), CHIP_ERROR_INVALID_ARGUMENT);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestSwitchCluster, Events)
{
    {
        const BitFlags<Feature> features{ Feature::kLatchingSwitch };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 0;
        EXPECT_NE(cluster.OnSwitchLatch(currentPosition), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMomentarySwitch, Feature::kMomentarySwitchRelease,
                                          Feature::kMomentarySwitchLongPress, Feature::kMomentarySwitchMultiPress };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 0;
        EXPECT_EQ(cluster.OnSwitchLatch(currentPosition), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMomentarySwitch, Feature::kMomentarySwitchRelease,
                                          Feature::kMomentarySwitchLongPress, Feature::kMomentarySwitchMultiPress };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 4,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 0;
        EXPECT_NE(cluster.OnInitialPress(currentPosition), std::nullopt);

        currentPosition = 1;
        EXPECT_NE(cluster.OnLongPress(currentPosition), std::nullopt);

        EXPECT_NE(cluster.OnShortRelease(currentPosition), std::nullopt);

        EXPECT_NE(cluster.OnLongRelease(currentPosition), std::nullopt);

        currentPosition = 2;
        EXPECT_NE(cluster.OnMultiPressOngoing(currentPosition, 1), std::nullopt);

        currentPosition = 3;
        EXPECT_NE(cluster.OnMultiPressComplete(currentPosition, 1), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kLatchingSwitch };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 4,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 0;
        EXPECT_EQ(cluster.OnInitialPress(currentPosition), std::nullopt);

        currentPosition = 1;
        EXPECT_EQ(cluster.OnLongPress(currentPosition), std::nullopt);

        EXPECT_EQ(cluster.OnShortRelease(currentPosition), std::nullopt);

        EXPECT_EQ(cluster.OnLongRelease(currentPosition), std::nullopt);

        currentPosition = 2;
        EXPECT_EQ(cluster.OnMultiPressOngoing(currentPosition, 1), std::nullopt);

        currentPosition = 3;
        EXPECT_EQ(cluster.OnMultiPressComplete(currentPosition, 1), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kLatchingSwitch };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 2;
        EXPECT_EQ(cluster.OnSwitchLatch(currentPosition), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kMomentarySwitch, Feature::kMomentarySwitchRelease,
                                          Feature::kMomentarySwitchLongPress, Feature::kMomentarySwitchMultiPress };
        SwitchCluster cluster(kRootEndpointId, features,
                              SwitchCluster::StartupConfiguration{
                                  .numberOfPositions = 4,
                                  .multiPressMax     = 2,
                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        uint8_t currentPosition = 4;
        EXPECT_EQ(cluster.OnInitialPress(currentPosition), std::nullopt);

        currentPosition = 5;
        EXPECT_EQ(cluster.OnLongPress(currentPosition), std::nullopt);

        EXPECT_EQ(cluster.OnShortRelease(currentPosition), std::nullopt);

        EXPECT_EQ(cluster.OnLongRelease(currentPosition), std::nullopt);

        currentPosition = 6;
        EXPECT_EQ(cluster.OnMultiPressOngoing(currentPosition, 1), std::nullopt);

        currentPosition = 7;
        EXPECT_EQ(cluster.OnMultiPressComplete(currentPosition, 1), std::nullopt);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
