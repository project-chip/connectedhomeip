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

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetupPool.h>
#include <binding-handler/BindingHandler.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Ids.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/tests/MessagingContext.h>
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
                               Clusters::Binding::TableEntry * lastEntry = nullptr, OperationalDeviceProxy ** lastPeer = nullptr,
                               Messaging::ExchangeManager ** lastExchangeManager = nullptr) :
        mDestructorCount(destructorCount),
        mDispatchCount(dispatchCount), mLastEntry(lastEntry), mLastPeer(lastPeer), mLastExchangeManager(lastExchangeManager)
    {}

    ~TestCustomContext() override { mDestructorCount++; }

    ClusterId GetClusterId() const override { return 0xFFFF; }

    void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                         Messaging::ExchangeManager & exchangeManager) override
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
        if (mLastExchangeManager != nullptr)
        {
            *mLastExchangeManager = &exchangeManager;
        }
    }

private:
    int & mDestructorCount;
    int * mDispatchCount                               = nullptr;
    Clusters::Binding::TableEntry * mLastEntry         = nullptr;
    OperationalDeviceProxy ** mLastPeer                = nullptr;
    Messaging::ExchangeManager ** mLastExchangeManager = nullptr;
};

template <typename... Args>
Platform::UniquePtr<BindingContext> MakeTestContext(Args &&... args)
{
    return Platform::UniquePtr<BindingContext>(Platform::New<TestCustomContext>(std::forward<Args>(args)...));
}

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

TEST_F(TestBindingHandler, ChangedHandlerSkipsDispatchWhenNotInitialized)
{
    int destructorCount = 0;
    int dispatchCount   = 0;
    TestCustomContext customContext(destructorCount, &dispatchCount);

    Clusters::Binding::TableEntry entry =
        Clusters::Binding::TableEntry::ForGroup(1 /* fabric */, 0x0101 /* group */, 1 /* local */, 0xFFFF);

    // Without InitBindingManager there is no exchange manager to hand to the context: dispatch is skipped.
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &customContext);
    EXPECT_EQ(dispatchCount, 0);
}

TEST_F(TestBindingHandler, DispatchBindingCommandValidationAndCleanup)
{
    EXPECT_EQ(DispatchBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, Platform::UniquePtr<BindingContext>(nullptr)),
              CHIP_ERROR_INVALID_ARGUMENT);

    // When Binding::Manager is not initialized, DispatchBindingCommand fails with CHIP_ERROR_INCORRECT_STATE
    // and must destroy the passed context exactly once.
    int destructorCount = 0;
    auto customContext  = MakeTestContext(destructorCount);
    ASSERT_NE(customContext, nullptr);

    EXPECT_EQ(DispatchBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, std::move(customContext)),
              CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);

    // Verify DispatchSimpleBindingCommand also cleans up and returns CHIP_ERROR_INCORRECT_STATE when uninitialized
    Clusters::OnOff::Commands::Toggle::Type toggleCmd;
    EXPECT_EQ(DispatchSimpleBindingCommand(Clusters::Binding::Manager::GetInstance(), 1, toggleCmd), CHIP_ERROR_INCORRECT_STATE);
}

/**
 * Fixture with a real Binding::Manager initialized through InitBindingManager, backed by a
 * loopback messaging stack and a real CASESessionManager.
 */
class TestBindingHandlerWithManager : public chip::Testing::LoopbackMessagingContext
{
public:
    static constexpr EndpointId kLocalEndpoint = 1;
    static constexpr ClusterId kTestClusterId  = 0xFFFF; // Matches TestCustomContext::GetClusterId()

    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        CASESessionManagerConfig config;
        config.sessionInitParams.sessionManager = &GetSecureSessionManager();
        config.sessionInitParams.exchangeMgr    = &GetExchangeManager();
        config.sessionInitParams.fabricTable    = &GetFabricTable();
        // Only validated for non-null: session establishment never gets far enough to use it in these tests.
        config.sessionInitParams.groupDataProvider = &mGroupDataProvider;
        config.clientPool                          = &mCASEClientPool;
        config.sessionSetupPool                    = &mSessionSetupPool;
        ASSERT_EQ(mCASESessionManager.Init(&GetSystemLayer(), config), CHIP_NO_ERROR);

        ASSERT_EQ(InitBindingManager(mBindingManager, { &GetFabricTable(), &mCASESessionManager, &mStorage }, GetExchangeManager()),
                  CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        ShutdownBindingHandler();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Clusters::Binding::Manager mBindingManager;
};

TEST_F(TestBindingHandlerWithManager, ChangedHandlerForwardsEntryAndPeerToContext)
{
    int destructorCount = 0;
    int dispatchCount   = 0;
    Clusters::Binding::TableEntry recordedEntry;
    OperationalDeviceProxy * recordedPeer            = reinterpret_cast<OperationalDeviceProxy *>(0x1);
    Messaging::ExchangeManager * recordedExchangeMgr = nullptr;

    TestCustomContext customContext(destructorCount, &dispatchCount, &recordedEntry, &recordedPeer, &recordedExchangeMgr);

    Clusters::Binding::TableEntry entry =
        Clusters::Binding::TableEntry::ForNode(1 /* fabric */, 0x1234 /* node */, 2 /* local */, 3 /* remote */, 0xFFFF);

    AllDevicesBoundDeviceChangedHandler(entry, nullptr, &customContext);
    EXPECT_EQ(dispatchCount, 1);
    EXPECT_EQ(recordedEntry, entry);
    EXPECT_EQ(recordedPeer, nullptr);
    EXPECT_EQ(recordedExchangeMgr, &GetExchangeManager());
}

TEST_F(TestBindingHandlerWithManager, ChangedHandlerHandlesNullContextGracefully)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    // Passing nullptr context should not crash
    AllDevicesBoundDeviceChangedHandler(entry, nullptr, nullptr);
}

TEST_F(TestBindingHandlerWithManager, ChangedHandlerHandlesNullPeerGracefully)
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

TEST_F(TestBindingHandlerWithManager, ChangedHandlerSupportsMultipleClusters)
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

TEST_F(TestBindingHandlerWithManager, ChangedHandlerHandlesClusterMismatch)
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

TEST_F(TestBindingHandlerWithManager, ChangedHandlerHandlesWildcardBinding)
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

TEST_F(TestBindingHandlerWithManager, ChangedHandlerHandlesUnsupportedBindingType)
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

TEST_F(TestBindingHandlerWithManager, DispatchSimpleCommandToGroupBinding)
{
    // Exercises the real SimpleBindingCommand multicast path with the injected (loopback) exchange manager.
    // No group keys are provisioned, so the send itself fails and is logged; the dispatch must still
    // complete and release the context without touching any global Server state.
    constexpr GroupId kGroupId = 0x0102;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kLocalEndpoint, Clusters::OnOff::Id)),
              CHIP_NO_ERROR);

    EXPECT_EQ(DispatchSimpleBindingCommand(mBindingManager, kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}),
              CHIP_NO_ERROR);
}

TEST_F(TestBindingHandlerWithManager, DispatchWithoutMatchingBindingReleasesContextOnce)
{
    int destructorCount = 0;
    int dispatchCount   = 0;
    auto context        = MakeTestContext(destructorCount, &dispatchCount);
    ASSERT_NE(context, nullptr);

    EXPECT_EQ(DispatchBindingCommand(mBindingManager, kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchRejectsManagerAfterShutdown)
{
    ShutdownBindingHandler();

    int destructorCount = 0;
    auto context        = MakeTestContext(destructorCount);
    ASSERT_NE(context, nullptr);

    // The manager is still initialized, but no longer registered with the handler: the context must
    // be destroyed by DispatchBindingCommand without being handed to the manager.
    EXPECT_EQ(DispatchBindingCommand(mBindingManager, kLocalEndpoint, std::move(context)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchToGroupBindingInvokesContextAndReleasesIt)
{
    constexpr GroupId kGroupId = 0x0101;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);

    int destructorCount = 0;
    int dispatchCount   = 0;
    Clusters::Binding::TableEntry recordedEntry;
    OperationalDeviceProxy * recordedPeer = reinterpret_cast<OperationalDeviceProxy *>(0x1);
    auto context                          = MakeTestContext(destructorCount, &dispatchCount, &recordedEntry, &recordedPeer);
    ASSERT_NE(context, nullptr);

    EXPECT_EQ(DispatchBindingCommand(mBindingManager, kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 1);
    EXPECT_EQ(recordedEntry.type, Clusters::Binding::MATTER_MULTICAST_BINDING);
    EXPECT_EQ(recordedEntry.groupId, kGroupId);
    EXPECT_EQ(recordedPeer, nullptr);
    EXPECT_EQ(destructorCount, 1);
}

// Regression test: NotifyBoundClusterChanged queues the context for the unicast binding *before*
// establishing the session. When session establishment then fails synchronously (here: the binding
// refers to a fabric the device does not know, so the address lookup cannot start), the call returns
// an error while the pending notification still references the context.
//
// DispatchBindingCommand used to delete the context in that case, leaving a dangling pointer in the
// pending notification map (use-after-free on the next connection, double free when the entry is removed).
TEST_F(TestBindingHandlerWithManager, ContextStaysAliveWhenNotifyFailsAfterQueueing)
{
    constexpr FabricIndex kUnknownFabricIndex = 200;
    constexpr NodeId kPeerNodeId              = 0x1234;
    ASSERT_EQ(GetFabricTable().FindFabricWithIndex(kUnknownFabricIndex), nullptr);
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(Clusters::Binding::TableEntry::ForNode(
                  kUnknownFabricIndex, kPeerNodeId, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    const uint8_t bindingIndex = mBindingManager.GetBindingTable().begin().GetIndex();

    int destructorCount = 0;
    int dispatchCount   = 0;
    auto context        = MakeTestContext(destructorCount, &dispatchCount);
    ASSERT_NE(context, nullptr);

    EXPECT_NE(DispatchBindingCommand(mBindingManager, kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);

    // The pending notification still owns the context: it must not have been destroyed.
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 0);

    // Removing the binding drops the last reference; the release handler destroys the context exactly once.
    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 1);
}

} // namespace
