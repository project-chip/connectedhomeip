/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;
using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::Status;

struct TestThermostatUserInterfaceConfigurationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    TemperatureDisplayModeEnum temperatureDisplayMode{};
    ASSERT_EQ(tester.ReadAttribute(TemperatureDisplayMode::Id, temperatureDisplayMode), CHIP_NO_ERROR);
    EXPECT_EQ(temperatureDisplayMode, TemperatureDisplayModeEnum::kCelsius);

    KeypadLockoutEnum keypadLockout{};
    ASSERT_EQ(tester.ReadAttribute(KeypadLockout::Id, keypadLockout), CHIP_NO_ERROR);
    EXPECT_EQ(keypadLockout, KeypadLockoutEnum::kNoLockout);
}

} // namespace

TEST_F(TestThermostatUserInterfaceConfigurationCluster, AttributeListTest)
{
    {
        ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, ThermostatUserInterfaceConfiguration::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { ScheduleProgrammingVisibility::kMetadataEntry };
        ThermostatUserInterfaceConfigurationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<ScheduleProgrammingVisibility::Id>();

        ThermostatUserInterfaceConfigurationCluster::Config config;
        config.optionalAttributes = optionalAttributeSet;

        ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, ThermostatUserInterfaceConfiguration::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, ReadAttributeTest)
{
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    TestMandatoryAttributes(tester);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, WriteAttributeTest)
{
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(TemperatureDisplayMode::Id, TemperatureDisplayModeEnum::kFahrenheit), CHIP_NO_ERROR);
    TemperatureDisplayModeEnum temperatureDisplayMode{};
    ASSERT_EQ(tester.ReadAttribute(TemperatureDisplayMode::Id, temperatureDisplayMode), CHIP_NO_ERROR);
    EXPECT_EQ(temperatureDisplayMode, TemperatureDisplayModeEnum::kFahrenheit);

    ASSERT_EQ(tester.WriteAttribute(KeypadLockout::Id, KeypadLockoutEnum::kLockout3), CHIP_NO_ERROR);
    KeypadLockoutEnum keypadLockout{};
    ASSERT_EQ(tester.ReadAttribute(KeypadLockout::Id, keypadLockout), CHIP_NO_ERROR);
    EXPECT_EQ(keypadLockout, KeypadLockoutEnum::kLockout3);

    auto status = tester.WriteAttribute(TemperatureDisplayMode::Id, TemperatureDisplayModeEnum::kUnknownEnumValue);
    EXPECT_EQ(status, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    status = tester.WriteAttribute(KeypadLockout::Id, KeypadLockoutEnum::kUnknownEnumValue);
    EXPECT_EQ(status, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, ScheduleProgrammingVisibilityTest)
{
    ThermostatUserInterfaceConfigurationCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<ScheduleProgrammingVisibility::Id>();

    ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes            = optionalAttributeSet;
    config.scheduleProgrammingVisibility = ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted;

    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ScheduleProgrammingVisibilityEnum scheduleProgrammingVisibility{};
    ASSERT_EQ(tester.ReadAttribute(ScheduleProgrammingVisibility::Id, scheduleProgrammingVisibility), CHIP_NO_ERROR);
    EXPECT_EQ(scheduleProgrammingVisibility, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted);

    ASSERT_EQ(
        tester.WriteAttribute(ScheduleProgrammingVisibility::Id, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied),
        CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(ScheduleProgrammingVisibility::Id, scheduleProgrammingVisibility), CHIP_NO_ERROR);
    EXPECT_EQ(scheduleProgrammingVisibility, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied);

    auto status = tester.WriteAttribute(ScheduleProgrammingVisibility::Id, ScheduleProgrammingVisibilityEnum::kUnknownEnumValue);
    EXPECT_EQ(status, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, SetterTest)
{
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kFahrenheit), Status::Success);
    EXPECT_EQ(cluster.GetTemperatureDisplayMode(), TemperatureDisplayModeEnum::kFahrenheit);

    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kLockout5), Status::Success);
    EXPECT_EQ(cluster.GetKeypadLockout(), KeypadLockoutEnum::kLockout5);

    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kUnknownEnumValue), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, TemperatureDisplayModeDelegate)
{
    struct TestDelegate : ThermostatUserInterfaceConfiguration::Delegate
    {
        void OnTemperatureDisplayModeChanged(TemperatureDisplayModeEnum value) override
        {
            EXPECT_EQ(value, TemperatureDisplayModeEnum::kFahrenheit);
            EXPECT_EQ(cluster->GetTemperatureDisplayMode(), value);
            ++calls;
        }

        ThermostatUserInterfaceConfigurationCluster * cluster = nullptr;
        unsigned calls                                        = 0;
    } delegate;

    ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes.Set<ScheduleProgrammingVisibility::Id>();
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);
    delegate.cluster = &cluster;
    cluster.SetDelegate(&delegate);

    // No-op writes and invalid enum values must not reach the application.
    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kCelsius), Status::Success);
    EXPECT_EQ(tester.WriteAttribute(TemperatureDisplayMode::Id, TemperatureDisplayModeEnum::kCelsius), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kUnknownEnumValue), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(TemperatureDisplayMode::Id, TemperatureDisplayModeEnum::kUnknownEnumValue),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(delegate.calls, 0u);

    EXPECT_EQ(tester.WriteAttribute(TemperatureDisplayMode::Id, TemperatureDisplayModeEnum::kFahrenheit), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTemperatureDisplayMode(), TemperatureDisplayModeEnum::kFahrenheit);
    EXPECT_EQ(delegate.calls, 1u);
    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kFahrenheit), Status::Success);
    EXPECT_EQ(delegate.calls, 1u);

    cluster.SetDelegate(nullptr);
    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kCelsius), Status::Success);
    EXPECT_EQ(delegate.calls, 1u);
    cluster.SetDelegate(&delegate);
    EXPECT_EQ(cluster.SetTemperatureDisplayMode(TemperatureDisplayModeEnum::kFahrenheit), Status::Success);
    EXPECT_EQ(cluster.GetTemperatureDisplayMode(), TemperatureDisplayModeEnum::kFahrenheit);
    EXPECT_EQ(delegate.calls, 2u);

    cluster.SetDelegate(nullptr);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, KeypadLockoutDelegate)
{
    struct TestDelegate : ThermostatUserInterfaceConfiguration::Delegate
    {
        void OnKeypadLockoutChanged(KeypadLockoutEnum value) override
        {
            EXPECT_EQ(value, KeypadLockoutEnum::kLockout3);
            EXPECT_EQ(cluster->GetKeypadLockout(), value);
            ++calls;
        }

        ThermostatUserInterfaceConfigurationCluster * cluster = nullptr;
        unsigned calls                                        = 0;
    } delegate;

    ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes.Set<ScheduleProgrammingVisibility::Id>();
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);
    delegate.cluster = &cluster;
    cluster.SetDelegate(&delegate);

    // No-op writes and invalid enum values must not reach the application.
    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kNoLockout), Status::Success);
    EXPECT_EQ(tester.WriteAttribute(KeypadLockout::Id, KeypadLockoutEnum::kNoLockout), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kUnknownEnumValue), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(KeypadLockout::Id, KeypadLockoutEnum::kUnknownEnumValue),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(delegate.calls, 0u);

    EXPECT_EQ(tester.WriteAttribute(KeypadLockout::Id, KeypadLockoutEnum::kLockout3), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetKeypadLockout(), KeypadLockoutEnum::kLockout3);
    EXPECT_EQ(delegate.calls, 1u);
    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kLockout3), Status::Success);
    EXPECT_EQ(delegate.calls, 1u);

    cluster.SetDelegate(nullptr);
    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kNoLockout), Status::Success);
    EXPECT_EQ(delegate.calls, 1u);
    cluster.SetDelegate(&delegate);
    EXPECT_EQ(cluster.SetKeypadLockout(KeypadLockoutEnum::kLockout3), Status::Success);
    EXPECT_EQ(cluster.GetKeypadLockout(), KeypadLockoutEnum::kLockout3);
    EXPECT_EQ(delegate.calls, 2u);

    cluster.SetDelegate(nullptr);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThermostatUserInterfaceConfigurationCluster, ScheduleProgrammingVisibilityDelegate)
{
    struct TestDelegate : ThermostatUserInterfaceConfiguration::Delegate
    {
        void OnScheduleProgrammingVisibilityChanged(ScheduleProgrammingVisibilityEnum value) override
        {
            EXPECT_EQ(value, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied);
            EXPECT_EQ(cluster->GetScheduleProgrammingVisibility(), value);
            ++calls;
        }

        ThermostatUserInterfaceConfigurationCluster * cluster = nullptr;
        unsigned calls                                        = 0;
    } delegate;

    ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes.Set<ScheduleProgrammingVisibility::Id>();
    ThermostatUserInterfaceConfigurationCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);
    delegate.cluster = &cluster;
    cluster.SetDelegate(&delegate);

    // No-op writes and invalid enum values must not reach the application.
    EXPECT_EQ(cluster.SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted),
              Status::Success);
    EXPECT_EQ(
        tester.WriteAttribute(ScheduleProgrammingVisibility::Id, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum::kUnknownEnumValue),
              Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(ScheduleProgrammingVisibility::Id, ScheduleProgrammingVisibilityEnum::kUnknownEnumValue),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(delegate.calls, 0u);

    EXPECT_EQ(
        tester.WriteAttribute(ScheduleProgrammingVisibility::Id, ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetScheduleProgrammingVisibility(), ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied);
    EXPECT_EQ(delegate.calls, 1u);
    EXPECT_EQ(cluster.SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied),
              Status::Success);
    EXPECT_EQ(delegate.calls, 1u);

    cluster.SetDelegate(nullptr);
    EXPECT_EQ(cluster.SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted),
              Status::Success);
    EXPECT_EQ(delegate.calls, 1u);
    cluster.SetDelegate(&delegate);
    EXPECT_EQ(cluster.SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied),
              Status::Success);
    EXPECT_EQ(cluster.GetScheduleProgrammingVisibility(), ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied);
    EXPECT_EQ(delegate.calls, 2u);

    cluster.SetDelegate(nullptr);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
