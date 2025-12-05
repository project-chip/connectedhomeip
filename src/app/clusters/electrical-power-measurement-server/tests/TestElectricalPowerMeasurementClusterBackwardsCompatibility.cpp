/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/electrical-power-measurement-server/CodegenIntegration.h>
#include <app/clusters/electrical-power-measurement-server/tests/MockElectricalPowerMeasurementDelegate.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestElectricalPowerMeasurementClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
    void TearDown() override {}

    chip::Test::TestServerClusterContext mContext;
};

TEST_F(TestElectricalPowerMeasurementClusterBackwardsCompatibility, TestInstanceLifecycle)
{
    // Valid combination of features and optional attributes
    BitMask<Feature> features(Feature::kAlternatingCurrent);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeVoltage,
                                              OptionalAttributes::kOptionalAttributeActiveCurrent);

    MockElectricalPowerMeasurementDelegate delegate;
    chip::app::SingleEndpointServerClusterRegistry & registry = chip::app::CodegenDataModelProvider::Instance().Registry();

    {
        Instance instance(kTestEndpointId, delegate, features, optionalAttrs);

        // Verify the cluster is registered
        chip::app::ServerClusterInterface * cluster = registry.Get({ kTestEndpointId, ElectricalPowerMeasurement::Id });
        EXPECT_NE(cluster, nullptr);
        EXPECT_EQ(cluster->Startup(mContext.Get()), CHIP_NO_ERROR);
    }

    // After instance is destroyed, cluster should be unregistered
    chip::app::ServerClusterInterface * cluster = registry.Get({ kTestEndpointId, ElectricalPowerMeasurement::Id });
    EXPECT_EQ(cluster, nullptr);
}

TEST_F(TestElectricalPowerMeasurementClusterBackwardsCompatibility, TestInstanceLifecycleWithError)
{

    // Invalid combination of features and optional attributes
    BitMask<Feature> features(Feature::kDirectCurrent);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeFrequency);

    MockElectricalPowerMeasurementDelegate delegate;
    chip::app::SingleEndpointServerClusterRegistry & registry = chip::app::CodegenDataModelProvider::Instance().Registry();
    {
        Instance instance(kTestEndpointId, delegate, features, optionalAttrs);

        // Verify the cluster is registered and handles the error.
        chip::app::ServerClusterInterface * cluster = registry.Get({ kTestEndpointId, ElectricalPowerMeasurement::Id });
        EXPECT_NE(cluster, nullptr);
        EXPECT_EQ(cluster->Startup(mContext.Get()), CHIP_ERROR_INCORRECT_STATE);
    }

    // After instance is destroyed, cluster should be unregistered
    chip::app::ServerClusterInterface * cluster = registry.Get({ kTestEndpointId, ElectricalPowerMeasurement::Id });
    EXPECT_EQ(cluster, nullptr);
}

} // namespace
