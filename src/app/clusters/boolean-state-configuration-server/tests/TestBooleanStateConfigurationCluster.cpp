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

#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <clusters/BooleanStateConfiguration/Enums.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanStateConfiguration;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Test::ClusterTester;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestBooleanStateConfigurationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

constexpr EndpointId kTestEndpointId = 1;

BooleanStateConfigurationCluster::StartupConfiguration DefaultConfig()
{
    return { .supportedSensitivityLevels = 7, .defaultSensitivityLevel = 3, .alarmsSupported = 0 };
}

TEST_F(TestBooleanStateConfigurationCluster, TestAttributeList)
{
    // cluster without any attributes
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, {}, {}, DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    // cluster supporting some things
    {
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kSensitivityLevel, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::SensorFault::Id>() }, DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;

        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::CurrentSensitivityLevel::kMetadataEntry,
                      Attributes::SupportedSensitivityLevels::kMetadataEntry,
                      Attributes::AlarmsActive::kMetadataEntry,
                      Attributes::AlarmsSupported::kMetadataEntry,
                      Attributes::SensorFault::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting only visual alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kVisual), {}, DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;

        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::AlarmsActive::kMetadataEntry,
                      Attributes::AlarmsSupported::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting alarm suppression (but not visual or audible)
    // This is not a valid configuration, but we should handle it gracefully
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kAlarmSuppress), {}, DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::AlarmsSuppressed::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting visual alarms and alarm suppression
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, { Feature::kVisual, Feature::kAlarmSuppress }, {},
                                                 DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;

        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::AlarmsActive::kMetadataEntry,
                      Attributes::AlarmsSuppressed::kMetadataEntry,
                      Attributes::AlarmsSupported::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting all features and optional attributes
    {
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible, Feature::kAlarmSuppress, Feature::kSensitivityLevel },
            { BooleanStateConfigurationCluster::OptionalAttributesSet()
                  .Set<Attributes::DefaultSensitivityLevel::Id>()
                  .Set<Attributes::AlarmsEnabled::Id>()
                  .Set<Attributes::SensorFault::Id>() },
            DefaultConfig());
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;

        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::CurrentSensitivityLevel::kMetadataEntry,
                      Attributes::SupportedSensitivityLevels::kMetadataEntry,
                      Attributes::DefaultSensitivityLevel::kMetadataEntry,
                      Attributes::AlarmsActive::kMetadataEntry,
                      Attributes::AlarmsSuppressed::kMetadataEntry,
                      Attributes::AlarmsEnabled::kMetadataEntry,
                      Attributes::AlarmsSupported::kMetadataEntry,
                      Attributes::SensorFault::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestAcceptedCommandList)
{
    // cluster without any features
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, {}, {}, DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting only visual alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kVisual), {}, DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::EnableDisableAlarm::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting only audible alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kAudible), {}, DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::EnableDisableAlarm::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting alarm suppression (but not visual or audible)
    // This is not a valid configuration, but we should handle it gracefully
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kAlarmSuppress), {}, DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting visual alarms and alarm suppression
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, { Feature::kVisual, Feature::kAlarmSuppress }, {},
                                                 DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::EnableDisableAlarm::kMetadataEntry,
                      Commands::SuppressAlarm::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
    // cluster supporting all features
    {
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible, Feature::kAlarmSuppress, Feature::kSensitivityLevel }, {},
            DefaultConfig());
        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kTestEndpointId, Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Commands::EnableDisableAlarm::kMetadataEntry,
                      Commands::SuppressAlarm::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

} // namespace
