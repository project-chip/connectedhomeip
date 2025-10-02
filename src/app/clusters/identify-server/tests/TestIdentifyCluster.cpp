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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include "TestIdentifyClusterHelpers.h"
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Identify/Attributes.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace chip::app::Clusters::Identify::Attributes;

namespace {

bool onIdentifyStartCalled      = false;
bool onIdentifyStopCalled       = false;
bool onEffectIdentifierCalled = false;

void onIdentifyStart(IdentifyCluster * cluster)
{
    onIdentifyStartCalled = true;
}

void onIdentifyStop(IdentifyCluster * cluster)
{
    onIdentifyStopCalled = true;
}

void onEffectIdentifier(IdentifyCluster * cluster)
{
    onEffectIdentifierCalled = true;
}

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

    chip::Test::TestServerClusterContext mContext;
    TestTimerDelegate mTestTimerDelegate;
};

TEST_F(TestIdentifyCluster, TestCreate)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
}

TEST_F(TestIdentifyCluster, AttributeListTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(endpoint, Identify::Id), attributes), CHIP_NO_ERROR);

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
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(endpoint, Identify::Id), acceptedCommands), CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        Commands::Identify::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestIdentifyCluster, AcceptedCommandsWithOptionalTriggerEffectTest)
{
    constexpr EndpointId endpoint = 1;
    // When passing in onEffectIdentifier callback, we're enabling the TriggerEffect command.
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(endpoint, Identify::Id), acceptedCommands), CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        Commands::Identify::kMetadataEntry,
        Commands::TriggerEffect::kMetadataEntry, // Expects the optional command because we passed in onEffectIdentifier.
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestIdentifyCluster, ClusterRevisionTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Read and verify ClusterRevision
    // Note: Identify.adoc specifies revision 6, but the generated code is at revision 5.
    uint16_t clusterRevision;
    EXPECT_EQ(ReadAttribute(cluster, { endpoint, Identify::Id, Globals::Attributes::ClusterRevision::Id }, clusterRevision),
              CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 5u);
}

TEST_F(TestIdentifyCluster, FeatureMapTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Read and verify FeatureMap
    uint32_t featureMap;
    EXPECT_EQ(ReadAttribute(cluster, { endpoint, Identify::Id, FeatureMap::Id }, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestIdentifyCluster, ReadIdentifyAttributesTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Read and verify IdentifyTime
    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, { endpoint, Identify::Id, IdentifyTime::Id }, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);

    // Read and verify IdentifyType
    uint8_t identifyType;
    EXPECT_EQ(ReadAttribute(cluster, { endpoint, Identify::Id, IdentifyType::Id }, identifyType), CHIP_NO_ERROR);
    EXPECT_EQ(identifyType, 0u);

    // Read non-existent attribute
    uint32_t nonExistentAttribute;
    EXPECT_NE(ReadAttribute(cluster, { endpoint, Identify::Id, 0xFFFF }, nonExistentAttribute), CHIP_NO_ERROR);
}

TEST_F(TestIdentifyCluster, WriteIdentifyTimeTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    uint16_t identifyTime;
    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // Write a value to IdentifyTime and verify it was written.
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 10u);

    // Write 0 to IdentifyTime and verify it was written.
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 0u), CHIP_NO_ERROR);
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
}

TEST_F(TestIdentifyCluster, WriteReadOnlyAttributesReturnUnsupportedWriteTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Attempt to write to the read-only IdentifyType attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, { endpoint, Identify::Id, IdentifyType::Id }, (uint8_t)0),
              CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));

    // Attempt to write to the read-only ClusterRevision attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, { endpoint, Identify::Id, Globals::Attributes::ClusterRevision::Id }, 0u),
              CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));

    // Attempt to write to the read-only FeatureMap attribute and verify it fails.
    EXPECT_EQ(WriteAttribute(cluster, { endpoint, Identify::Id, FeatureMap::Id }, 0u), CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

TEST_F(TestIdentifyCluster, IdentifyTypeCtorInitTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kVisibleIndicator)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithEffectIdentifier(EffectIdentifierEnum::kBreathe));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    uint8_t identifyType;
    EXPECT_EQ(ReadAttribute(cluster, { endpoint, Identify::Id, IdentifyType::Id }, identifyType), CHIP_NO_ERROR);
    EXPECT_EQ(identifyType, (uint8_t)IdentifyTypeEnum::kVisibleIndicator);
}

TEST_F(TestIdentifyCluster, IdentifyTimeCountdownTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    uint16_t identifyTime;
    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // Write a value to IdentifyTime to start the countdown.
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 5u), CHIP_NO_ERROR);

    // Advance the clock 1 second at a time and check the attribute value.
    for (uint16_t i = 0; i < 5; ++i)
    {
        mTestTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
        EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
        EXPECT_EQ(identifyTime, 4u - i);
    }

    // The timer should not be active anymore.
    EXPECT_FALSE(mTestTimerDelegate.IsTimerActive(&cluster));
}

TEST_F(TestIdentifyCluster, OnIdentifyStartStopCallbackTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone)
                                .WithOnIdentifyStart(onIdentifyStart)
                                .WithOnIdentifyStop(onIdentifyStop)
                                .WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // Test onIdentifyStart callback.
    onIdentifyStartCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStartCalled);

    // Test onIdentifyStop callback.
    onIdentifyStopCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 0u), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStopCalled);

    // Test onIdentifyStop callback with timer.
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 2u), CHIP_NO_ERROR);
    onIdentifyStopCalled = false;
    mTestTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_FALSE(onIdentifyStopCalled);
    mTestTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_TRUE(onIdentifyStopCalled);
}

TEST_F(TestIdentifyCluster, OnStartNotCalledMultipleTimes)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone)
                                .WithOnIdentifyStart(onIdentifyStart)
                                .WithOnIdentifyStop(onIdentifyStop)
                                .WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // Start identifying.
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStartCalled);

    // Reset the flag and write a non-zero value again.
    onIdentifyStartCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 5u), CHIP_NO_ERROR);
    EXPECT_FALSE(onIdentifyStartCalled);
}

TEST_F(TestIdentifyCluster, OnStopNotCalledIfNotIdentifying)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone)
                                .WithOnIdentifyStart(onIdentifyStart)
                                .WithOnIdentifyStop(onIdentifyStop)
                                .WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);
    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // Ensure we are not identifying.
    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);

    // Write 0 and check that the stop callback is not called.
    onIdentifyStopCalled = false;
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 0u), CHIP_NO_ERROR);
    EXPECT_FALSE(onIdentifyStopCalled);
}

TEST_F(TestIdentifyCluster, InvokeIdentifyCommandTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(
        IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test with a value of 10 for identifyTime.
    {
        constexpr uint16_t kIdentifyTime = 10;
        Commands::Identify::Type data;
        data.identifyTime = kIdentifyTime;

        auto result = InvokeCommand(cluster, Commands::Identify::Id, data);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);

        uint16_t identifyTime;
        EXPECT_EQ(ReadAttribute(cluster, { kEndpointId, Identify::Id, IdentifyTime::Id }, identifyTime), CHIP_NO_ERROR);
        EXPECT_EQ(identifyTime, kIdentifyTime);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test with a specific effectIdentifier and effectVariant.
    {
        Commands::TriggerEffect::Type data;
        data.effectIdentifier = EffectIdentifierEnum::kBlink;
        data.effectVariant    = EffectVariantEnum::kDefault;

        onEffectIdentifierCalled = false;
        auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(onEffectIdentifierCalled);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandAllEffectsTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

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
        EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
        EXPECT_TRUE(onEffectIdentifierCalled);
    }
}

TEST_F(TestIdentifyCluster, InvokeTriggerEffectCommandInvalidVariantTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test with an invalid effect variant.
    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = static_cast<EffectVariantEnum>(0xFF); // Invalid variant.

    onEffectIdentifierCalled = false;
    auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(onEffectIdentifierCalled);
}

TEST_F(TestIdentifyCluster, TriggerEffectWhileIdentifyingTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Start identifying.
    const auto identifyTimePath = ConcreteDataAttributePath(kEndpointId, Identify::Id, IdentifyTime::Id);
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = EffectVariantEnum::kDefault;

    onEffectIdentifierCalled = false;
    auto result              = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(onEffectIdentifierCalled);

    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
}

TEST_F(TestIdentifyCluster, TriggerEffectFinishEffectTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Start identifying.
    const auto identifyTimePath = ConcreteDataAttributePath(kEndpointId, Identify::Id, IdentifyTime::Id);
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kFinishEffect;
    data.effectVariant    = EffectVariantEnum::kDefault;

    auto result = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);

    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 1u);
}

TEST_F(TestIdentifyCluster, TriggerEffectStopEffectTest)
{
    constexpr EndpointId kEndpointId = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(kEndpointId, IdentifyTypeEnum::kNone)
                                .WithTimerDelegate(&mTestTimerDelegate)
                                .WithOnEffectIdentifier(onEffectIdentifier));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    // Start identifying.
    const auto identifyTimePath = ConcreteDataAttributePath(kEndpointId, Identify::Id, IdentifyTime::Id);
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);

    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kStopEffect;
    data.effectVariant    = EffectVariantEnum::kDefault;

    auto result = InvokeCommand(cluster, Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);

    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
}

TEST_F(TestIdentifyCluster, IdentifyTimeAttributeReportingTest)
{
    constexpr EndpointId endpoint = 1;
    IdentifyCluster cluster(IdentifyCluster::Config(endpoint, IdentifyTypeEnum::kNone).WithTimerDelegate(&mTestTimerDelegate));
    EXPECT_EQ(cluster.Startup(mContext.Get()), CHIP_NO_ERROR);

    auto & changeListener       = mContext.ChangeListener();
    const auto identifyTimePath = ConcreteDataAttributePath(endpoint, Identify::Id, IdentifyTime::Id);

    // 1. Test client write from 0 to non-zero
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 10u), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, identifyTimePath.mEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, identifyTimePath.mClusterId);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, identifyTimePath.mAttributeId);

    // 2. Test countdown does NOT report
    changeListener.DirtyList().clear();
    mTestTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    uint16_t identifyTime;
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 9u);
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);

    // 3. Test countdown completion reports (changes to 0)
    changeListener.DirtyList().clear();
    // Advance clock by the remaining 9 seconds to finish the countdown.
    for (int i = 0; i < 9; ++i)
    {
        mTestTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    }
    EXPECT_EQ(ReadAttribute(cluster, identifyTimePath, identifyTime), CHIP_NO_ERROR);
    EXPECT_EQ(identifyTime, 0u);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, identifyTimePath.mEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, identifyTimePath.mClusterId);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, identifyTimePath.mAttributeId);

    // 4. Test client write from non-zero to 0
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 5u), CHIP_NO_ERROR); // Start again to have a non-zero value.
    changeListener.DirtyList().clear();
    EXPECT_EQ(WriteAttribute(cluster, identifyTimePath, 0u), CHIP_NO_ERROR);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, identifyTimePath.mEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, identifyTimePath.mClusterId);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, identifyTimePath.mAttributeId);

    // 5. Test Identify command reports
    changeListener.DirtyList().clear();
    Commands::Identify::Type data;
    data.identifyTime = 15;
    auto result       = InvokeCommand(cluster, Commands::Identify::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, identifyTimePath.mEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, identifyTimePath.mClusterId);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, identifyTimePath.mAttributeId);
}

} // namespace