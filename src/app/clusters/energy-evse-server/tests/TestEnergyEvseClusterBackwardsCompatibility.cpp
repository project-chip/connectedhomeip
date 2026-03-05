/*
 *
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

#include <app/clusters/energy-evse-server/CodegenIntegration.h>
#include <app/clusters/energy-evse-server/tests/MockEvseDelegate.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::Testing;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestEvseClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestEvseClusterBackwardsCompatibility, TestInstanceLifecycle)
{
    // Test 1: Create Instance with all features
    {
        MockEvseDelegate mockDelegate;
        BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kSoCReporting, Feature::kPlugAndCharge, Feature::kRfid,
                                     Feature::kV2x);
        BitMask<OptionalAttributes> optionalAttrs;
        BitMask<OptionalCommands> optionalCmds;

        Instance instance(kTestEndpointId, mockDelegate, allFeatures, optionalAttrs, optionalCmds);

        // Test initialization (registration)
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify cluster is registered in the registry
        auto * registeredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, EnergyEvse::Id));
        ASSERT_NE(registeredCluster, nullptr);

        // Test feature checking
        EXPECT_TRUE(instance.HasFeature(Feature::kChargingPreferences));
        EXPECT_TRUE(instance.HasFeature(Feature::kSoCReporting));
        EXPECT_TRUE(instance.HasFeature(Feature::kPlugAndCharge));
        EXPECT_TRUE(instance.HasFeature(Feature::kRfid));
        EXPECT_TRUE(instance.HasFeature(Feature::kV2x));

        // Test shutdown (unregistration)
        instance.Shutdown();

        // Verify cluster is unregistered from the registry
        auto * unregisteredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, EnergyEvse::Id));
        EXPECT_EQ(unregisteredCluster, nullptr);
    }

    // Test 2: Instance with no features enabled
    {
        MockEvseDelegate mockDelegate;
        BitMask<Feature> noFeatures;
        BitMask<OptionalAttributes> optionalAttrs;
        BitMask<OptionalCommands> optionalCmds;

        Instance instance(kTestEndpointId, mockDelegate, noFeatures, optionalAttrs, optionalCmds);

        // Test initialization
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify no features are enabled
        EXPECT_FALSE(instance.HasFeature(Feature::kChargingPreferences));
        EXPECT_FALSE(instance.HasFeature(Feature::kSoCReporting));
        EXPECT_FALSE(instance.HasFeature(Feature::kPlugAndCharge));
        EXPECT_FALSE(instance.HasFeature(Feature::kRfid));
        EXPECT_FALSE(instance.HasFeature(Feature::kV2x));

        // Test shutdown
        instance.Shutdown();
    }
}

} // namespace
