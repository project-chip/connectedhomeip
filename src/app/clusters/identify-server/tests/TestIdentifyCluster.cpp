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

#include "ClusterActions.h"
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Identify/Attributes.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Metadata.h>
#include <lib/support/TimerDelegateMock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace chip::app::Clusters::Identify::Attributes;

namespace {

constexpr EndpointId kTestEndpointId = 1;

bool onIdentifyStartCalled    = false;
bool onIdentifyStopCalled     = false;
bool onEffectIdentifierCalled = false;

class TestIdentifyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override { onIdentifyStartCalled = true; }
    void OnIdentifyStop(IdentifyCluster & cluster) override { onIdentifyStopCalled = true; }
    void OnTriggerEffect(IdentifyCluster & cluster) override { onEffectIdentifierCalled = true; }
    bool IsTriggerEffectEnabled() const override { return true; }
};

TestIdentifyDelegate gTestIdentifyDelegate;

struct TestIdentifyCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        onIdentifyStartCalled    = false;
        onIdentifyStopCalled     = false;
        onEffectIdentifierCalled = false;
    }

    TimerDelegateMock mMockTimerDelegate;
};

TEST_F(TestIdentifyCluster, TestCreate)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestIdentifyCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, Identify::Id), attributes), CHIP_NO_ERROR);

    const DataModel::AttributeEntry expectedAttributes[] = {
        Attributes::IdentifyTime::kMetadataEntry,
        Attributes::IdentifyType::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedAttributes), {}), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestIdentifyCluster, AcceptedCommandsMandatoryOnlyTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, Identify::Id), acceptedCommands), CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        Commands::Identify::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestIdentifyCluster, AcceptedCommandsWithOptionalTriggerEffectTest)
{
    // When passing in onEffectIdentifier callback, we're enabling the TriggerEffect command.
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    chip::Test::TestServerClusterContext context;
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, Identify::Id), acceptedCommands), CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        Commands::Identify::kMetadataEntry,
        Commands::TriggerEffect::kMetadataEntry, // Expects the optional command because we passed in onEffectIdentifier.
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestIdentifyCluster, ReadAttributesTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(
        IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithIdentifyType(IdentifyTypeEnum::kVisibleIndicator));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Read and verify IdentifyType
    uint8_t identifyType;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyType::Id, identifyType), CHIP_NO_ERROR);
    EXPECT_EQ(identifyType, (uint8_t) IdentifyTypeEnum::kVisibleIndicator);

    // Read and verify FeatureMap
    uint32_t featureMap;
    EXPECT_EQ(ReadNumericAttribute(cluster, FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    // Read and verify ClusterRevision
    uint16_t clusterRevision;
    EXPECT_EQ(ReadNumericAttribute(cluster, Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);

    // Read non-existent attribute
    uint32_t nonExistentAttribute;
    EXPECT_NE(ReadNumericAttribute(cluster, 0xFFFF, nonExistentAttribute), CHIP_NO_ERROR);
}

TEST_F(TestIdentifyCluster, WriteUnchangedIdentifyTimeDoesNotNotify)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & changeListener = context.ChangeListener();

    // Write a value to IdentifyTime and verify it was reported.
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, Identify::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, IdentifyTime::Id);

    // Write the same value again and verify that the attribute is not reported as changed.
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestIdentifyCluster, WriteReadOnlyAttributesReturnUnsupportedWriteTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Attempt to write to the read-only IdentifyType attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyType::Id, (uint8_t) 0), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    // Attempt to write to the read-only ClusterRevision attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, Globals::Attributes::ClusterRevision::Id, static_cast<uint16_t>(0)),
              CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    // Attempt to write to the read-only FeatureMap attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, FeatureMap::Id, static_cast<uint32_t>(0)), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
}

TEST_F(TestIdentifyCluster, IdentifyTimeCountdownTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    uint16_t identifyTime;
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(5)), CHIP_NO_ERROR);

    // Advance the clock 1 second at a time and check the attribute value.
    for (uint16_t i = 0; i < 5; ++i)
    {
        mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
        EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
        EXPECT_EQ(identifyTime, 4u - i);
    }

    // The timer should not be active anymore.
    EXPECT_FALSE(mMockTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestIdentifyCluster, OnIdentifyStartStopCallbackTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Test onIdentifyStart callback.
    onIdentifyStartCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStartCalled);

    // Test onIdentifyStop callback.
    onIdentifyStopCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(0)), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStopCalled);
    // Test onIdentifyStop callback with timer.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(2)), CHIP_NO_ERROR);
    onIdentifyStopCalled = false;
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_FALSE(onIdentifyStopCalled);
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_TRUE(onIdentifyStopCalled);
}

TEST_F(TestIdentifyCluster, OnStartNotCalledMultipleTimes)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStartCalled);

    // Reset the flag and write a non-zero value again.
    onIdentifyStartCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(5)), CHIP_NO_ERROR);
    EXPECT_FALSE(onIdentifyStartCalled);
}

TEST_F(TestIdentifyCluster, OnStopNotCalledIfNotIdentifying)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    // Ensure we are not identifying.
    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);

    // Write 0 and check that the stop callback is not called.
    onIdentifyStopCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(0)), CHIP_NO_ERROR);
    EXPECT_FALSE(onIdentifyStopCalled);
}

TEST_F(TestIdentifyCluster, InvokeIdentifyCommandTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Test with a value of 10 for identifyTime.
    {
        constexpr uint16_t kIdentifyTime = 10;
        Commands::Identify::Type data;
        data.identifyTime = kIdentifyTime;

        auto result = InvokeCommand(cluster, Commands::Identify::Id, data);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);

        uint16_t identifyTime;
        EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
        EXPECT_EQ(identifyTime, kIdentifyTime);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Test with a specific effectIdentifier and effectVariant.
    {
        Commands::TriggerEffect::Type data;
        data.effectIdentifier = EffectIdentifierEnum::kBlink;
        data.effectVariant    = EffectVariantEnum::kDefault;

        onEffectIdentifierCalled = false;
        auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(onEffectIdentifierCalled);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandAllEffectsTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Test all effect identifiers.
    const EffectIdentifierEnum effectsToTest[] = { EffectIdentifierEnum::kBreathe, EffectIdentifierEnum::kOkay,
                                                   EffectIdentifierEnum::kChannelChange };

    for (const auto & effect : effectsToTest)
    {
        Commands::TriggerEffect::Type data;
        data.effectIdentifier = effect;
        data.effectVariant    = EffectVariantEnum::kDefault;

        onEffectIdentifierCalled = false;
        auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
                  Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(onEffectIdentifierCalled);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandInvalidVariantTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Test with an invalid effect variant.
    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = static_cast<EffectVariantEnum>(0xFF); // Invalid variant.

    onEffectIdentifierCalled = false;
    auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(onEffectIdentifierCalled);
}

TEST_F(TestIdentifyCluster, TriggerEffectWhileIdentifyingTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = EffectVariantEnum::kDefault;

    onEffectIdentifierCalled = false;
    auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(onEffectIdentifierCalled);

    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
}

TEST_F(TestIdentifyCluster, TriggerEffectFinishEffectTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kFinishEffect;
    data.effectVariant    = EffectVariantEnum::kDefault;

    auto result = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);

    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 1u);
}

TEST_F(TestIdentifyCluster, TriggerEffectStopEffectTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kStopEffect;
    data.effectVariant    = EffectVariantEnum::kDefault;

    auto result = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);

    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
}

TEST_F(TestIdentifyCluster, StopIdentifyingTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate).WithDelegate(&gTestIdentifyDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);

    // Verify identifying started
    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 10u);

    // Stop identifying.
    cluster.StopIdentifying();

    // Verify identifying stopped
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);

    // Verify stop callback was called
    EXPECT_TRUE(onIdentifyStopCalled);
}

TEST_F(TestIdentifyCluster, IdentifyTimeAttributeReportingTest)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & changeListener = context.ChangeListener();

    // 1. Test client write from 0 to non-zero
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, Identify::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, IdentifyTime::Id);

    // 2. Test countdown does NOT report
    changeListener.DirtyList().clear();
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    uint16_t identifyTime;
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 9u);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);

    // 3. Test countdown completion reports (changes to 0)
    changeListener.DirtyList().clear();
    // Advance clock by the remaining 9 seconds to finish the countdown.
    for (int i = 0; i < 9; ++i)
    {
        mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    }
    EXPECT_EQ(ReadNumericAttribute(cluster, IdentifyTime::Id, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, Identify::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, IdentifyTime::Id);

    // 4. Test client write from non-zero to 0
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(5)),
              CHIP_NO_ERROR); // Start again to have a non-zero value.
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, IdentifyTime::Id, static_cast<uint16_t>(0)), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, Identify::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, IdentifyTime::Id);

    // 5. Test Identify command reports
    changeListener.DirtyList().clear();
    Commands::Identify::Type data;
    data.identifyTime = 15;
    auto result       = InvokeCommand(cluster, Commands::Identify::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, Identify::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, IdentifyTime::Id);
}

TEST_F(TestIdentifyCluster, TestGetters)
{
    chip::Test::TestServerClusterContext context;
    IdentifyCluster cluster(IdentifyCluster::Config(kTestEndpointId, mMockTimerDelegate)
                                .WithIdentifyType(IdentifyTypeEnum::kVisibleIndicator)
                                .WithEffectIdentifier(EffectIdentifierEnum::kBreathe)
                                .WithEffectVariant(EffectVariantEnum::kDefault));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetIdentifyType(), IdentifyTypeEnum::kVisibleIndicator);
    EXPECT_EQ(cluster.GetEffectIdentifier(), EffectIdentifierEnum::kBreathe);
    EXPECT_EQ(cluster.GetEffectVariant(), EffectVariantEnum::kDefault);
}

} // namespace
