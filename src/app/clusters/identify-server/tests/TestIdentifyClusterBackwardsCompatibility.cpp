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

#include "ClusterActions.h"
#include "TestTimerDelegate.h"
#include <app/clusters/identify-server/identify-server.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <gtest/gtest.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace chip::app::Clusters::Identify::Attributes;

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

    chip::Test::TestServerClusterContext mContext;
    TestTimerDelegate mTestTimerDelegate;
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
                             onEffectIdentifier, EffectIdentifierEnum::kBlink, EffectVariantEnum::kDefault, &mTestTimerDelegate);
    EXPECT_EQ(identify.mCluster.Cluster().Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test onIdentifyStart callback by writing to IdentifyTime.
    EXPECT_EQ(WriteAttribute(identify.mCluster.Cluster(), IdentifyTime::Id, 10u), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStartCalled);

    // Test onIdentifyStop callback by writing to IdentifyTime.
    EXPECT_EQ(WriteAttribute(identify.mCluster.Cluster(), IdentifyTime::Id, 0u), CHIP_NO_ERROR);
    EXPECT_TRUE(onIdentifyStopCalled);

    // Test onEffectIdentifier callback by invoking TriggerEffect command.
    Commands::TriggerEffect::Type data;
    data.effectIdentifier = EffectIdentifierEnum::kBlink;
    data.effectVariant    = EffectVariantEnum::kDefault;
    auto result           = InvokeCommand(identify.mCluster.Cluster(), Commands::TriggerEffect::Id, data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(onEffectIdentifierCalled);
}

TEST_F(TestIdentifyClusterBackwardsCompatibility, TestCurrentEffectIdentifierUpdate)
{
    onEffectIdentifierCalled = false;
    struct Identify identify(1, nullptr, nullptr, chip::app::Clusters::Identify::IdentifyTypeEnum::kNone, onEffectIdentifier,
                             EffectIdentifierEnum::kStopEffect, EffectVariantEnum::kDefault, &mTestTimerDelegate);
    EXPECT_EQ(identify.mCluster.Cluster().Startup(mContext.Get()), CHIP_NO_ERROR);

    // Check that the effect identifier is the default one
    EXPECT_EQ(identify.mCurrentEffectIdentifier, EffectIdentifierEnum::kStopEffect);

    // Invoke the TriggerEffect command with kBlink
    Commands::TriggerEffect::Type dataBlink;
    dataBlink.effectIdentifier = EffectIdentifierEnum::kBlink;
    dataBlink.effectVariant    = EffectVariantEnum::kDefault;
    auto resultBlink           = InvokeCommand(identify.mCluster.Cluster(), Commands::TriggerEffect::Id, dataBlink);
    ASSERT_TRUE(resultBlink.has_value());
    EXPECT_EQ(resultBlink.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(identify.mCurrentEffectIdentifier, EffectIdentifierEnum::kBlink);

    // Invoke the TriggerEffect command with kFinishEffect
    Commands::TriggerEffect::Type dataFinish;
    dataFinish.effectIdentifier = EffectIdentifierEnum::kFinishEffect;
    dataFinish.effectVariant    = EffectVariantEnum::kDefault;
    auto resultFinish           = InvokeCommand(identify.mCluster.Cluster(), Commands::TriggerEffect::Id, dataFinish);
    ASSERT_TRUE(resultFinish.has_value());
    EXPECT_EQ(resultFinish.value().GetStatusCode().GetStatus(), // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Success);
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
                             EffectIdentifierEnum::kBlink, EffectVariantEnum::kDefault, &mTestTimerDelegate);
    EXPECT_EQ(identify.mCluster.Cluster().Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test that mActive is false initially.
    EXPECT_FALSE(identify.mActive);

    // Test that mActive is true after writing a non-zero value to IdentifyTime.
    EXPECT_EQ(WriteAttribute(identify.mCluster.Cluster(), IdentifyTime::Id, 10u), CHIP_NO_ERROR);
    EXPECT_TRUE(identify.mActive);

    // Test that mActive is false after writing 0 to IdentifyTime.
    EXPECT_EQ(WriteAttribute(identify.mCluster.Cluster(), IdentifyTime::Id, 0u), CHIP_NO_ERROR);
    EXPECT_FALSE(identify.mActive);
}

} // namespace
