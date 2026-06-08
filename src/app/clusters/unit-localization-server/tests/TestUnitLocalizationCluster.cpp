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

#include <app/clusters/unit-localization-server/UnitLocalizationCluster.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/UnitLocalization/Attributes.h>
#include <clusters/UnitLocalization/Metadata.h>

#include <iostream>

using namespace chip::Testing;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;
using namespace chip::app::Clusters::UnitLocalization::Attributes;

struct TestUnitLocalizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

// Tests a cluster without Features enabled, not really useful by itself but possible.
TEST_F(TestUnitLocalizationCluster, AttributeNoFeatures)
{
    UnitLocalizationCluster unitLocalizationCluster{ kRootEndpointId, {} };
    // This cluster doesn't have any unique Mandatory attribute.
    ASSERT_TRUE(IsAttributesListEqualTo(unitLocalizationCluster, {}));
}

// Tests a cluster with the TemperatureUnit attribute enabled.
TEST_F(TestUnitLocalizationCluster, AttributesTemperatureFeature)
{
    const BitFlags<Feature> features{ Feature::kTemperatureUnit };
    UnitLocalizationCluster unitLocalizationCluster{ kRootEndpointId, features };

    ASSERT_TRUE(IsAttributesListEqualTo(unitLocalizationCluster,
                                        { TemperatureUnit::kMetadataEntry, SupportedTemperatureUnits::kMetadataEntry }));
}

TEST_F(TestUnitLocalizationCluster, ReadMandatoryAttributes)
{
    UnitLocalizationCluster unitLocalizationCluster{ kRootEndpointId, {} };

    ClusterTester tester(unitLocalizationCluster);
    uint16_t clusterRevision = 0;
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);

    uint32_t featureMap = 1;
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestUnitLocalizationCluster, ReadSupportedTemperatureUnits)
{
    const BitFlags<Feature> features{ Feature::kTemperatureUnit };
    UnitLocalizationCluster unitLocalizationCluster{ kRootEndpointId, features };

    // Set supported units as Fahrenheit and Celsius
    TempUnitEnum supportedUnits[2] = { TempUnitEnum::kCelsius, TempUnitEnum::kFahrenheit };
    DataModel::List<TempUnitEnum> unitsList(supportedUnits);
    ASSERT_EQ(unitLocalizationCluster.SetSupportedTemperatureUnits(unitsList), CHIP_NO_ERROR);

    // Read attribute
    ClusterTester tester(unitLocalizationCluster);
    SupportedTemperatureUnits::TypeInfo::DecodableType supportedTempUnits{};
    ASSERT_EQ(tester.ReadAttribute(SupportedTemperatureUnits::Id, supportedTempUnits), CHIP_NO_ERROR);

    // Verify that the values are stored properly
    auto it = supportedTempUnits.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), TempUnitEnum::kCelsius);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), TempUnitEnum::kFahrenheit);
    ASSERT_FALSE(it.Next());
}

TEST_F(TestUnitLocalizationCluster, WriteAndReadTemperatureUnit)
{
    chip::Testing::TestServerClusterContext context;
    const BitFlags<Feature> features{ Feature::kTemperatureUnit };
    UnitLocalizationCluster unitLocalizationCluster{ kRootEndpointId, features };
    EXPECT_EQ(unitLocalizationCluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(unitLocalizationCluster);

    // Set supported units as Kelvin and Celsius
    TempUnitEnum supportedUnits[2] = { TempUnitEnum::kCelsius, TempUnitEnum::kKelvin };
    DataModel::List<TempUnitEnum> unitsList(supportedUnits);
    ASSERT_EQ(unitLocalizationCluster.SetSupportedTemperatureUnits(unitsList), CHIP_NO_ERROR);

    TempUnitEnum temperatureUnit        = TempUnitEnum::kKelvin;
    TempUnitEnum currentTemperatureUnit = TempUnitEnum::kFahrenheit;

    // Set TemperatureUnit to Kelvin
    ASSERT_EQ(tester.WriteAttribute(TemperatureUnit::Id, temperatureUnit), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(TemperatureUnit::Id, currentTemperatureUnit), CHIP_NO_ERROR);
    EXPECT_EQ(currentTemperatureUnit, TempUnitEnum::kKelvin);
    // Set TemperatureUnit to Celsius

    temperatureUnit = TempUnitEnum::kCelsius;
    ASSERT_EQ(tester.WriteAttribute(TemperatureUnit::Id, temperatureUnit), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(TemperatureUnit::Id, currentTemperatureUnit), CHIP_NO_ERROR);
    EXPECT_EQ(currentTemperatureUnit, TempUnitEnum::kCelsius);

    // Fahrenheit not supported, should return error
    temperatureUnit = TempUnitEnum::kFahrenheit;
    ASSERT_EQ(tester.WriteAttribute(TemperatureUnit::Id, temperatureUnit), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}
