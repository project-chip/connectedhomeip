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

#include "MockPowerTopologyDelegate.h"
#include <app/clusters/power-topology-server/CodegenIntegration.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;
using namespace chip::Test;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestPowerTopologyClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
    void TearDown() override {}

    TestServerClusterContext mContext;
};

TEST_F(TestPowerTopologyClusterBackwardsCompatibility, TestInstanceLifecycle)
{
    MockPowerTopologyDelegate mockDelegate;
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeAvailableEndpoints,
                                              OptionalAttributes::kOptionalAttributeActiveEndpoints);

    Instance instance(kTestEndpointId, mockDelegate, features, optionalAttrs);

    // Test initialization
    EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

    // Test shutdown
    instance.Shutdown();

    // Test re-initialization after shutdown
    EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);
    instance.Shutdown();
}

TEST_F(TestPowerTopologyClusterBackwardsCompatibility, TestHasFeatureAPI)
{
    MockPowerTopologyDelegate mockDelegate;

    // Test HasFeature with multiple features
    {
        BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
        BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeAvailableEndpoints,
                                                  OptionalAttributes::kOptionalAttributeActiveEndpoints);
        Instance instance(kTestEndpointId, mockDelegate, features, optionalAttrs);
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        EXPECT_TRUE(instance.HasFeature(Feature::kSetTopology));
        EXPECT_TRUE(instance.HasFeature(Feature::kDynamicPowerFlow));
        EXPECT_FALSE(instance.HasFeature(Feature::kNodeTopology));
        EXPECT_FALSE(instance.HasFeature(Feature::kTreeTopology));

        instance.Shutdown();
    }

    // Test HasFeature with single feature
    {
        BitMask<Feature> features(Feature::kNodeTopology);
        BitMask<OptionalAttributes> noOptionalAttrs;
        Instance instance(kTestEndpointId, mockDelegate, features, noOptionalAttrs);
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        EXPECT_TRUE(instance.HasFeature(Feature::kNodeTopology));
        EXPECT_FALSE(instance.HasFeature(Feature::kSetTopology));
        EXPECT_FALSE(instance.HasFeature(Feature::kDynamicPowerFlow));
        EXPECT_FALSE(instance.HasFeature(Feature::kTreeTopology));

        instance.Shutdown();
    }
}

TEST_F(TestPowerTopologyClusterBackwardsCompatibility, TestSupportsOptAttrAPI)
{
    MockPowerTopologyDelegate mockDelegate;

    // Test with all optional attributes
    {
        BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
        BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeAvailableEndpoints,
                                                  OptionalAttributes::kOptionalAttributeActiveEndpoints);
        Instance instance(kTestEndpointId, mockDelegate, features, optionalAttrs);
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        EXPECT_TRUE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeAvailableEndpoints));
        EXPECT_TRUE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveEndpoints));

        instance.Shutdown();
    }

    // Test with no optional attributes
    {
        BitMask<Feature> features(Feature::kNodeTopology);
        BitMask<OptionalAttributes> noOptionalAttrs;
        Instance instance(kTestEndpointId, mockDelegate, features, noOptionalAttrs);
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        EXPECT_FALSE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeAvailableEndpoints));
        EXPECT_FALSE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveEndpoints));

        instance.Shutdown();
    }

    // Test with only AvailableEndpoints
    {
        BitMask<Feature> features(Feature::kSetTopology);
        BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeAvailableEndpoints);
        Instance instance(kTestEndpointId, mockDelegate, features, optionalAttrs);
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        EXPECT_TRUE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeAvailableEndpoints));
        EXPECT_FALSE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveEndpoints));

        instance.Shutdown();
    }
}

TEST_F(TestPowerTopologyClusterBackwardsCompatibility, TestMultipleInstancesOnDifferentEndpoints)
{
    MockPowerTopologyDelegate mockDelegate1;
    MockPowerTopologyDelegate mockDelegate2;

    BitMask<Feature> features1(Feature::kSetTopology);
    BitMask<OptionalAttributes> optionalAttrs1(OptionalAttributes::kOptionalAttributeAvailableEndpoints);
    Instance instance1(kTestEndpointId, mockDelegate1, features1, optionalAttrs1);

    BitMask<Feature> features2(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    BitMask<OptionalAttributes> optionalAttrs2(OptionalAttributes::kOptionalAttributeAvailableEndpoints,
                                               OptionalAttributes::kOptionalAttributeActiveEndpoints);
    Instance instance2(kTestEndpointId + 1, mockDelegate2, features2, optionalAttrs2);

    // Initialize both instances
    EXPECT_EQ(instance1.Init(), CHIP_NO_ERROR);
    EXPECT_EQ(instance2.Init(), CHIP_NO_ERROR);

    // Verify they maintain independent state
    EXPECT_TRUE(instance1.HasFeature(Feature::kSetTopology));
    EXPECT_FALSE(instance1.HasFeature(Feature::kDynamicPowerFlow));

    EXPECT_TRUE(instance2.HasFeature(Feature::kSetTopology));
    EXPECT_TRUE(instance2.HasFeature(Feature::kDynamicPowerFlow));

    // Shutdown in reverse order
    instance2.Shutdown();
    instance1.Shutdown();
}

TEST_F(TestPowerTopologyClusterBackwardsCompatibility, TestInstanceConstructorAPI)
{
    MockPowerTopologyDelegate mockDelegate;

    // Test that Instance constructor accepts the expected parameters
    BitMask<Feature> features(Feature::kSetTopology, Feature::kDynamicPowerFlow);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeAvailableEndpoints,
                                              OptionalAttributes::kOptionalAttributeActiveEndpoints);

    // This test verifies the backwards compatibility of the Instance constructor signature
    Instance instance(kTestEndpointId, mockDelegate, features, optionalAttrs);

    EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);
    EXPECT_TRUE(instance.HasFeature(Feature::kSetTopology));
    EXPECT_TRUE(instance.HasFeature(Feature::kDynamicPowerFlow));
    EXPECT_TRUE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeAvailableEndpoints));
    EXPECT_TRUE(instance.SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveEndpoints));
    instance.Shutdown();
}

} // namespace
