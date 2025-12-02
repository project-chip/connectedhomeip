/*
 *
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

#include <app/ConcreteClusterPath.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/testing/ClusterTester.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <gtest/gtest.h>
#include <lib/support/TimerDelegateMock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace chip::app::Clusters::Identify::Attributes;

// Forward declaration of the callback to emulate ember initialization
void MatterIdentifyClusterInitCallback(chip::EndpointId endpointId);
// Stub for DataModelHandler init function
void InitDataModelHandler() {}

namespace {

bool onIdentifyStartCalled    = false;
bool onIdentifyStopCalled     = false;
bool onEffectIdentifierCalled = false;

void onIdentifyStart(struct Identify * identify)
{
    onIdentifyStartCalled = true;
}

void onIdentifyStop(struct Identify * identify)
{
    onIdentifyStopCalled = true;
}

void onEffectIdentifier(struct Identify * identify)
{
    onEffectIdentifierCalled           = true;
    identify->mCurrentEffectIdentifier = identify->mCluster.Cluster().GetEffectIdentifier();
}

struct TestIdentifyClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mMockTimerDelegate;
};

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestLegacyInstantiattion)
{
    struct Identify identify(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestLegacyCallbacks)
{
    onIdentifyStartCalled    = false;
    onIdentifyStopCalled     = false;
    onEffectIdentifierCalled = false;

    // Old style struct
    struct Identify identify(1, onIdentifyStart, onIdentifyStop, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,
                             onEffectIdentifier, EffectIdentifierEnum::kBlink, EffectVariantEnum::kDefault, &mMockTimerDelegate);
    chip::Test::ClusterTester tester(identify.mCluster.Cluster());
    EXPECT_EQ(identify.mCluster.Cluster().Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Test onIdentifyStart callback by writing to IdentifyTime.
    EXPECT_TRUE(tester.WriteAttribute(IdentifyTime::Id, 10u).IsSuccess());
    EXPECT_TRUE(onIdentifyStartCalled);

    // Test onIdentifyStop callback by writing to IdentifyTime.
    EXPECT_TRUE(tester.WriteAttribute(IdentifyTime::Id, 0u).IsSuccess());
    EXPECT_TRUE(onIdentifyStopCalled);

    // Test onEffectIdentifier callback by invoking TriggerEffect command.
    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = EffectVariantEnum::kDefault;
    auto result           = tester.Invoke(Commands::TriggerEffect::Id, data);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(onEffectIdentifierCalled);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestCurrentEffectIdentifierUpdate)
{
    onEffectIdentifierCalled = false;
    struct Identify identify(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone, onEffectIdentifier,
                             EffectIdentifierEnum::kStopEffect, EffectVariantEnum::kDefault, &mMockTimerDelegate);
    chip::Test::ClusterTester tester(identify.mCluster.Cluster());
    EXPECT_EQ(identify.mCluster.Cluster().Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Check that the effect identifier is the default one
    EXPECT_EQ(identify.mCurrentEffectIdentifier, EffectIdentifierEnum::kStopEffect);

    // Invoke the TriggerEffect command with kBlink
    Commands::TriggerEffect::Type dataBlink;
    dataBlink.effectIdentifier = EffectIdentifierEnum::kBlink;
    dataBlink.effectVariant    = EffectVariantEnum::kDefault;
    auto resultBlink           = tester.Invoke(Commands::TriggerEffect::Id, dataBlink);
    EXPECT_TRUE(resultBlink.IsSuccess());
    EXPECT_EQ(identify.mCurrentEffectIdentifier, EffectIdentifierEnum::kBlink);

    // Invoke the TriggerEffect command with kFinishEffect
    Commands::TriggerEffect::Type dataFinish;
    dataFinish.effectIdentifier = EffectIdentifierEnum::kFinishEffect;
    dataFinish.effectVariant    = EffectVariantEnum::kDefault;
    auto resultFinish           = tester.Invoke(Commands::TriggerEffect::Id, dataFinish);
    EXPECT_TRUE(resultFinish.IsSuccess());
    EXPECT_EQ(identify.mCurrentEffectIdentifier, EffectIdentifierEnum::kFinishEffect);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestIdentifyTypeInitialization)
{
    // Test with kNone
    struct Identify identifyNone(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone);
    EXPECT_EQ(identifyNone.mIdentifyType, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone);

    // Test with kLightOutput
    struct Identify identifyLightOutput(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kLightOutput);
    EXPECT_EQ(identifyLightOutput.mIdentifyType, chip::app::Clusters::Identify::IdentifyTypeEnum::kLightOutput);

    // Test with kAudibleBeep
    struct Identify identifyAudibleBeep(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kAudibleBeep);
    EXPECT_EQ(identifyAudibleBeep.mIdentifyType, chip::app::Clusters::Identify::IdentifyTypeEnum::kAudibleBeep);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestMActive)
{
    struct Identify identify(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone, nullptr,
                             EffectIdentifierEnum::kBlink, EffectVariantEnum::kDefault, &mMockTimerDelegate);
    chip::Test::ClusterTester tester(identify.mCluster.Cluster());
    EXPECT_EQ(identify.mCluster.Cluster().Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Test that mActive is false initially.
    EXPECT_FALSE(identify.mActive);

    // Test that mActive is true after writing a non-zero value to IdentifyTime.
    EXPECT_TRUE(tester.WriteAttribute(IdentifyTime::Id, 10u).IsSuccess());
    EXPECT_TRUE(identify.mActive);

    // Test that mActive is false after writing 0 to IdentifyTime.
    EXPECT_TRUE(tester.WriteAttribute(IdentifyTime::Id, 0u).IsSuccess());
    EXPECT_FALSE(identify.mActive);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestLateLegacyInstantiation)
{
    EndpointId endpointId = 1;

    // Call MatterIdentifyClusterInitCallback(enpointId) to emulate ember initialization
    MatterIdentifyClusterInitCallback(endpointId);

    // Instantiate legacy Identify late (after the callback/ember init)
    struct Identify identify(endpointId, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone);

    // Check the cluster is properly registered in the CodegenDMP Registry
    EXPECT_TRUE(CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(endpointId, Clusters::Identify::Id)) !=
                nullptr);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, StopIdentifyingTest)
{
    onIdentifyStopCalled = false;

    // Old style struct
    struct Identify identify(1, onIdentifyStart, onIdentifyStop, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone,
                             onEffectIdentifier, EffectIdentifierEnum::kBlink, EffectVariantEnum::kDefault, &mMockTimerDelegate);
    chip::Test::ClusterTester tester(identify.mCluster.Cluster());
    EXPECT_EQ(identify.mCluster.Cluster().Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Start identifying.
    EXPECT_TRUE(tester.WriteAttribute(IdentifyTime::Id, 10u).IsSuccess());
    EXPECT_TRUE(identify.mActive);

    // Find the cluster on the endpoint and call StopIdentifying
    IdentifyCluster * cluster = FindIdentifyClusterOnEndpoint(1);
    ASSERT_NE(cluster, nullptr);
    cluster->StopIdentifying();

    // Verify identifying stopped
    EXPECT_FALSE(identify.mActive);

    // Verify stop callback was called
    EXPECT_TRUE(onIdentifyStopCalled);
}

} // namespace
