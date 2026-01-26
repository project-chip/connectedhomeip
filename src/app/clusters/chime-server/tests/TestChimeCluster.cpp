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

#include <app/clusters/chime-server/ChimeCluster.h>
#include <clusters/Chime/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockChimeDelegate : public ChimeDelegate
{
public:
    struct MockChimeSound
    {
        uint8_t id;
        std::string name;
    };
    std::vector<MockChimeSound> sounds = { { 1, "Ding Dong" }, { 2, "Ring Ring" } };

    bool playChimeSoundCalled                                = false;
    Protocols::InteractionModel::Status playChimeSoundStatus = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR getChimeSoundByIndexError                     = CHIP_NO_ERROR;

    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) override
    {
        if (getChimeSoundByIndexError != CHIP_NO_ERROR)
        {
            return getChimeSoundByIndexError;
        }
        if (chimeIndex < sounds.size())
        {
            chimeID = sounds[chimeIndex].id;
            return CopyCharSpanToMutableCharSpan(CharSpan(sounds[chimeIndex].name.c_str(), sounds[chimeIndex].name.length()), name);
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override
    {
        if (chimeIndex < sounds.size())
        {
            chimeID = sounds[chimeIndex].id;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override
    {
        playChimeSoundCalled = true;
        return playChimeSoundStatus;
    }
};

struct TestChimeCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);
        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override { app::SetSafeAttributePersistenceProvider(nullptr); }

    MockChimeDelegate mMockDelegate;

    ChimeCluster mCluster{ kTestEndpointId, mMockDelegate };

    ClusterTester mClusterTester{ mCluster };

    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
};

TEST_F(TestChimeCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> mandatoryAttributes(Chime::Attributes::kMandatoryMetadata.begin(),
                                                               Chime::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, mandatoryAttributes));
}

TEST_F(TestChimeCluster, TestAcceptedCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              {
                                                  Chime::Commands::PlayChimeSound::kMetadataEntry,
                                              }));
}

TEST_F(TestChimeCluster, TestDelegateErrors)
{
    // Test 1: PlayChimeSound delegate failure
    mMockDelegate.playChimeSoundStatus = Protocols::InteractionModel::Status::Busy;
    auto result                        = mClusterTester.Invoke<Commands::PlayChimeSound::Type>(Commands::PlayChimeSound::Type());
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_TRUE(mMockDelegate.playChimeSoundCalled);
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::Busy);
    }

    // Test 2: GetChimeSoundByIndex delegate failure
    mMockDelegate.getChimeSoundByIndexError = CHIP_ERROR_INTERNAL;
    Attributes::InstalledChimeSounds::TypeInfo::DecodableType list;
    EXPECT_NE(mClusterTester.ReadAttribute(Attributes::InstalledChimeSounds::Id, list), CHIP_NO_ERROR);
}

TEST_F(TestChimeCluster, TestNoOpWrites)
{
    // 1. Write Initial Value (Change)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(1)), CHIP_NO_ERROR);

    auto & dirtyList = mClusterTester.GetDirtyList();
    EXPECT_EQ(dirtyList.size(), 1u);
    dirtyList.clear();

    // 2. Write Same Value (No-Op)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 0u);

    // 3. Write Enabled Initial (Change)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Enabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);
    dirtyList.clear();

    // 4. Write Enabled Same (No-Op)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Enabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 0u);
}

TEST_F(TestChimeCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestChimeCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestChimeCluster, TestReadSelectedChime)
{
    uint8_t selectedChime = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 0);
}

TEST_F(TestChimeCluster, TestReadEnabled)
{
    bool enabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
    EXPECT_EQ(enabled, true);
}

TEST_F(TestChimeCluster, TestInstalledChimeSoundsLifecycle)
{
    // 1. Verify initial content (TestReadInstalledChimeSounds)
    Attributes::InstalledChimeSounds::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::InstalledChimeSounds::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().chimeID, 1);
    EXPECT_TRUE(it.GetValue().name.data_equal(CharSpan("Ding Dong", 9)));

    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().chimeID, 2);
    EXPECT_TRUE(it.GetValue().name.data_equal(CharSpan("Ring Ring", 9)));

    ASSERT_FALSE(it.Next());

    // 2. Change content
    mMockDelegate.sounds.push_back({ 3, "New Sound" });

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::InstalledChimeSounds::Id, list), CHIP_NO_ERROR);
    it = list.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().chimeID, 1);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().chimeID, 2);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue().chimeID, 3);
    EXPECT_TRUE(it.GetValue().name.data_equal(CharSpan("New Sound", 9)));
    ASSERT_FALSE(it.Next());
}

TEST_F(TestChimeCluster, TestWriteAttributes)
{
    // Test writing SelectedChime
    // Write valid value (1)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(1)), CHIP_NO_ERROR);
    uint8_t selectedChime = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 1);

    // Write valid value (2)
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(2)), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 2);

    // Write invalid value (3) - should fail with NotFound as per spec
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(3)),
              Protocols::InteractionModel::Status::NotFound);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 2); // Should remain unchanged

    // Test writing Enabled
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Enabled::Id, false), CHIP_NO_ERROR);
    bool enabled = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
    EXPECT_EQ(enabled, false);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Enabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
    EXPECT_EQ(enabled, true);
}

TEST_F(TestChimeCluster, TestPersistence)
{
    TestServerClusterContext context;
    app::DefaultSafeAttributePersistenceProvider persistenceProvider;
    EXPECT_EQ(persistenceProvider.Init(&context.Get().storage), CHIP_NO_ERROR);
    app::SetSafeAttributePersistenceProvider(&persistenceProvider);
    MockChimeDelegate mockDelegate;

    // 1. Initial startup, verify default values
    {
        ChimeCluster cluster(kTestEndpointId, mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        uint8_t selectedChime = 1;
        EXPECT_EQ(tester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
        EXPECT_EQ(selectedChime, 0);

        bool enabled = false;
        EXPECT_EQ(tester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
        EXPECT_EQ(enabled, true);

        // Modify values
        EXPECT_EQ(tester.WriteAttribute(Attributes::SelectedChime::Id, static_cast<uint8_t>(2)), CHIP_NO_ERROR);
        EXPECT_EQ(tester.WriteAttribute(Attributes::Enabled::Id, false), CHIP_NO_ERROR);
    }

    // 2. Restart (create new cluster instance), verify modified values are loaded
    {
        ChimeCluster cluster(kTestEndpointId, mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        uint8_t selectedChime;
        EXPECT_EQ(tester.ReadAttribute(Attributes::SelectedChime::Id, selectedChime), CHIP_NO_ERROR);
        EXPECT_EQ(selectedChime, 2);

        bool enabled;
        EXPECT_EQ(tester.ReadAttribute(Attributes::Enabled::Id, enabled), CHIP_NO_ERROR);
        EXPECT_EQ(enabled, false);
    }
}

TEST_F(TestChimeCluster, TestPlayChimeSound)
{
    // 1. Test PlayChimeSound when Enabled is true (default)
    auto result = mClusterTester.Invoke<Commands::PlayChimeSound::Type>(Commands::PlayChimeSound::Type());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(mMockDelegate.playChimeSoundCalled);

    // 2. Test PlayChimeSound when Enabled is false
    mMockDelegate.playChimeSoundCalled = false;
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Enabled::Id, false), CHIP_NO_ERROR);

    result = mClusterTester.Invoke<Commands::PlayChimeSound::Type>(Commands::PlayChimeSound::Type());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(mMockDelegate.playChimeSoundCalled);
}

} // namespace
