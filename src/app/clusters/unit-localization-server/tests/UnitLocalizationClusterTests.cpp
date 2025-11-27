/*
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

#include <pw_unit_test/framework.h>

#include <app/clusters/unit-localization-server/unit-localization-server.h>

struct TestUnitLocalizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestUnitLocalizationCluster, AttributeTest)
{

    TestServerClusterContext clusterContext;

    ConcreteClusterPath clusterPath(kRootEndpointId, UnitLocalization::Id);
    BitFlags<UnitLocalization::Feature> features{ 0 };

    UnitLocalizationCluster cluster(clusterPath, features);

    // Test attributes listing with no features enabled
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;

    ASSERT_EQ(onlyMandatory.Attributes(clusterPath, attributesBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedAttributes;
    ASSERT_EQ(expectedAttributes.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
    ASSERT_EQ(expectedAttributes.AppendElements({ UnitLocalization::Attributes::TemperatureUnit::}), CHIP_NO_ERROR);

    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedAttributes.TakeBuffer()));
}

TEST_F(TestUnitLocalizationCluster, MigrationTest)
{

    TestServerClusterContext clusterContext;

    ConcreteClusterPath clusterPath(kRootEndpointId, UnitLocalization::Id);
    BitFlags<UnitLocalization::Feature> features{ 0 };

    UnitLocalizationCluster cluster(clusterPath, features);

    // Test attributes listing with no features enabled
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;

    ASSERT_EQ(onlyMandatory.Attributes(clusterPath, attributesBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedAttributes;
    ASSERT_EQ(expectedAttributes.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
    ASSERT_EQ(expectedAttributes.AppendElements({ UnitLocalization::Attributes::TemperatureUnit::}), CHIP_NO_ERROR);

    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedAttributes.TakeBuffer()));
}
