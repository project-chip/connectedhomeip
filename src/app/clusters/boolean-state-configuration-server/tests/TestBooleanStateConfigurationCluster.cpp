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

#include <app/clusters/boolean-state-configuration-server/BooleanStateConfigurationCluster.h>
#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-delegate.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/BooleanStateConfiguration/Commands.h>
#include <clusters/BooleanStateConfiguration/Enums.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TypeTraits.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanStateConfiguration;

using chip::app::ClusterShutdownType;
using chip::app::Clusters::BooleanStateConfiguration::Feature;
using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::ClusterTester;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::TestServerClusterContext;

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

TEST_F(TestBooleanStateConfigurationCluster, TestFeatureMap)
{
    // Check that the FeatureMap includes the FAULTEV bit even when no other features are set
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, {}, {}, DefaultConfig());
        ClusterTester tester(cluster);

        uint32_t features = 0;
        EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, features),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(features, to_underlying(Feature::kFaultEvents));
    }

    // Check that the FeatureMap includes the FAULTEV bit when other features are also set
    {
        BooleanStateConfigurationCluster cluster(kTestEndpointId, BitMask<Feature>(Feature::kVisual), {}, DefaultConfig());
        ClusterTester tester(cluster);
        uint32_t features = 0;
        EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, features),
                  Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(features, to_underlying(Feature::kVisual) | to_underlying(Feature::kFaultEvents));
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestSensitivityClamping)
{
    TestServerClusterContext context;

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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestPersistenceAndStartup)
{
    TestServerClusterContext context;

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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
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

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestBooleanStateConfigurationCluster, TestAlarmsEnabledPersistence)
{
    TestServerClusterContext context;

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

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
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
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Test delegate class to verify attribute change callbacks
class TestDelegate : public BooleanStateConfiguration::Delegate
{
public:
    CHIP_ERROR HandleSuppressAlarm(BooleanStateConfiguration::AlarmModeBitmap alarmToSuppress) override
    {
        mSuppressAlarmCalled = true;
        mSuppressAlarmValue  = alarmToSuppress;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleEnableDisableAlarms(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms) override
    {
        mEnableDisableAlarmsCalled = true;
        mEnableDisableAlarmsValue  = alarms;
        return CHIP_NO_ERROR;
    }

    bool OnCurrentSensitivityLevelChanged(uint8_t newValue) override
    {
        mCurrentSensitivityLevelCalled = true;
        mCurrentSensitivityLevelValue  = newValue;
        return true;
    }

    bool OnAlarmsActiveChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) override
    {
        mAlarmsActiveCalled = true;
        mAlarmsActiveValue  = newValue;
        return true;
    }

    bool OnAlarmsSuppressedChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) override
    {
        mAlarmsSuppressedCalled = true;
        mAlarmsSuppressedValue  = newValue;
        return true;
    }

    bool OnAlarmsEnabledChanged(chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> newValue) override
    {
        mAlarmsEnabledCalled = true;
        mAlarmsEnabledValue  = newValue;
        return true;
    }

    bool OnSensorFaultChanged(chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> newValue) override
    {
        mSensorFaultCalled = true;
        mSensorFaultValue  = newValue;
        return true;
    }

    // Reset all flags
    void Reset()
    {
        mCurrentSensitivityLevelCalled = false;
        mAlarmsActiveCalled            = false;
        mAlarmsSuppressedCalled        = false;
        mAlarmsEnabledCalled           = false;
        mSensorFaultCalled             = false;
        mSuppressAlarmCalled           = false;
        mEnableDisableAlarmsCalled     = false;
    }

    // Getters for test verification
    bool WasCurrentSensitivityLevelCalled() const { return mCurrentSensitivityLevelCalled; }
    uint8_t GetCurrentSensitivityLevelValue() const { return mCurrentSensitivityLevelValue; }

    bool WasAlarmsActiveCalled() const { return mAlarmsActiveCalled; }
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> GetAlarmsActiveValue() const { return mAlarmsActiveValue; }

    bool WasAlarmsSuppressedCalled() const { return mAlarmsSuppressedCalled; }
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> GetAlarmsSuppressedValue() const { return mAlarmsSuppressedValue; }

    bool WasAlarmsEnabledCalled() const { return mAlarmsEnabledCalled; }
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> GetAlarmsEnabledValue() const { return mAlarmsEnabledValue; }

    bool WasSensorFaultCalled() const { return mSensorFaultCalled; }
    chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> GetSensorFaultValue() const { return mSensorFaultValue; }

private:
    bool mCurrentSensitivityLevelCalled = false;
    uint8_t mCurrentSensitivityLevelValue{};

    bool mAlarmsActiveCalled = false;
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> mAlarmsActiveValue{};

    bool mAlarmsSuppressedCalled = false;
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> mAlarmsSuppressedValue{};

    bool mAlarmsEnabledCalled = false;
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> mAlarmsEnabledValue{};

    bool mSensorFaultCalled = false;
    chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> mSensorFaultValue{};

    bool mSuppressAlarmCalled                                      = false;
    BooleanStateConfiguration::AlarmModeBitmap mSuppressAlarmValue = BooleanStateConfiguration::AlarmModeBitmap::kVisual;

    bool mEnableDisableAlarmsCalled = false;
    chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> mEnableDisableAlarmsValue{};
};

TEST_F(TestBooleanStateConfigurationCluster, TestTypeSafeDelegateCallbacks)
{
    TestServerClusterContext context;
    TestDelegate delegate;

    // Test CurrentSensitivityLevel callback via WriteAttribute
    {
        auto config = DefaultConfig().WithSupportedSensitivityLevels(10);
        BooleanStateConfigurationCluster cluster(kTestEndpointId, Feature::kSensitivityLevel, {}, config);
        cluster.SetDelegate(&delegate);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        // Write a new sensitivity level
        uint8_t newLevel = 5;
        EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSensitivityLevel::Id, newLevel),
                  Protocols::InteractionModel::Status::Success);

        // Verify callback was called with correct value
        EXPECT_TRUE(delegate.WasCurrentSensitivityLevelCalled());
        EXPECT_EQ(delegate.GetCurrentSensitivityLevelValue(), newLevel);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test AlarmsEnabled callback via InvokeCommand (EnableDisableAlarm)
    {
        delegate.Reset();
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kVisual).AddAlarmsSupported(AlarmModeBitmap::kAudible);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAudible },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);
        cluster.SetDelegate(&delegate);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        // Invoke EnableDisableAlarm command
        Commands::EnableDisableAlarm::Type request;
        request.alarmsToEnableDisable.Set(AlarmModeBitmap::kVisual);
        request.alarmsToEnableDisable.Set(AlarmModeBitmap::kAudible);

        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());

        // Verify AlarmsEnabled callback was called
        EXPECT_TRUE(delegate.WasAlarmsEnabledCalled());
        EXPECT_TRUE(delegate.GetAlarmsEnabledValue().Has(AlarmModeBitmap::kVisual));
        EXPECT_TRUE(delegate.GetAlarmsEnabledValue().Has(AlarmModeBitmap::kAudible));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test AlarmsActive callback via SetAlarmsActive
    {
        delegate.Reset();
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kVisual);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, BitMask<Feature>(Feature::kVisual),
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);
        cluster.SetDelegate(&delegate);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Enable alarms first
        Commands::EnableDisableAlarm::Type enableRequest;
        enableRequest.alarmsToEnableDisable.Set(AlarmModeBitmap::kVisual);
        ClusterTester tester(cluster);
        EXPECT_TRUE(tester.Invoke(enableRequest).IsSuccess());

        delegate.Reset();

        // Set alarms active
        chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsToSet;
        alarmsToSet.Set(AlarmModeBitmap::kVisual);
        EXPECT_EQ(cluster.SetAlarmsActive(alarmsToSet), Protocols::InteractionModel::Status::Success);

        // Verify callback was called
        EXPECT_TRUE(delegate.WasAlarmsActiveCalled());
        EXPECT_TRUE(delegate.GetAlarmsActiveValue().Has(AlarmModeBitmap::kVisual));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test AlarmsSuppressed callback via SuppressAlarm command
    {
        delegate.Reset();
        auto config = DefaultConfig().AddAlarmsSupported(AlarmModeBitmap::kVisual);
        BooleanStateConfigurationCluster cluster(
            kTestEndpointId, { Feature::kVisual, Feature::kAlarmSuppress },
            { BooleanStateConfigurationCluster::OptionalAttributesSet().Set<Attributes::AlarmsEnabled::Id>() }, config);
        cluster.SetDelegate(&delegate);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        // Enable and activate alarms first
        Commands::EnableDisableAlarm::Type enableRequest;
        enableRequest.alarmsToEnableDisable.Set(AlarmModeBitmap::kVisual);
        EXPECT_TRUE(tester.Invoke(enableRequest).IsSuccess());

        chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsToSet;
        alarmsToSet.Set(AlarmModeBitmap::kVisual);
        cluster.SetAlarmsActive(alarmsToSet);

        delegate.Reset();

        // Suppress alarms
        Commands::SuppressAlarm::Type suppressRequest;
        suppressRequest.alarmsToSuppress.Set(AlarmModeBitmap::kVisual);
        auto result = tester.Invoke(suppressRequest);
        EXPECT_TRUE(result.IsSuccess());

        // Verify callback was called
        EXPECT_TRUE(delegate.WasAlarmsSuppressedCalled());
        EXPECT_TRUE(delegate.GetAlarmsSuppressedValue().Has(AlarmModeBitmap::kVisual));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

} // namespace
