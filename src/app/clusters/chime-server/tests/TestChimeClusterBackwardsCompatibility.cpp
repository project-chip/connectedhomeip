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
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/chime-server/chime-server.h>
#include <app/server-cluster/testingClusterTester.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <gtest/gtest.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;

void MatterChimePluginServerInitCallback();
// Stub for DataModelHandler init function
void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockChimeDelegate : public ChimeDelegate
{
public:
    bool playChimeSoundCalled = false;

    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) override
    {
        if (chimeIndex == 0)
        {
            chimeID = 1;
            return CopyCharSpanToMutableCharSpan(CharSpan("Ding Dong", 9), name);
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override
    {
        if (chimeIndex == 0)
        {
            chimeID = 1;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    Protocols::InteractionModel::Status PlayChimeSound() override
    {
        playChimeSoundCalled = true;
        return Protocols::InteractionModel::Status::Success;
    }
};

struct TestChimeClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);
        EXPECT_EQ(mChimeServer.Init(), CHIP_NO_ERROR);
        EXPECT_EQ(mChimeServer.mCluster.Cluster().Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override { app::SetSafeAttributePersistenceProvider(nullptr); }

    MockChimeDelegate mMockDelegate;

    ChimeServer mChimeServer{ kTestEndpointId, mMockDelegate };

    chip::Testing::ClusterTester mClusterTester{ mChimeServer.mCluster.Cluster() };

    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
};

TEST_F(TestChimeClusterBackwardsCompatibility, TestLegacyInstantiation)
{
    // Verify it registered with CodegenDataModelProvider
    EXPECT_TRUE(CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, Chime::Id)) != nullptr);

    // Test Reading Attribute (via ChimeCluster logic)
    uint8_t selectedChime = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 0);

    // Test Writing Attribute (via ChimeCluster logic)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 1);

    // Test Command (via ChimeCluster logic, invoking delegate)
    auto result = mClusterTester.Invoke<Commands::PlayChimeSound::Type>(Commands::PlayChimeSound::Type());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(mMockDelegate.playChimeSoundCalled);
}

TEST_F(TestChimeClusterBackwardsCompatibility, TestLegacySetters)
{
    // Test SetSelectedChime (Legacy API)
    EXPECT_EQ(mChimeServer.SetSelectedChime(1), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(mChimeServer.GetSelectedChime(), 1);

    // Verify it affected the attribute
    uint8_t selectedChime = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 1);

    // Test SetEnabled (Legacy API)
    EXPECT_EQ(mChimeServer.SetEnabled(false), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(mChimeServer.GetEnabled(), false);

    bool enabled = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
    EXPECT_EQ(enabled, false);
}

TEST_F(TestChimeClusterBackwardsCompatibility, TestLegacyLifecycle)
{
    constexpr EndpointId kLifecycleEndpointId = 2;
    {
        ChimeServer chimeServer(kLifecycleEndpointId, mMockDelegate);

        // Check Init
        EXPECT_EQ(chimeServer.Init(), CHIP_NO_ERROR);

        // Check GetEndpointId
        EXPECT_EQ(chimeServer.GetEndpointId(), kLifecycleEndpointId);

        // Verify registered
        EXPECT_TRUE(CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kLifecycleEndpointId, Chime::Id)) !=
                    nullptr);
    }
    // Verify unregistered after destruction
    EXPECT_TRUE(CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kLifecycleEndpointId, Chime::Id)) ==
                nullptr);
}

} // namespace
