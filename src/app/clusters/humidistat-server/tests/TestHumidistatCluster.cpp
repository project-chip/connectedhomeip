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

#include <optional>

#include <app/clusters/humidistat-server/HumidistatCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Humidistat/Commands.h>
#include <clusters/Humidistat/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Humidistat;
using namespace chip::app::Clusters::Humidistat::Attributes;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

using CSC = ClusterStatusCode;

struct TestHumidistatCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

constexpr EndpointId kTestEndpointId = 1;

TEST_F(TestHumidistatCluster, AttributeList)
{
    // No features, no optional attributes enabled - only required attributes should be present.
    {
        HumidistatCluster cluster(kTestEndpointId, {}, {});

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                            }));
    }

    // Supports only Humidifier mode with sensor - only attributes relevant to that mode should be present, including optional
    // TargetSetpoint.
    {
        const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor };

        HumidistatCluster::OptionalAttributeSet optionalAttrs;
        optionalAttrs.Set<TargetSetpoint::Id>();

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                UserSetpoint::kMetadataEntry,
                                                MinSetpoint::kMetadataEntry,
                                                MaxSetpoint::kMetadataEntry,
                                                Step::kMetadataEntry,
                                                TargetSetpoint::kMetadataEntry,
                                                MistType::kMetadataEntry,
                                            }));
    }

    // Support sensor without humidifier - only attributes relevant to sensor should be present, no MistType.
    {
        const BitFlags<Feature> features{ Feature::kSensor };

        HumidistatCluster::OptionalAttributeSet optionalAttrs;
        optionalAttrs.Set<TargetSetpoint::Id>();

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                UserSetpoint::kMetadataEntry,
                                                MinSetpoint::kMetadataEntry,
                                                MaxSetpoint::kMetadataEntry,
                                                Step::kMetadataEntry,
                                                TargetSetpoint::kMetadataEntry,
                                            }));
    }

    // Support optimal with sensor - only attributes relevant to optimal should be present, including optional TargetSetpoint.
    {
        const BitFlags<Feature> features{ Feature::kOptimal, Feature::kSensor };

        HumidistatCluster::OptionalAttributeSet optionalAttrs;
        optionalAttrs.Set<TargetSetpoint::Id>();

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                UserSetpoint::kMetadataEntry,
                                                MinSetpoint::kMetadataEntry,
                                                MaxSetpoint::kMetadataEntry,
                                                Step::kMetadataEntry,
                                                TargetSetpoint::kMetadataEntry,
                                                Optimal::kMetadataEntry,
                                            }));
    }

    // Support sensor alone without optional TargetSetpoint - only attributes relevant to sensor should be present, excluding
    // optional TargetSetpoint.
    {
        const BitFlags<Feature> features{ Feature::kSensor };

        HumidistatCluster cluster(kTestEndpointId, features, {});

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                UserSetpoint::kMetadataEntry,
                                                MinSetpoint::kMetadataEntry,
                                                MaxSetpoint::kMetadataEntry,
                                                Step::kMetadataEntry,
                                            }));
    }

    // Support continuous feature - only attributes relevant to continuous should be present.
    {
        const BitFlags<Feature> features{ Feature::kContinuous };

        HumidistatCluster::OptionalAttributeSet optionalAttrs;

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                Continuous::kMetadataEntry,
                                            }));
    }

    // Support sleep alone - only attributes relevant to sleep should be present.
    {
        const BitFlags<Feature> features;

        HumidistatCluster::OptionalAttributeSet optionalAttrs;
        optionalAttrs.Set<Sleep::Id>();

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                Sleep::kMetadataEntry,
                                            }));
    }

    // All features enabled, all optional attributes enabled - all attributes should be present.

    {
        const BitFlags<Feature> features{ Feature::kSensor, Feature::kHumidifier, Feature::kContinuous, Feature::kOptimal };

        HumidistatCluster::OptionalAttributeSet optionalAttrs;
        optionalAttrs.Set<Sleep::Id>().Set<TargetSetpoint::Id>();

        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Mode::kMetadataEntry,
                                                SystemState::kMetadataEntry,
                                                UserSetpoint::kMetadataEntry,
                                                MinSetpoint::kMetadataEntry,
                                                MaxSetpoint::kMetadataEntry,
                                                Step::kMetadataEntry,
                                                TargetSetpoint::kMetadataEntry,
                                                MistType::kMetadataEntry,
                                                Continuous::kMetadataEntry,
                                                Sleep::kMetadataEntry,
                                                Optimal::kMetadataEntry,
                                            }));
    }
}

TEST_F(TestHumidistatCluster, AcceptedCommandList)
{
    HumidistatCluster cluster(kTestEndpointId, {}, {});

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::SetSettings::kMetadataEntry,
                                              }));
}

TEST_F(TestHumidistatCluster, ReadAttributes)
{
    HumidistatCluster::StartupConfiguration config;
    config.systemState    = SystemStateEnum::kHumidifying;
    config.mode           = ModeEnum::kHumidifier;
    config.userSetpoint   = 45;
    config.minSetpoint    = 10;
    config.maxSetpoint    = 80;
    config.step           = 5;
    config.targetSetpoint = 50;
    config.mistType       = chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold, MistTypeBitmap::kMistWarm);
    config.continuous     = true;
    config.sleep          = true;
    config.optimal        = false;

    const BitFlags<Feature> features{ Feature::kSensor,  Feature::kHumidifier, Feature::kContinuous,
                                      Feature::kOptimal, Feature::kColdMist,   Feature::kWarmMist };

    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>().Set<TargetSetpoint::Id>();

    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ModeEnum mode{};
    ASSERT_EQ(tester.ReadAttribute(Mode::Id, mode), CHIP_NO_ERROR);
    ASSERT_EQ(mode, ModeEnum::kHumidifier);

    SystemStateEnum state{};
    ASSERT_EQ(tester.ReadAttribute(SystemState::Id, state), CHIP_NO_ERROR);
    ASSERT_EQ(state, SystemStateEnum::kHumidifying);

    chip::Percent userSetpoint{};
    ASSERT_EQ(tester.ReadAttribute(UserSetpoint::Id, userSetpoint), CHIP_NO_ERROR);
    ASSERT_EQ(userSetpoint, config.userSetpoint);

    chip::Percent minSetpoint{};
    ASSERT_EQ(tester.ReadAttribute(MinSetpoint::Id, minSetpoint), CHIP_NO_ERROR);
    ASSERT_EQ(minSetpoint, config.minSetpoint);

    chip::Percent maxSetpoint{};
    ASSERT_EQ(tester.ReadAttribute(MaxSetpoint::Id, maxSetpoint), CHIP_NO_ERROR);
    ASSERT_EQ(maxSetpoint, config.maxSetpoint);

    chip::Percent step{};
    ASSERT_EQ(tester.ReadAttribute(Step::Id, step), CHIP_NO_ERROR);
    ASSERT_EQ(step, config.step);

    chip::Percent targetSetpoint{};
    ASSERT_EQ(tester.ReadAttribute(TargetSetpoint::Id, targetSetpoint), CHIP_NO_ERROR);
    ASSERT_EQ(targetSetpoint, config.targetSetpoint);

    chip::BitMask<MistTypeBitmap> mistType;
    ASSERT_EQ(tester.ReadAttribute(MistType::Id, mistType), CHIP_NO_ERROR);
    ASSERT_EQ(mistType.Raw(), config.mistType.Raw());

    bool continuous = false;
    ASSERT_EQ(tester.ReadAttribute(Continuous::Id, continuous), CHIP_NO_ERROR);
    ASSERT_EQ(continuous, config.continuous);

    bool sleep = false;
    ASSERT_EQ(tester.ReadAttribute(Sleep::Id, sleep), CHIP_NO_ERROR);
    ASSERT_EQ(sleep, config.sleep);

    bool optimal = true;
    ASSERT_EQ(tester.ReadAttribute(Optimal::Id, optimal), CHIP_NO_ERROR);
    ASSERT_EQ(optimal, config.optimal);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsMode)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kDehumidifier, Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set mode to Humidifier
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kHumidifier);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetMode(), ModeEnum::kHumidifier);
        EXPECT_TRUE(tester.IsAttributeDirty(Mode::Id));
    }

    tester.GetDirtyList().clear();

    // Set mode to Dehumidifier
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kDehumidifier);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetMode(), ModeEnum::kDehumidifier);
        EXPECT_TRUE(tester.IsAttributeDirty(Mode::Id));
    }

    tester.GetDirtyList().clear();

    // Set mode to unsupported value (Auto not enabled) — expect ConstraintError
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kAuto);
        auto result = tester.Invoke(request);
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::ConstraintError)));
        EXPECT_EQ(cluster.GetMode(), ModeEnum::kDehumidifier); // unchanged
    }

    tester.GetDirtyList().clear();

    // No-op: set same mode again — no dirty notification
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kDehumidifier);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_FALSE(tester.IsAttributeDirty(Mode::Id));
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsUserSetpoint)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint  = 20;
    config.maxSetpoint  = 80;
    config.step         = 10;
    config.userSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {}, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Set step-aligned value
    {
        Commands::SetSettings::Type request;
        request.userSetpoint.SetValue(60);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetUserSetpoint(), 60);
    }

    tester.GetDirtyList().clear();

    // Snap-to-nearest: 53 rounds to 50 (closer to 50 than 60)
    {
        Commands::SetSettings::Type request;
        request.userSetpoint.SetValue(53);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetUserSetpoint(), 50);
    }

    tester.GetDirtyList().clear();

    // Snap-to-nearest: 56 rounds to 60 (closer to 60 than 50)
    {
        Commands::SetSettings::Type request;
        request.userSetpoint.SetValue(56);
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetUserSetpoint(), 60);
    }

    tester.GetDirtyList().clear();

    // Out of range — expect ConstraintError
    {
        Commands::SetSettings::Type request;
        request.userSetpoint.SetValue(90);
        auto result = tester.Invoke(request);
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::ConstraintError)));
        EXPECT_EQ(cluster.GetUserSetpoint(), 60); // unchanged
    }

    // Below range — expect ConstraintError
    {
        Commands::SetSettings::Type request;
        request.userSetpoint.SetValue(10);
        auto result = tester.Invoke(request);
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::ConstraintError)));
        EXPECT_EQ(cluster.GetUserSetpoint(), 60); // unchanged
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsUnsupportedFieldsIgnored)
{
    // Cluster with NO optional features — all optional fields must be silently ignored.
    HumidistatCluster cluster(kTestEndpointId, {}, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetSettings::Type request;
    request.mistType.SetValue(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold));
    request.continuous.SetValue(true);
    request.sleep.SetValue(true);
    request.optimal.SetValue(true);
    request.userSetpoint.SetValue(42);

    auto result = tester.Invoke(request);
    EXPECT_TRUE(result.IsSuccess());

    // Nothing should have changed since no features are enabled.
    EXPECT_EQ(cluster.GetMode(), ModeEnum::kOff);
    EXPECT_FALSE(tester.IsAttributeDirty(MistType::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(Continuous::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(Sleep::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(Optimal::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(UserSetpoint::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsMultipleFields)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint  = 0;
    config.maxSetpoint  = 100;
    config.step         = 5;
    config.userSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor, Feature::kContinuous, Feature::kOptimal,
                                      Feature::kColdMist };

    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>();

    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set multiple fields in a single command
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kHumidifier);
        request.userSetpoint.SetValue(65);
        request.mistType.SetValue(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold));
        request.continuous.SetValue(true);
        request.sleep.SetValue(true);
        request.optimal.SetValue(true);

        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());

        EXPECT_EQ(cluster.GetMode(), ModeEnum::kHumidifier);
        EXPECT_EQ(cluster.GetUserSetpoint(), 65);
        EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());
        EXPECT_TRUE(cluster.GetContinuous());
        EXPECT_TRUE(cluster.GetSleep());
        EXPECT_TRUE(cluster.GetOptimal());

        EXPECT_TRUE(tester.IsAttributeDirty(Mode::Id));
        EXPECT_TRUE(tester.IsAttributeDirty(UserSetpoint::Id));
        EXPECT_TRUE(tester.IsAttributeDirty(MistType::Id));
        EXPECT_TRUE(tester.IsAttributeDirty(Continuous::Id));
        EXPECT_TRUE(tester.IsAttributeDirty(Sleep::Id));
        EXPECT_TRUE(tester.IsAttributeDirty(Optimal::Id));
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsMistTypeValidation)
{
    // Cluster with HUM + ColdMist only (no WarmMist)
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kColdMist };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Setting ColdMist should succeed when mode is Humidifier
    {
        Commands::SetSettings::Type request;
        request.mode.SetValue(ModeEnum::kHumidifier);
        request.mistType.SetValue(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold));
        auto result = tester.Invoke(request);
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());
    }

    tester.GetDirtyList().clear();

    // Setting WarmMist should fail with InvalidInState (feature not supported)
    {
        Commands::SetSettings::Type request;
        request.mistType.SetValue(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistWarm));
        auto result = tester.Invoke(request);
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::InvalidInState)));
        // MistType unchanged
        EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());
    }

    // Setting an empty MistType while mode is Humidifier should fail.
    {
        Commands::SetSettings::Type request;
        request.mistType.SetValue(chip::BitMask<MistTypeBitmap>());
        auto result = tester.Invoke(request);
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::ConstraintError)));
        EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetMistTypeRequiresAtLeastOneBitInHumidifierMode)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kColdMist };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.SetMode(ModeEnum::kHumidifier), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetMistType(chip::BitMask<MistTypeBitmap>()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.GetMistType().Raw(), 0u);
    EXPECT_FALSE(tester.IsAttributeDirty(MistType::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsEmptyCommand)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Empty command with no fields set — should succeed with no changes
    Commands::SetSettings::Type request;
    auto result = tester.Invoke(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetSettingsModeFailStopsProcessing)
{
    // If Mode fails, subsequent fields should not be applied.
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint  = 0;
    config.maxSetpoint  = 100;
    config.step         = 1;
    config.userSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {}, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetSettings::Type request;
    request.mode.SetValue(ModeEnum::kAuto); // unsupported
    request.userSetpoint.SetValue(75);

    auto result = tester.Invoke(request);
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(result.GetStatusCode(), std::make_optional(CSC(Status::ConstraintError)));
    EXPECT_EQ(cluster.GetMode(), ModeEnum::kOff); // unchanged
    EXPECT_EQ(cluster.GetUserSetpoint(), 50);     // unchanged — not processed

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, TestPersistence)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kSensor, Feature::kContinuous, Feature::kOptimal,
                                      Feature::kColdMist };

    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>();

    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint  = 0;
    config.maxSetpoint  = 100;
    config.step         = 5;
    config.userSetpoint = 50;

    TestServerClusterContext context;

    // 1. Start cluster, set non-default values for all non-volatile attributes.
    {
        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.SetMode(ModeEnum::kHumidifier), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetSystemState(SystemStateEnum::kHumidifying), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetUserSetpoint(75), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetMistType(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold)), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetContinuous(true), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetSleep(true), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetOptimal(true), CHIP_NO_ERROR);
    }

    // 2. New cluster instance with same storage context — values should be loaded from persistence,
    //    overriding the default StartupConfiguration.
    {
        HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetMode(), ModeEnum::kHumidifier);
        EXPECT_EQ(cluster.GetSystemState(), SystemStateEnum::kHumidifying);
        EXPECT_EQ(cluster.GetUserSetpoint(), 75);
        EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());
        EXPECT_TRUE(cluster.GetContinuous());
        EXPECT_TRUE(cluster.GetSleep());
        EXPECT_TRUE(cluster.GetOptimal());
    }
}

TEST_F(TestHumidistatCluster, SetModeClearsMistType)
{
    // Spec: "If the value of Mode is not set to Humidifier, all bits of MistType SHALL be set to zero."
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kDehumidifier, Feature::kColdMist, Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set mode to Humidifier and set MistType
    ASSERT_EQ(cluster.SetMode(ModeEnum::kHumidifier), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetMistType(chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMistType().Raw(), chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold).Raw());

    tester.GetDirtyList().clear();

    // Change mode away from Humidifier — MistType must be cleared.
    ASSERT_EQ(cluster.SetMode(ModeEnum::kDehumidifier), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMistType().Raw(), 0u);
    EXPECT_TRUE(tester.IsAttributeDirty(MistType::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, SetTargetSetpoint)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 10;
    config.userSetpoint   = 50;
    config.targetSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<TargetSetpoint::Id>();
    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set a valid in-range target setpoint
    EXPECT_EQ(cluster.SetTargetSetpoint(60), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTargetSetpoint(), 60);
    EXPECT_TRUE(tester.IsAttributeDirty(TargetSetpoint::Id));

    tester.GetDirtyList().clear();

    // No-op: same value — no dirty notification
    EXPECT_EQ(cluster.SetTargetSetpoint(60), CHIP_NO_ERROR);
    EXPECT_FALSE(tester.IsAttributeDirty(TargetSetpoint::Id));

    // Out of range — expect ConstraintError
    EXPECT_EQ(cluster.SetTargetSetpoint(90), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.GetTargetSetpoint(), 60); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, UserSetpointSyncsTargetSetpointWhenSleepAndOptimalAreFalse)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 10;
    config.userSetpoint   = 50;
    config.targetSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>().Set<TargetSetpoint::Id>();
    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    EXPECT_EQ(cluster.SetUserSetpoint(60), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUserSetpoint(), 60);
    EXPECT_EQ(cluster.GetTargetSetpoint(), 60);
    EXPECT_TRUE(tester.IsAttributeDirty(UserSetpoint::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(TargetSetpoint::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, UserSetpointDoesNotSyncHiddenTargetSetpoint)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 10;
    config.userSetpoint   = 50;
    config.targetSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {}, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    EXPECT_EQ(cluster.SetUserSetpoint(60), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUserSetpoint(), 60);
    EXPECT_EQ(cluster.GetTargetSetpoint(), 50);
    EXPECT_TRUE(tester.IsAttributeDirty(UserSetpoint::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(TargetSetpoint::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, ClearingSleepSyncsTargetSetpointBackToUserSetpoint)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 10;
    config.userSetpoint   = 50;
    config.targetSetpoint = 70;
    config.sleep          = true;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>().Set<TargetSetpoint::Id>();
    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    EXPECT_EQ(cluster.SetSleep(false), CHIP_NO_ERROR);
    EXPECT_FALSE(cluster.GetSleep());
    EXPECT_EQ(cluster.GetTargetSetpoint(), cluster.GetUserSetpoint());
    EXPECT_TRUE(tester.IsAttributeDirty(Sleep::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(TargetSetpoint::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Fake delegate for testing callback behavior
class FakeHumidistatDelegate : public HumidistatDelegate
{
public:
    void OnModeChanged(Humidistat::ModeEnum newMode) override
    {
        modeChangedCount++;
        lastMode = newMode;
    }

    void OnSystemStateChanged(Humidistat::SystemStateEnum newSystemState) override
    {
        systemStateChangedCount++;
        lastSystemState = newSystemState;
    }

    void OnUserSetpointChanged(chip::Percent newUserSetpoint) override
    {
        userSetpointChangedCount++;
        lastUserSetpoint = newUserSetpoint;
    }

    void OnTargetSetpointChanged(chip::Percent newTargetSetpoint) override
    {
        targetSetpointChangedCount++;
        lastTargetSetpoint = newTargetSetpoint;
    }

    void OnMistTypeChanged(chip::BitMask<Humidistat::MistTypeBitmap> newMistType) override
    {
        mistTypeChangedCount++;
        lastMistType = newMistType;
    }

    void OnContinuousChanged(bool newContinuous) override
    {
        continuousChangedCount++;
        lastContinuous = newContinuous;
    }

    void OnSleepChanged(bool newSleep) override
    {
        sleepChangedCount++;
        lastSleep = newSleep;
    }

    void OnOptimalChanged(bool newOptimal) override
    {
        optimalChangedCount++;
        lastOptimal = newOptimal;
    }

    int modeChangedCount           = 0;
    int systemStateChangedCount    = 0;
    int userSetpointChangedCount   = 0;
    int targetSetpointChangedCount = 0;
    int mistTypeChangedCount       = 0;
    int continuousChangedCount     = 0;
    int sleepChangedCount          = 0;
    int optimalChangedCount        = 0;

    std::optional<Humidistat::ModeEnum> lastMode;
    std::optional<Humidistat::SystemStateEnum> lastSystemState;
    std::optional<chip::Percent> lastUserSetpoint;
    std::optional<chip::Percent> lastTargetSetpoint;
    std::optional<chip::BitMask<Humidistat::MistTypeBitmap>> lastMistType;
    std::optional<bool> lastContinuous;
    std::optional<bool> lastSleep;
    std::optional<bool> lastOptimal;
};

TEST_F(TestHumidistatCluster, DelegateCallback_OnModeChanged)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kDehumidifier };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetMode(ModeEnum::kHumidifier);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMode(), ModeEnum::kHumidifier);
    EXPECT_EQ(delegate.modeChangedCount, 1);
    EXPECT_EQ(delegate.lastMode, std::optional<ModeEnum>(ModeEnum::kHumidifier));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnSystemStateChanged)
{
    const BitFlags<Feature> features{ Feature::kHumidifier };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetSystemState(SystemStateEnum::kHumidifying);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetSystemState(), SystemStateEnum::kHumidifying);
    EXPECT_EQ(delegate.systemStateChangedCount, 1);
    EXPECT_EQ(delegate.lastSystemState, std::optional<SystemStateEnum>(SystemStateEnum::kHumidifying));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnUserSetpointChanged)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint  = 20;
    config.maxSetpoint  = 80;
    config.step         = 10;
    config.userSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster cluster(kTestEndpointId, features, {}, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetUserSetpoint(60);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUserSetpoint(), 60);
    EXPECT_EQ(delegate.userSetpointChangedCount, 1);
    EXPECT_EQ(delegate.lastUserSetpoint, std::optional<chip::Percent>(60));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnTargetSetpointChanged)
{
    HumidistatCluster::StartupConfiguration config;
    config.minSetpoint    = 20;
    config.maxSetpoint    = 80;
    config.step           = 10;
    config.userSetpoint   = 50;
    config.targetSetpoint = 50;

    const BitFlags<Feature> features{ Feature::kSensor };
    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<TargetSetpoint::Id>();
    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetTargetSetpoint(70);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTargetSetpoint(), 70);
    EXPECT_EQ(delegate.targetSetpointChangedCount, 1);
    EXPECT_EQ(delegate.lastTargetSetpoint, std::optional<chip::Percent>(70));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnMistTypeChanged)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kColdMist };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    // SetMode to Humidifier first so MistType can be set
    ASSERT_EQ(cluster.SetMode(ModeEnum::kHumidifier), CHIP_NO_ERROR);

    const auto expectedMistType = chip::BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold);

    CHIP_ERROR err = cluster.SetMistType(expectedMistType);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMistType().Raw(), expectedMistType.Raw());
    EXPECT_EQ(delegate.mistTypeChangedCount, 1);
    if (!delegate.lastMistType.has_value())
    {
        ADD_FAILURE() << "Expected lastMistType to be set";
    }
    else
    {
        EXPECT_EQ(delegate.lastMistType->Raw(), expectedMistType.Raw());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnContinuousChanged)
{
    const BitFlags<Feature> features{ Feature::kContinuous };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetContinuous(true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetContinuous());
    EXPECT_EQ(delegate.continuousChangedCount, 1);
    EXPECT_EQ(delegate.lastContinuous, std::optional<bool>(true));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnSleepChanged)
{
    const BitFlags<Feature> features{};
    HumidistatCluster::OptionalAttributeSet optionalAttrs;
    optionalAttrs.Set<Sleep::Id>();

    HumidistatCluster cluster(kTestEndpointId, features, optionalAttrs, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetSleep(true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetSleep());
    EXPECT_EQ(delegate.sleepChangedCount, 1);
    EXPECT_EQ(delegate.lastSleep, std::optional<bool>(true));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_OnOptimalChanged)
{
    const BitFlags<Feature> features{ Feature::kOptimal };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    CHIP_ERROR err = cluster.SetOptimal(true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetOptimal());
    EXPECT_EQ(delegate.optimalChangedCount, 1);
    EXPECT_EQ(delegate.lastOptimal, std::optional<bool>(true));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_NullptrSafe)
{
    const BitFlags<Feature> features{ Feature::kHumidifier };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    // No delegate set (implicitly nullptr) — should not crash
    CHIP_ERROR err = cluster.SetMode(ModeEnum::kHumidifier);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMode(), ModeEnum::kHumidifier);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_SetDelegateTwice)
{
    const BitFlags<Feature> features{ Feature::kHumidifier, Feature::kDehumidifier };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate1;
    FakeHumidistatDelegate delegate2;

    // Set first delegate
    cluster.SetDelegate(&delegate1);

    // Replace delegate with second one
    cluster.SetDelegate(&delegate2);

    // Only delegate2 should be called
    CHIP_ERROR err = cluster.SetMode(ModeEnum::kHumidifier);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(delegate1.modeChangedCount, 0);
    EXPECT_EQ(delegate2.modeChangedCount, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestHumidistatCluster, DelegateCallback_NoCallOnNoChange)
{
    const BitFlags<Feature> features{ Feature::kHumidifier };
    HumidistatCluster cluster(kTestEndpointId, features, {});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    FakeHumidistatDelegate delegate;
    cluster.SetDelegate(&delegate);

    // First call: value changes
    CHIP_ERROR err1 = cluster.SetMode(ModeEnum::kHumidifier);
    EXPECT_EQ(err1, CHIP_NO_ERROR);

    // Second call: no change (no-op)
    CHIP_ERROR err2 = cluster.SetMode(ModeEnum::kHumidifier);
    EXPECT_EQ(err2, CHIP_NO_ERROR);
    EXPECT_EQ(delegate.modeChangedCount, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
