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

#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
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
using namespace chip::Test;

class DummyDelegate : public Delegate
{
public:
    DummyDelegate() {};
    ~DummyDelegate() override = default;

    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override
    {
        return DataModel::Nullable<chip::Percent>();
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

    chip::Test::TestServerClusterContext testContext;
    DummyDelegate delegate;
    DummyTimeSyncTracker timeSyncTracker;
};

TEST_F(TestValveConfigurationAndControlCluster, AttributeTest)
{
    // Test 1: Base configuration with no features or optional attributes
    {
        const BitFlags<Feature> features{ 0U };
        ValveConfigurationAndControlCluster valveCluster(
            kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(valveCluster.Attributes(ConcreteClusterPath(kRootEndpointId, ValveConfigurationAndControl::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        valveCluster.Shutdown();
    }

    // Test 2: With optional ValveFault attribute
    {
        const DataModel::AttributeEntry optionalAttributes[] = { ValveFault::kMetadataEntry };
        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<ValveFault::Id>();
        const BitFlags<Feature> features{ 0U };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(valveCluster.Attributes(ConcreteClusterPath(kRootEndpointId, ValveConfigurationAndControl::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        valveCluster.Shutdown();
    }

    // Test 3: With Level feature (adds CurrentLevel, TargetLevel, and LevelStep attributes)
    {
        const AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = { { true, CurrentLevel::kMetadataEntry },
                                                                                          { true, TargetLevel::kMetadataEntry },
                                                                                          { true, LevelStep::kMetadataEntry } };

        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<LevelStep::Id>();
        const BitFlags<Feature> features{ Feature::kLevel };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(valveCluster.Attributes(ConcreteClusterPath(kRootEndpointId, ValveConfigurationAndControl::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributeEntries)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        valveCluster.Shutdown();
    }

    // Test 4: With optional DefaultOpenLevel attribute enabled.
    {
        const AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = { { true, CurrentLevel::kMetadataEntry },
                                                                                          { true, TargetLevel::kMetadataEntry },
                                                                                          { true,
                                                                                            DefaultOpenLevel::kMetadataEntry } };

        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<DefaultOpenLevel::Id>();
        const BitFlags<Feature> features{ Feature::kLevel };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(valveCluster.Attributes(ConcreteClusterPath(kRootEndpointId, ValveConfigurationAndControl::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributeEntries)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        valveCluster.Shutdown();
    }

    // Test 5: With TimeSync feature (adds AutoCloseTime attribute)
    {
        const AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = { { true, AutoCloseTime::kMetadataEntry } };
        const BitFlags<Feature> features{ Feature::kTimeSync };
        ValveConfigurationAndControlCluster valveCluster(
            kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(valveCluster.Attributes(ConcreteClusterPath(kRootEndpointId, ValveConfigurationAndControl::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributeEntries)), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        valveCluster.Shutdown();
    }
}

TEST_F(TestValveConfigurationAndControlCluster, ReadAttributeTest)
{
    // Test 1: Reading mandatory attributes
    {
        const BitFlags<Feature> features{ 0U };
        ValveConfigurationAndControlCluster valveCluster(
            kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(valveCluster);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
        ASSERT_EQ(revision, kRevision);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        OpenDuration::TypeInfo::DecodableType openDuration{};
        ASSERT_EQ(tester.ReadAttribute(OpenDuration::Id, openDuration), CHIP_NO_ERROR);

        DefaultOpenDuration::TypeInfo::DecodableType defaultOpenDuration{};
        ASSERT_EQ(tester.ReadAttribute(DefaultOpenDuration::Id, defaultOpenDuration), CHIP_NO_ERROR);

        RemainingDuration::TypeInfo::DecodableType remainingDuration{};
        ASSERT_EQ(tester.ReadAttribute(RemainingDuration::Id, remainingDuration), CHIP_NO_ERROR);

        CurrentState::TypeInfo::DecodableType currentState{};
        ASSERT_EQ(tester.ReadAttribute(CurrentState::Id, currentState), CHIP_NO_ERROR);

        TargetState::TypeInfo::DecodableType targetState{};
        ASSERT_EQ(tester.ReadAttribute(TargetState::Id, targetState), CHIP_NO_ERROR);

        valveCluster.Shutdown();
    }

    // Test 2: Reading with optional DefaultOpenLevel attribute
    {
        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<DefaultOpenLevel::Id>();
        const BitFlags<Feature> features{ 0U };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(valveCluster);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        chip::Percent defaultOpenLevel{};
        ASSERT_EQ(tester.ReadAttribute(DefaultOpenLevel::Id, defaultOpenLevel), CHIP_NO_ERROR);

        valveCluster.Shutdown();
    }

    // Test 3: Reading with optional ValveFault attribute
    {
        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<ValveFault::Id>();
        const BitFlags<Feature> features{ 0U };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(valveCluster);

        BitMask<ValveFaultBitmap> valveFault{};
        ASSERT_EQ(tester.ReadAttribute(ValveFault::Id, valveFault), CHIP_NO_ERROR);

        valveCluster.Shutdown();
    }

    // Test 4: Reading with Level feature
    {
        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<LevelStep::Id>();
        const BitFlags<Feature> features{ Feature::kLevel };
        ValveConfigurationAndControlCluster valveCluster(kRootEndpointId, features, optionalAttributeSet, &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(valveCluster);

        CurrentLevel::TypeInfo::DecodableType currentLevel{};
        ASSERT_EQ(tester.ReadAttribute(CurrentLevel::Id, currentLevel), CHIP_NO_ERROR);

        TargetLevel::TypeInfo::DecodableType targetLevel{};
        ASSERT_EQ(tester.ReadAttribute(TargetLevel::Id, targetLevel), CHIP_NO_ERROR);

        uint8_t levelStep{};
        ASSERT_EQ(tester.ReadAttribute(LevelStep::Id, levelStep), CHIP_NO_ERROR);

        valveCluster.Shutdown();
    }

    // Test 5: Reading with TimeSync feature
    {
        const BitFlags<Feature> features{ Feature::kTimeSync };
        ValveConfigurationAndControlCluster valveCluster(
            kRootEndpointId, features, ValveConfigurationAndControlCluster::OptionalAttributeSet(), &timeSyncTracker);
        valveCluster.SetDelegate(&delegate);
        ASSERT_EQ(valveCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(valveCluster);

        AutoCloseTime::TypeInfo::DecodableType autoCloseTime{};
        ASSERT_EQ(tester.ReadAttribute(AutoCloseTime::Id, autoCloseTime), CHIP_NO_ERROR);

        valveCluster.Shutdown();
    }
}

} // namespace
