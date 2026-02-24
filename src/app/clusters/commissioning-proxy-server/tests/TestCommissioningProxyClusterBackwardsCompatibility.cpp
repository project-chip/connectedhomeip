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

#include <app/clusters/commissioning-proxy-server/CodegenIntegration.h>
#include <app/clusters/commissioning-proxy-server/tests/CommissioningProxyMockDelegate.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissioningProxy;
using namespace chip::Testing;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestCommissioningProxyClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}

    TestServerClusterContext mContext;
};

TEST_F(TestCommissioningProxyClusterBackwardsCompatibility, TestInstanceLifecycle)
{
    // Test 1: Create Instance with all features
    {
        CommissioningProxyMockDelegate mockDelegate;
        BitMask<Feature> allFeatures(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);

        Instance instance(kTestEndpointId, mockDelegate, allFeatures);

        // Test initialization (registration)
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify cluster is registered in the registry
        auto * registeredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id));
        ASSERT_NE(registeredCluster, nullptr);

        // Test feature checking
        EXPECT_TRUE(instance.HasFeature(Feature::kBackgroundScan));
        EXPECT_TRUE(instance.HasFeature(Feature::kWiFiNetworkInterface));

        // Test shutdown (unregistration)
        instance.Shutdown(ClusterShutdownType::kClusterShutdown);

        // Verify cluster is unregistered from the registry
        auto * unregisteredCluster =
            CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, CommissioningProxy::Id));
        EXPECT_EQ(unregisteredCluster, nullptr);
    }

    // Test Instance with no features enabled
    {
        CommissioningProxyMockDelegate mockDelegate;
        BitMask<Feature> noFeatures;
        Instance instance(kTestEndpointId, mockDelegate, noFeatures);

        // Test initialization
        EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

        // Verify no features are enabled
        EXPECT_FALSE(instance.HasFeature(Feature::kBackgroundScan));
        EXPECT_FALSE(instance.HasFeature(Feature::kWiFiNetworkInterface));

        // Test shutdown
        instance.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

} // namespace
