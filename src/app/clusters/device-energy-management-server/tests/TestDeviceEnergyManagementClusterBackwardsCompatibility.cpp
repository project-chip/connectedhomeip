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

#include <app/clusters/device-energy-management-server/CodegenIntegration.h>
#include <app/clusters/device-energy-management-server/tests/DeviceEnergyManagementMockDelegate.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::Test;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestDeviceEnergyManagementClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}

    chip::Test::TestServerClusterContext mContext;
};

TEST_F(TestDeviceEnergyManagementClusterBackwardsCompatibility, TestInstanceLifecycle)
{
    // Test 1: Create Instance with all features
    {
        DeviceEnergyManagementMockDelegate mockDelegate;
        BitMask<Feature> allFeatures(Feature::kPowerAdjustment, Feature::kPowerForecastReporting, Feature::kStateForecastReporting,
                                     Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                     Feature::kConstraintBasedAdjustment);

        Instance instance(kTestEndpointId, mockDelegate, allFeatures);

        // Test initialization (registration)
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify cluster is registered in the registry
        auto * registeredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, DeviceEnergyManagement::Id));
        ASSERT_NE(registeredCluster, nullptr);

        // Test feature checking
        EXPECT_TRUE(instance.HasFeature(Feature::kPowerAdjustment));
        EXPECT_TRUE(instance.HasFeature(Feature::kPowerForecastReporting));
        EXPECT_TRUE(instance.HasFeature(Feature::kStateForecastReporting));
        EXPECT_TRUE(instance.HasFeature(Feature::kStartTimeAdjustment));
        EXPECT_TRUE(instance.HasFeature(Feature::kPausable));
        EXPECT_TRUE(instance.HasFeature(Feature::kForecastAdjustment));
        EXPECT_TRUE(instance.HasFeature(Feature::kConstraintBasedAdjustment));

        // Test shutdown (unregistration)
        instance.Shutdown();

        // Verify cluster is unregistered from the registry
        auto * unregisteredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, DeviceEnergyManagement::Id));
        EXPECT_EQ(unregisteredCluster, nullptr);
    }

    // Test Instance with no features enabled
    {
        DeviceEnergyManagementMockDelegate mockDelegate;
        BitMask<Feature> noFeatures;
        Instance instance(kTestEndpointId, mockDelegate, noFeatures);

        // Test initialization
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify no features are enabled
        EXPECT_FALSE(instance.HasFeature(Feature::kPowerAdjustment));
        EXPECT_FALSE(instance.HasFeature(Feature::kStartTimeAdjustment));
        EXPECT_FALSE(instance.HasFeature(Feature::kPausable));

        // Test shutdown
        instance.Shutdown();
    }
}

} // namespace
