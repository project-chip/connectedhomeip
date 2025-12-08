/*
 *
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

#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/power-topology-server/tests/MockPowerTopologyDelegate.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/PowerTopology/Attributes.h>
#include <clusters/PowerTopology/Events.h>
#include <clusters/PowerTopology/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestPowerTopologyCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

TEST_F(TestPowerTopologyCluster, EmptyAttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> noFeatures;
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = noFeatures,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, {}));
    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, AllAttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> allFeatures(Feature::kNodeTopology, Feature::kTreeTopology, Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = allFeatures,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::AvailableEndpoints::kMetadataEntry,
                                            Attributes::ActiveEndpoints::kMetadataEntry,
                                        }));
    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, NoActiveEndpointsWithoutDYPFTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::AvailableEndpoints::kMetadataEntry,
                                        }));
    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, OnlyDynamicPowerFlowFeatureTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, FeatureMapReadTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    FeatureMap::TypeInfo::DecodableType featureMapValue{};
    ASSERT_TRUE(tester.ReadAttribute(FeatureMap::Id, featureMapValue).IsSuccess());
    EXPECT_EQ(featureMapValue, features.Raw());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsEmptyTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.ClearAvailableEndpoints(); // Empty list

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsSingleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpoints({ 5 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 5u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsMultipleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpoints({ 1, 2, 5, 10 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = availableEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 1u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 2u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 5u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 10u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadActiveEndpointsEmptyTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.ClearActiveEndpoints();

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = activeEndpoints.begin();
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadActiveEndpointsMultipleTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetActiveEndpoints({ 3, 7, 9 });

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = activeEndpoints.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 3u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 7u);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), 9u);
    EXPECT_FALSE(iter.Next());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, DelegateAvailableEndpointsErrorTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpointsError(CHIP_ERROR_INTERNAL);

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
    auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
    EXPECT_FALSE(status.IsSuccess());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, DelegateActiveEndpointsErrorTest)
{
    chip::Testing::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kDynamicPowerFlow, Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetActiveEndpointsError(CHIP_ERROR_INTERNAL);

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
    auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
    EXPECT_FALSE(status.IsSuccess());

    cluster.Shutdown();
}

} // namespace
