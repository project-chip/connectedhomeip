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

#include <app/clusters/power-topology-server/CodegenIntegration.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/clusters/power-topology-server/tests/MockPowerTopologyDelegate.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;
using namespace chip::Testing;

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

    Instance instance(kTestEndpointId, mockDelegate, features);

    // Test initialization
    EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);

    // Test shutdown
    instance.Shutdown();
}

} // namespace
