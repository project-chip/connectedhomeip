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
#include "clusters/BooleanStateConfiguration/Commands.h"
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/boolean-state-configuration-server/BooleanStateConfigurationCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/BooleanStateConfiguration/Enums.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanStateConfiguration;

using chip::app::Clusters::BooleanStateConfiguration::Feature;
using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Test::ClusterTester;
using chip::Test::TestServerClusterContext;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestBooleanStateConfigurationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

constexpr EndpointId kTestEndpointId = 1;

class StartupConfigurationBuilder
{
public:
    StartupConfigurationBuilder() = default;
    operator BooleanStateConfigurationCluster::StartupConfiguration() { return build(); }

    BooleanStateConfigurationCluster::StartupConfiguration build()
    {
        return { .supportedSensitivityLevels = mSupportedSensitivityLevels,
                 .defaultSensitivityLevel    = mDefaultSensitivityLevel,
                 .alarmsSupported            = mAlarmsSupported };
    }

    uint8_t DefaultSensitivityLevel() const { return mDefaultSensitivityLevel; }
    uint8_t SupportedSensitivityLevels() const { return mSupportedSensitivityLevels; }

    StartupConfigurationBuilder & WithSupportedSensitivityLevels(uint8_t level)
    {
        mSupportedSensitivityLevels = level;
        return *this;
    }
    StartupConfigurationBuilder & WithDefaultSensitivityLevel(uint8_t level)
    {
        mDefaultSensitivityLevel = level;
        return *this;
    }

    StartupConfigurationBuilder & AddAlarmsSupported(BooleanStateConfiguration::AlarmModeBitmap alarm)
    {
        mAlarmsSupported.Set(alarm);
        return *this;
    }

private:
    uint8_t mSupportedSensitivityLevels = 7;
    uint8_t mDefaultSensitivityLevel    = 3;
    BooleanStateConfigurationCluster::AlarmModeBitMask mAlarmsSupported;
};

StartupConfigurationBuilder DefaultConfig()
{
    return {};
}

class ScopedSafeAttributePersistence
{
public:
    ScopedSafeAttributePersistence(TestServerClusterContext & context) : mOldPersistence(app::GetSafeAttributePersistenceProvider())
    {
        VerifyOrDie(mPersistence.Init(&context.StorageDelegate()) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistence);
    }
    ~ScopedSafeAttributePersistence() { app::SetSafeAttributePersistenceProvider(mOldPersistence); }

private:
    app::SafeAttributePersistenceProvider * mOldPersistence;
    app::DefaultSafeAttributePersistenceProvider mPersistence;
};

TEST_F(TestBooleanStateConfigurationCluster, TestAttributeList)
{
    // cluster without any attributes
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, {}, {}, DefaultConfig());

        ASSERT_TRUE(IsAttributesListEqualTo(cluster, {}));
    }

    // cluster supporting some things
    {
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kSensitivityLevel, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::SensorFault::Id>() }, DefaultConfig());

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::CurrentSensitivityLevel::kMetadataEntry,
                                                Attributes::SupportedSensitivityLevels::kMetadataEntry,
                                                Attributes::AlarmsActive::kMetadataEntry,
                                                Attributes::AlarmsSupported::kMetadataEntry,
                                                Attributes::SensorFault::kMetadataEntry,
                                            }));
    }
    // cluster supporting only visual alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kVisual), {}, DefaultConfig());
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::AlarmsActive::kMetadataEntry,
                                                Attributes::AlarmsSupported::kMetadataEntry,
                                            }));
    }
    // cluster supporting alarm suppression (but not visual or audible)
    // This is not a valid configuration, but we should handle it gracefully
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kAlarmSuppress), {}, DefaultConfig());
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::AlarmsSuppressed::kMetadataEntry,
                                            }));
    }
    // cluster supporting visual alarms and alarm suppression
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, { Feature::kVisual, Feature::kAlarmSuppress }, {},
                                                 DefaultConfig());
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::AlarmsActive::kMetadataEntry,
                                                Attributes::AlarmsSuppressed::kMetadataEntry,
                                                Attributes::AlarmsSupported::kMetadataEntry,
                                            }));
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
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::CurrentSensitivityLevel::kMetadataEntry,
                                                Attributes::SupportedSensitivityLevels::kMetadataEntry,
                                                Attributes::DefaultSensitivityLevel::kMetadataEntry,
                                                Attributes::AlarmsActive::kMetadataEntry,
                                                Attributes::AlarmsSuppressed::kMetadataEntry,
                                                Attributes::AlarmsEnabled::kMetadataEntry,
                                                Attributes::AlarmsSupported::kMetadataEntry,
                                                Attributes::SensorFault::kMetadataEntry,
                                            }));
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestAcceptedCommandList)
{
    // cluster without any features
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, {}, {}, DefaultConfig());
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));
    }
    // cluster supporting only visual alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kVisual), {}, DefaultConfig());
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      Commands::EnableDisableAlarm::kMetadataEntry,
                                                  }));
    }
    // cluster supporting only audible alarms
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kAudible), {}, DefaultConfig());
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      Commands::EnableDisableAlarm::kMetadataEntry,
                                                  }));
    }
    // cluster supporting visual alarms and alarm suppression
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, { Feature::kVisual, Feature::kAlarmSuppress }, {},
                                                 DefaultConfig());
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      Commands::EnableDisableAlarm::kMetadataEntry,
                                                      Commands::SuppressAlarm::kMetadataEntry,
                                                  }));
    }
    // cluster supporting all features
    {
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible, Feature::kAlarmSuppress, Feature::kSensitivityLevel }, {},
            DefaultConfig());
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      Commands::EnableDisableAlarm::kMetadataEntry,
                                                      Commands::SuppressAlarm::kMetadataEntry,
                                                  }));
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestSensitivityClamping)
{
    TestServerClusterContext context;
    ScopedSafeAttributePersistence persistence(context);

    // supportedSensitivityLevels is clamped to [2, 10]
    {
        // Test value below min
        auto config = DefaultConfig().WithSupportedSensitivityLevels(1);
        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ClusterTester tester(cluster);

        uint8_t supportedLevels = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::SupportedSensitivityLevels::Id, supportedLevels),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(supportedLevels, BooleanStateConfigurationCluster::kMinSupportedSensitivityLevels);
    }
    {
        // Test value above max
        auto config = DefaultConfig().WithSupportedSensitivityLevels(101);
        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ClusterTester tester(cluster);

        uint8_t supportedLevels = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::SupportedSensitivityLevels::Id, supportedLevels),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(supportedLevels, BooleanStateConfigurationCluster::kMaxSupportedSensitivityLevels);
    }

    // defaultSensitivityLevel is clamped to supported-1
    {
        auto config = DefaultConfig().WithDefaultSensitivityLevel(5).WithSupportedSensitivityLevels(5);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, Feature::kSensitivityLevel,
            BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::DefaultSensitivityLevel::Id>(), config);
        ClusterTester tester(cluster);

        uint8_t defaultLevel = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::DefaultSensitivityLevel::Id, defaultLevel),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(defaultLevel, 4);
    }

    // Writing CurrentSensitivityLevel is clamped
    {
        auto config = DefaultConfig().WithSupportedSensitivityLevels(10);
        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        uint8_t currentLevel = 0;

        // Write a valid level
        EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSensitivityLevel::Id, static_cast<uint8_t>(5)),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, currentLevel),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(currentLevel, 5);

        // Write an invalid level
        EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSensitivityLevel::Id, static_cast<uint8_t>(10)),
                  Protocols::InteractionModel::Status::ConstraintError);

        // Value should not have changed
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, currentLevel),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(currentLevel, 5);
        cluster.Shutdown();
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestPersistenceAndStartup)
{
    TestServerClusterContext context;
    ScopedSafeAttributePersistence persistence(context);

    // 1. Create a cluster, write a value.
    {
        auto config = DefaultConfig().WithSupportedSensitivityLevels(9);

        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        // check default value first
        uint8_t sensitivity = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, sensitivity),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(sensitivity, config.DefaultSensitivityLevel());

        // Write a new value. This will be persisted.
        uint8_t levelToWrite = 6;
        EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSensitivityLevel::Id, levelToWrite),
                  Protocols::InteractionModel::Status::Success);
        cluster.Shutdown();
    }

    // 2. Create a new cluster instance with the same context, and check if the value was restored.
    {
        auto config = DefaultConfig().WithSupportedSensitivityLevels(9);

        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint8_t sensitivity = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, sensitivity),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(sensitivity, 6); // Check if value is persisted.
        cluster.Shutdown();
    }

    // 3. Create another new cluster with a smaller supported range and check clamping on startup.
    {
        auto smallerConfig = DefaultConfig().WithSupportedSensitivityLevels(5).WithDefaultSensitivityLevel(3);
        // The stored value 6 is now out of bounds.
        // Default sensitivity for this config is 3.

        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, smallerConfig);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR); // Should read 6 and clamp it to (max - 1 == 4)

        ClusterTester tester(cluster);
        uint8_t sensitivity = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, sensitivity),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(sensitivity, 4); // 5-1. Clamped from persisted value.

        cluster.Shutdown();
    }

    // 4. Test that if persistence fails, default is used. Let's clear the storage.
    context.StorageDelegate().ClearStorage();
    {
        auto config = DefaultConfig().WithSupportedSensitivityLevels(9).WithDefaultSensitivityLevel(5);

        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        uint8_t sensitivity = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::CurrentSensitivityLevel::Id, sensitivity),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(sensitivity, 5); // Should be default, as storage is empty.
        cluster.Shutdown();
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestAlarmsEnabledPersistence)
{
    TestServerClusterContext context;
    ScopedSafeAttributePersistence persistence(context);

    // 1. Create a cluster, set a value for AlarmsEnabled, which should be persisted.
    {
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kVisual);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);

        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        // Check default value first
        BooleanStateConfigurationCluster::AlarmModeBitMask alarmsEnabled;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AlarmsEnabled::Id, alarmsEnabled), Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(alarmsEnabled.Raw(), 0);

        // Set a new value. This will be persisted.
        Commands::EnableDisableAlarm::Type request;
        request.alarmsToEnableDisable.Set(AlarmModeBitmap::kVisual);

        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());

        EXPECT_EQ(tester.ReadAttribute(Attributes::AlarmsEnabled::Id, alarmsEnabled), Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(alarmsEnabled.Has(AlarmModeBitmap::kVisual));
        EXPECT_FALSE(alarmsEnabled.Has(AlarmModeBitmap::kAudible));

        cluster.Shutdown();
    }

    // 2. Create a new cluster instance with the same context, and check if the value was restored.
    {
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kAudible).AddAlarmsSupported(AlarmModeBitmap::kVisual);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        BooleanStateConfigurationCluster::AlarmModeBitMask alarmsEnabled;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AlarmsEnabled::Id, alarmsEnabled), Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(alarmsEnabled.Has(AlarmModeBitmap::kVisual)); // Check if value is persisted.
        EXPECT_FALSE(alarmsEnabled.Has(AlarmModeBitmap::kAudible));
        cluster.Shutdown();
    }

    // 3. Test that if persistence fails, default is used. Let's clear the storage.
    context.StorageDelegate().ClearStorage();
    {
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kAudible).AddAlarmsSupported(AlarmModeBitmap::kVisual);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        BooleanStateConfigurationCluster::AlarmModeBitMask alarmsEnabled;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AlarmsEnabled::Id, alarmsEnabled), Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(alarmsEnabled.Raw(), 0); // Should be default, as storage is empty.
        cluster.Shutdown();
    }
}

} // namespace
