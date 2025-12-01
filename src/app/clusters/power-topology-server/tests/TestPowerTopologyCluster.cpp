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

#include "MockPowerTopologyDelegate.h"
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
using namespace chip::Test;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate dummyDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = dummyDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Read FeatureMap attribute using ClusterTester
    ClusterTester tester(cluster);
    FeatureMap::TypeInfo::DecodableType featureMapValue;
    auto status = tester.ReadAttribute(FeatureMap::Id, featureMapValue);
    EXPECT_TRUE(status.IsSuccess());
    EXPECT_EQ(featureMapValue, features.Raw());

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsEmptyTest)
{
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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

TEST_F(TestPowerTopologyCluster, ReadAvailableEndpointsWithoutFeatureTest)
{
    chip::Test::TestServerClusterContext context;
    BitMask<Feature> features; // No SetTopology feature
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetAvailableEndpoints({ 1, 2, 3 });

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
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Failure);

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, ReadActiveEndpointsWithoutFeatureTest)
{
    chip::Test::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology); // No DynamicPowerFlow feature
    MockPowerTopologyDelegate mockDelegate;
    mockDelegate.SetActiveEndpoints({ 1, 2, 3 });

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
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Failure);

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, DelegateAvailableEndpointsErrorTest)
{
    chip::Test::TestServerClusterContext context;
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
    chip::Test::TestServerClusterContext context;
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

TEST_F(TestPowerTopologyCluster, ReadUnsupportedAttributeTest)
{
    chip::Test::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology);
    MockPowerTopologyDelegate mockDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Try to read an attribute that doesn't exist
    ClusterTester tester(cluster);
    constexpr AttributeId kInvalidAttributeId = 0xFFFF;
    uint32_t dummyValue;
    auto status = tester.ReadAttribute(kInvalidAttributeId, dummyValue);
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedAttribute);

    cluster.Shutdown();
}

TEST_F(TestPowerTopologyCluster, EndpointListBoundariesTest)
{
    chip::Test::TestServerClusterContext context;
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    MockPowerTopologyDelegate mockDelegate;

    PowerTopologyCluster cluster(PowerTopologyCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = mockDelegate,
        .features   = features,

    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Test empty list
    {
        mockDelegate.SetAvailableEndpoints({});
        AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
        auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
        EXPECT_TRUE(status.IsSuccess());

        auto iter = availableEndpoints.begin();
        EXPECT_FALSE(iter.Next());
    }

    // Test maximum list size (20 endpoints per spec)
    {
        std::vector<EndpointId> maxEndpoints;
        for (size_t i = 0; i < 20; i++)
        {
            maxEndpoints.push_back(static_cast<EndpointId>(i + 100));
        }
        mockDelegate.SetAvailableEndpoints(maxEndpoints);

        AvailableEndpoints::TypeInfo::DecodableType availableEndpoints;
        auto status = tester.ReadAttribute(AvailableEndpoints::Id, availableEndpoints);
        EXPECT_TRUE(status.IsSuccess());

        auto iter    = availableEndpoints.begin();
        size_t count = 0;
        while (iter.Next())
        {
            EXPECT_EQ(iter.GetValue(), static_cast<EndpointId>(100 + count));
            count++;
        }
        EXPECT_EQ(count, 20u);
    }

    // Test single endpoint
    {
        mockDelegate.SetActiveEndpoints({ 42 });
        ActiveEndpoints::TypeInfo::DecodableType activeEndpoints;
        auto status = tester.ReadAttribute(ActiveEndpoints::Id, activeEndpoints);
        EXPECT_TRUE(status.IsSuccess());

        auto iter = activeEndpoints.begin();
        ASSERT_TRUE(iter.Next());
        EXPECT_EQ(iter.GetValue(), 42u);
        EXPECT_FALSE(iter.Next());
    }

    cluster.Shutdown();
}

} // namespace
