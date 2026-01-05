/*
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

#include <pw_unit_test/framework.h>

#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ValveConfigurationAndControl/Attributes.h>
#include <clusters/ValveConfigurationAndControl/Commands.h>
#include <clusters/ValveConfigurationAndControl/Events.h>
#include <clusters/ValveConfigurationAndControl/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
using namespace chip::app::Clusters::ValveConfigurationAndControl::Attributes;
using namespace chip::Testing;

class DummyDelegate : public Delegate
{
public:
    DummyDelegate(){};
    ~DummyDelegate() override = default;

    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override
    {
        return DataModel::Nullable<chip::Percent>();
    }

    CHIP_ERROR HandleCloseValve() override { return CHIP_NO_ERROR; }

    void HandleRemainingDurationTick(uint32_t duration) override {}
};

class InstantDelegate : public Delegate
{
public:
    InstantDelegate(){};
    ~InstantDelegate() override = default;

    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override
    {
        return DataModel::Nullable<chip::Percent>(100);
    }

    CHIP_ERROR HandleCloseValve() override { return CHIP_NO_ERROR; }

    void HandleRemainingDurationTick(uint32_t duration) override {}
};
class DummyTimeSyncTracker : public TimeSyncTracker
{
public:
    bool IsTimeSyncClusterSupported() override { return true; }
    bool IsValidUTCTime() override { return false; }
};

struct TestValveConfigurationAndControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestValveConfigurationAndControlCluster() {}

    TestServerClusterContext testContext;
    DummyDelegate delegate;
    InstantDelegate instantDelegate;
    DummyTimeSyncTracker timeSyncTracker;
};

// Base configuration with no features or optional attributes
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestEmptyOptional)
{
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, {}, {}, config, &timeSyncTracker);

    ASSERT_TRUE(
        IsAttributesListEqualTo(valveCluster,
                                { OpenDuration::kMetadataEntry, DefaultOpenDuration::kMetadataEntry,
                                  RemainingDuration::kMetadataEntry, CurrentState::kMetadataEntry, TargetState::kMetadataEntry }));
}

// With optional ValveFault attribute
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestValveFault)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<ValveFault::Id>();
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, {}, optionalAttributeSet, config, &timeSyncTracker);

    ASSERT_TRUE(IsAttributesListEqualTo(valveCluster,
                                        { OpenDuration::kMetadataEntry, DefaultOpenDuration::kMetadataEntry,
                                          RemainingDuration::kMetadataEntry, CurrentState::kMetadataEntry,
                                          TargetState::kMetadataEntry, ValveFault::kMetadataEntry }));
}

// With Level feature (adds CurrentLevel, TargetLevel, and LevelStep attributes)
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestLevelStep)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<LevelStep::Id>();
    const BitFlags<Feature> features{ Feature::kLevel };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);

    ASSERT_TRUE(
        IsAttributesListEqualTo(valveCluster,
                                { OpenDuration::kMetadataEntry, DefaultOpenDuration::kMetadataEntry,
                                  RemainingDuration::kMetadataEntry, CurrentState::kMetadataEntry, TargetState::kMetadataEntry,
                                  CurrentLevel::kMetadataEntry, TargetLevel::kMetadataEntry, LevelStep::kMetadataEntry }));
}

// With optional DefaultOpenLevel attribute enabled.
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestDefaultOpenLevel)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<DefaultOpenLevel::Id>();
    const BitFlags<Feature> features{ Feature::kLevel };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);

    ASSERT_TRUE(
        IsAttributesListEqualTo(valveCluster,
                                { OpenDuration::kMetadataEntry, DefaultOpenDuration::kMetadataEntry,
                                  RemainingDuration::kMetadataEntry, CurrentState::kMetadataEntry, TargetState::kMetadataEntry,
                                  CurrentLevel::kMetadataEntry, TargetLevel::kMetadataEntry, DefaultOpenLevel::kMetadataEntry }));
}

// With TimeSync feature (adds AutoCloseTime attribute)
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestTimeSync)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<ValveFault::Id>().Set<DefaultOpenLevel::Id>().Set<LevelStep::Id>();
    const BitFlags<Feature> features{ Feature::kTimeSync, Feature::kLevel };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);
    ASSERT_TRUE(
        IsAttributesListEqualTo(valveCluster,
                                { OpenDuration::kMetadataEntry, DefaultOpenDuration::kMetadataEntry, AutoCloseTime::kMetadataEntry,
                                  RemainingDuration::kMetadataEntry, CurrentState::kMetadataEntry, TargetState::kMetadataEntry,
                                  CurrentLevel::kMetadataEntry, TargetLevel::kMetadataEntry, DefaultOpenLevel::kMetadataEntry,
                                  ValveFault::kMetadataEntry, LevelStep::kMetadataEntry }));
}

// With all the optional and feature attributes enabled.
TEST_F(TestValveConfigurationAndControlCluster, AttributeTestAll)
{
    const BitFlags<Feature> features{ Feature::kTimeSync };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, {}, config, &timeSyncTracker);
    ASSERT_TRUE(IsAttributesListEqualTo(valveCluster,
                                        {
                                            OpenDuration::kMetadataEntry,
                                            DefaultOpenDuration::kMetadataEntry,
                                            AutoCloseTime::kMetadataEntry,
                                            RemainingDuration::kMetadataEntry,
                                            CurrentState::kMetadataEntry,
                                            TargetState::kMetadataEntry,
                                        }));
}

// Reading mandatory attributes
TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTestMandatory)
{
    const BitFlags<Feature> features;
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(
        kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    ASSERT_EQ(revision, kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    OpenDuration::TypeInfo::DecodableType openDuration;
    ASSERT_EQ(tester.ReadAttribute(OpenDuration::Id, openDuration), CHIP_NO_ERROR);

    DefaultOpenDuration::TypeInfo::DecodableType defaultOpenDuration;
    ASSERT_EQ(tester.ReadAttribute(DefaultOpenDuration::Id, defaultOpenDuration), CHIP_NO_ERROR);

    RemainingDuration::TypeInfo::DecodableType remainingDuration;
    ASSERT_EQ(tester.ReadAttribute(RemainingDuration::Id, remainingDuration), CHIP_NO_ERROR);

    CurrentState::TypeInfo::DecodableType currentState;
    ASSERT_EQ(tester.ReadAttribute(CurrentState::Id, currentState), CHIP_NO_ERROR);

    TargetState::TypeInfo::DecodableType targetState;
    ASSERT_EQ(tester.ReadAttribute(TargetState::Id, targetState), CHIP_NO_ERROR);

    valveCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reading with optional DefaultOpenLevel attribute
TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTestDefaultOpenLevel)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<DefaultOpenLevel::Id>();
    const BitFlags<Feature> features;
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    chip::Percent defaultOpenLevel{};
    ASSERT_EQ(tester.ReadAttribute(DefaultOpenLevel::Id, defaultOpenLevel), CHIP_NO_ERROR);

    valveCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reading with optional ValveFault attribute
TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTestValveFault)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<ValveFault::Id>();
    const BitFlags<Feature> features;
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    BitMask<ValveFaultBitmap> valveFault;
    ASSERT_EQ(tester.ReadAttribute(ValveFault::Id, valveFault), CHIP_NO_ERROR);

    valveCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reading with Level feature
TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTestLevel)
{
    ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
    optionalAttributeSet.Set<LevelStep::Id>();
    const BitFlags<Feature> features{ Feature::kLevel };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    CurrentLevel::TypeInfo::DecodableType currentLevel;
    ASSERT_EQ(tester.ReadAttribute(CurrentLevel::Id, currentLevel), CHIP_NO_ERROR);

    TargetLevel::TypeInfo::DecodableType targetLevel;
    ASSERT_EQ(tester.ReadAttribute(TargetLevel::Id, targetLevel), CHIP_NO_ERROR);

    uint8_t levelStep{};
    ASSERT_EQ(tester.ReadAttribute(LevelStep::Id, levelStep), CHIP_NO_ERROR);

    valveCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Reading with TimeSync feature
TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTestTimeSync)
{
    const BitFlags<Feature> features{ Feature::kTimeSync };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(
        kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    AutoCloseTime::TypeInfo::DecodableType autoCloseTime;
    ASSERT_EQ(tester.ReadAttribute(AutoCloseTime::Id, autoCloseTime), CHIP_NO_ERROR);

    valveCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Test Open command in a no-Level Valve with a DummyDelegate (non instant change).
TEST_F(TestValveConfigurationAndControlCluster, OpenCommandWithoutFault)
{
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(
        kRootEndpointId, {}, ValveConfigurationAndControlCluster::OptionalAttributeSet(), config, &timeSyncTracker);
    valveCluster.SetDelegate(&delegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    Commands::Open::Type request;
    request.openDuration = Optional<uint32_t>::Missing();
    request.targetLevel = Optional<Percent>::Missing();

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    // CurrentState should have changed to Transitioning
    CurrentState::TypeInfo::DecodableType currentState;
    EXPECT_EQ(tester.ReadAttribute(CurrentState::Id, currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, ValveStateEnum::kTransitioning);
    // TargetState should have changed to Open
    TargetState::TypeInfo::DecodableType targetState;
    EXPECT_EQ(tester.ReadAttribute(TargetState::Id, targetState), CHIP_NO_ERROR);
    EXPECT_EQ(targetState, ValveStateEnum::kOpen);
    // OpenDuration should be equal to DefaultOpenDuration since it wasn't provided in the command
    DefaultOpenDuration::TypeInfo::DecodableType defaultOpenDuration;
    EXPECT_EQ(tester.ReadAttribute(DefaultOpenDuration::Id, defaultOpenDuration), CHIP_NO_ERROR);
    OpenDuration::TypeInfo::DecodableType openDuration;
    EXPECT_EQ(tester.ReadAttribute(OpenDuration::Id, openDuration), CHIP_NO_ERROR);
    EXPECT_EQ(defaultOpenDuration, openDuration);
    // RemaniningDuration should be set to the same value as OpenDuration if read after setting it.
    RemainingDuration::TypeInfo::DecodableType remainingDuration;
    EXPECT_EQ(tester.ReadAttribute(RemainingDuration::Id, remainingDuration), CHIP_NO_ERROR);
    EXPECT_EQ(remainingDuration, openDuration);
}

// Test Close command in a Level Valve with an InstantDelegate (instant change).
TEST_F(TestValveConfigurationAndControlCluster, CloseCommandWithoutFault)
{
    const BitFlags<Feature> features{ Feature::kLevel };
    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };
    ValveConfigurationAndControlCluster valveCluster(
        kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), config, &timeSyncTracker);
    valveCluster.SetDelegate(&instantDelegate);
    ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(valveCluster);

    // First we need to send an Open command to change the status
    Commands::Open::Type openRequest;
    openRequest.openDuration = Optional<uint32_t>::Missing();
    openRequest.targetLevel = Optional<Percent>::Missing();

    auto result = tester.Invoke(openRequest);
    ASSERT_TRUE(result.IsSuccess());

    // CurrentState should have changed to Open
    CurrentState::TypeInfo::DecodableType currentState;
    EXPECT_EQ(tester.ReadAttribute(CurrentState::Id, currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, ValveStateEnum::kOpen);
    // TargetState should have changed to null
    TargetState::TypeInfo::DecodableType targetState;
    EXPECT_EQ(tester.ReadAttribute(TargetState::Id, targetState), CHIP_NO_ERROR);
    EXPECT_EQ(targetState, DataModel::NullNullable);
    // OpenDuration should be equal to DefaultOpenDuration since it wasn't provided in the command
    DefaultOpenDuration::TypeInfo::DecodableType defaultOpenDuration;
    EXPECT_EQ(tester.ReadAttribute(DefaultOpenDuration::Id, defaultOpenDuration), CHIP_NO_ERROR);
    OpenDuration::TypeInfo::DecodableType openDuration;
    EXPECT_EQ(tester.ReadAttribute(OpenDuration::Id, openDuration), CHIP_NO_ERROR);
    EXPECT_EQ(defaultOpenDuration, openDuration);
    // RemaniningDuration should be set to the same value as OpenDuration if read after setting it.
    RemainingDuration::TypeInfo::DecodableType remainingDuration;
    EXPECT_EQ(tester.ReadAttribute(RemainingDuration::Id, remainingDuration), CHIP_NO_ERROR);
    EXPECT_EQ(remainingDuration, openDuration);

    // Then we send the Close command.
    Commands::Close::Type closeRequest;

    result = tester.Invoke(closeRequest);
    ASSERT_TRUE(result.IsSuccess());

    // CurrentState should have changed to Transitioning
    EXPECT_EQ(tester.ReadAttribute(CurrentState::Id, currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, ValveStateEnum::kTransitioning);
    // TargetState should have changed to Close
    EXPECT_EQ(tester.ReadAttribute(TargetState::Id, targetState), CHIP_NO_ERROR);
    EXPECT_EQ(targetState, ValveStateEnum::kClosed);
    // OpenDuration should be null
    EXPECT_EQ(tester.ReadAttribute(OpenDuration::Id, openDuration), CHIP_NO_ERROR);
    EXPECT_EQ(defaultOpenDuration, DataModel::NullNullable);
    // RemaniningDuration should be set to null.
    EXPECT_EQ(tester.ReadAttribute(RemainingDuration::Id, remainingDuration), CHIP_NO_ERROR);
    EXPECT_EQ(remainingDuration, DataModel::NullNullable);
}

} // namespace
