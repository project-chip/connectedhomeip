/*
 *
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

#include <binding-handler/BindingHandler.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class TestBindingHandler : public ::testing::Test
{
protected:
    void SetUp() override { EXPECT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    void TearDown() override { Platform::MemoryShutdown(); }
};

class TestCustomContext : public BindingContext
{
public:
    explicit TestCustomContext(int & destructorCount, int * dispatchCount = nullptr,
                               Clusters::Binding::TableEntry * lastEntry = nullptr,
                               OperationalDeviceProxy ** lastPeer        = nullptr) :
        mDestructorCount(destructorCount),
        mDispatchCount(dispatchCount), mLastEntry(lastEntry), mLastPeer(lastPeer)
    {}

    ~TestCustomContext() override { mDestructorCount++; }

    ClusterId GetClusterId() const override { return 0xFFFF; }

    void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device) override
    {
        if (mDispatchCount != nullptr)
        {
            (*mDispatchCount)++;
        }
        if (mLastEntry != nullptr)
        {
            *mLastEntry = binding;
        }
        if (mLastPeer != nullptr)
        {
            *mLastPeer = peer_device;
        }
    }

private:
    int & mDestructorCount;
    int * mDispatchCount                         = nullptr;
    Clusters::Binding::TableEntry * mLastEntry   = nullptr;
    OperationalDeviceProxy ** mLastPeer          = nullptr;
};

TEST_F(TestBindingHandler, ContextReleaseHandlerDeletesContext)
{
    // Test that passing nullptr does not crash
    AllDevicesBoundDeviceContextReleaseHandler(nullptr);

    // Test polymorphic context destruction and memory cleanup
    int destructorCount  = 0;
    auto * customContext = Platform::New<TestCustomContext>(destructorCount);
    VerifyOrDie(customContext != nullptr);

    AllDevicesBoundDeviceContextReleaseHandler(customContext);
    EXPECT_EQ(destructorCount, 1);

    // Test releasing SimpleBindingCommand
    Clusters::Chime::Commands::PlayChimeSound::Type playChimeCmd;
    auto * chimeCmd = Platform::New<SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type>>(playChimeCmd);
    VerifyOrDie(chimeCmd != nullptr);

    AllDevicesBoundDeviceContextReleaseHandler(chimeCmd);
}

TEST_F(TestBindingHandler, ChangedHandlerForwardsEntryAndPeerToContext)
{
    int destructorCount = 0;
    int dispatchCount   = 0;
    Clusters::Binding::TableEntry recordedEntry;
    OperationalDeviceProxy * recordedPeer = reinterpret_cast<OperationalDeviceProxy *>(0x1);

    TestCustomContext customContext(destructorCount, &dispatchCount, &recordedEntry, &recordedPeer);

    Clusters::Binding::TableEntry entry =
        Clusters::Binding::TableEntry::ForNode(1 /* fabric */, 0x1234 /* node */, 2 /* local */, 3 /* remote */, 0xFFFF);

    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &customContext);
    EXPECT_EQ(dispatchCount, 1);
    EXPECT_EQ(recordedEntry, entry);
    EXPECT_EQ(recordedPeer, nullptr);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesNullContextGracefully)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    // Passing nullptr context should not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, nullptr);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesNullPeerGracefully)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    chimeCmd.chimeID.SetValue(1);
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    // Passing nullptr peer_device should not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerSupportsMultipleClusters)
{
    // 1. OnOff Cluster
    Clusters::Binding::TableEntry onOffEntry;
    onOffEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    onOffEntry.local     = 1;
    onOffEntry.remote    = 2;
    onOffEntry.clusterId = Clusters::OnOff::Id;

    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> onOffCmd(toggleCmd);
    EXPECT_EQ(onOffCmd.GetClusterId(), Clusters::OnOff::Id);
    AllDevicesBoundDeviceChangedHandler(onOffEntry, nullptr, &onOffCmd);

    // 2. Identify Cluster (using move constructor)
    Clusters::Binding::TableEntry identifyEntry;
    identifyEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    identifyEntry.local     = 1;
    identifyEntry.remote    = 2;
    identifyEntry.clusterId = Clusters::Identify::Id;

    Clusters::Identify::Commands::Identify::Type identifyCmd;
    identifyCmd.identifyTime = 5;
    SimpleBindingCommand<Clusters::Identify::Commands::Identify::Type> identifyBindingCmd(std::move(identifyCmd));
    EXPECT_EQ(identifyBindingCmd.GetClusterId(), Clusters::Identify::Id);
    AllDevicesBoundDeviceChangedHandler(identifyEntry, nullptr, &identifyBindingCmd);

    // 3. Chime Cluster
    Clusters::Binding::TableEntry chimeEntry;
    chimeEntry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    chimeEntry.local     = 1;
    chimeEntry.remote    = 2;
    chimeEntry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> chimeBindingCmd(chimeCmd);
    EXPECT_EQ(chimeBindingCmd.GetClusterId(), Clusters::Chime::Id);
    AllDevicesBoundDeviceChangedHandler(chimeEntry, nullptr, &chimeBindingCmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesClusterMismatch)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::OnOff::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    // Passing a Chime command to an OnOff binding entry should detect mismatch and not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesWildcardBinding)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = std::nullopt; // Wildcard binding

    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> cmd(toggleCmd);

    // Wildcard binding should adopt the command's cluster ID and execute without crashing
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, ChangedHandlerHandlesUnsupportedBindingType)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNUSED_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &cmd);
}

TEST_F(TestBindingHandler, DispatchAndScheduleBindingCommandValidationAndCleanup)
{
    EXPECT_EQ(DispatchBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, Platform::UniquePtr<BindingContext>(nullptr)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ScheduleBindingCommand(DeviceLayer::PlatformMgr(), Clusters::Binding::Manager::GetInstance(), 1,
                                     Platform::UniquePtr<BindingContext>(nullptr)),
              CHIP_ERROR_INVALID_ARGUMENT);

    // When Binding::Manager is not initialized, DispatchBindingCommand fails with CHIP_ERROR_INCORRECT_STATE
    // and must destroy the passed UniquePtr<TestCustomContext> exactly once.
    int destructorCount = 0;
    auto customContext  = Platform::MakeUnique<TestCustomContext>(destructorCount);
    ASSERT_NE(customContext, nullptr);

    EXPECT_EQ(DispatchBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, std::move(customContext)),
              CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);

    // Verify DispatchSimpleBindingCommand also cleans up and returns CHIP_ERROR_INCORRECT_STATE when uninitialized
    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    EXPECT_EQ(DispatchSimpleBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, toggleCmd),
              CHIP_ERROR_INCORRECT_STATE);
}

} // namespace

