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

#include <app/ConcreteClusterPath.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Enums.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <pw_unit_test/framework.h>

// Required when linking against the codegen mock model.
void InitDataModelHandler() {}

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

namespace {

constexpr ClusterId kTestClusterId   = CarbonDioxideConcentrationMeasurement::Id;
constexpr EndpointId kTestEndpointId = kRootEndpointId;
constexpr float kTestMin             = 0.0f;
constexpr float kTestMax             = 100.0f;

using NumericInstance = ConcentrationMeasurement::Instance<true, false, false, false, false, false>;

struct TestConcentrationMeasurementBackwardsCompat : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Fixture for tests that require an already-initialized instance.
struct TestConcentrationMeasurementBackwardsCompatInitialized : public TestConcentrationMeasurementBackwardsCompat
{
    void SetUp() override
    {
        ASSERT_EQ(instance.SetMinMeasuredValue(DataModel::MakeNullable(kTestMin)), CHIP_NO_ERROR);
        ASSERT_EQ(instance.SetMaxMeasuredValue(DataModel::MakeNullable(kTestMax)), CHIP_NO_ERROR);
        ASSERT_EQ(instance.Init(), CHIP_NO_ERROR);
    }

    NumericInstance instance{ kTestEndpointId, kTestClusterId, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm };
};

} // namespace

TEST_F(TestConcentrationMeasurementBackwardsCompat, PreInitSetters_AcceptValues)
{
    NumericInstance instance(kTestEndpointId, kTestClusterId, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    EXPECT_EQ(instance.SetMinMeasuredValue(DataModel::MakeNullable(kTestMin)), CHIP_NO_ERROR);
    EXPECT_EQ(instance.SetMaxMeasuredValue(DataModel::MakeNullable(kTestMax)), CHIP_NO_ERROR);
    EXPECT_EQ(instance.SetUncertainty(0.5f), CHIP_NO_ERROR);
}

TEST_F(TestConcentrationMeasurementBackwardsCompatInitialized, PreInitSetters_FailAfterInit)
{
    EXPECT_EQ(instance.SetMinMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(instance.SetMaxMeasuredValue(DataModel::MakeNullable(99.0f)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(instance.SetUncertainty(1.0f), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestConcentrationMeasurementBackwardsCompatInitialized, Init_RegistersClusterInRegistry)
{
    auto * reg = CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, kTestClusterId));
    EXPECT_NE(reg, nullptr);
}

TEST_F(TestConcentrationMeasurementBackwardsCompatInitialized, DoubleInit_IsNoOp)
{
    EXPECT_EQ(instance.Init(), CHIP_NO_ERROR);
}

TEST_F(TestConcentrationMeasurementBackwardsCompatInitialized, SettersForwardConstraintsToCluster)
{
    EXPECT_EQ(instance.SetMeasuredValue(DataModel::MakeNullable(50.0f)), CHIP_NO_ERROR);
    EXPECT_EQ(instance.SetMeasuredValue(DataModel::MakeNullable(200.0f)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}
