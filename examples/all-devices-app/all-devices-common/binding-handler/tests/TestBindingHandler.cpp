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
#include <binding-handler/BindingContext.h>
#include <binding-handler/BindingHandler.h>
#include <binding-handler/SimpleBindingCommand.h>
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

TEST_F(TestBindingHandler, DispatchBeforeInitFailsAndDestroysContext)
{
    BindingHandler handler;
    EXPECT_FALSE(handler.IsInitialized());

    EXPECT_EQ(handler.Dispatch(1, Platform::UniquePtr<BindingContext>(nullptr)), CHIP_ERROR_INVALID_ARGUMENT);

    int destructorCount = 0;
    auto customContext  = MakeTestContext(destructorCount);
    ASSERT_NE(customContext, nullptr);

    EXPECT_EQ(handler.Dispatch(1, std::move(customContext)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);

    EXPECT_EQ(handler.DispatchSimple(1, Clusters::OnOff::Commands::Toggle::Type{}), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestBindingHandler, ShutdownWithoutInitIsSafe)
{
    BindingHandler handler;
    handler.Shutdown();
    EXPECT_FALSE(handler.IsInitialized());
}

/**
 * Fixture with a real Binding::Manager initialized through BindingHandler::Init, backed by a
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

        ASSERT_EQ(InitHandler(mHandler), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mHandler.Shutdown();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    CHIP_ERROR InitHandler(BindingHandler & handler)
    {
        return handler.Init(mBindingManager, { &GetFabricTable(), &mCASESessionManager, &mStorage }, GetExchangeManager());
    }

    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Clusters::Binding::Manager mBindingManager;
    BindingHandler mHandler;
};

TEST_F(TestBindingHandlerWithManager, OnlyOneHandlerCanBeInitialized)
{
    EXPECT_TRUE(mHandler.IsInitialized());
    EXPECT_EQ(InitHandler(mHandler), CHIP_ERROR_INCORRECT_STATE);

    BindingHandler other;
    EXPECT_EQ(InitHandler(other), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(other.IsInitialized());

    // Shutting down a handler that is not initialized must not affect the active one.
    other.Shutdown();
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, Platform::UniquePtr<BindingContext>(nullptr)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(mHandler.IsInitialized());
}

TEST_F(TestBindingHandlerWithManager, AnotherHandlerCanBeInitializedAfterShutdownOrDestruction)
{
    mHandler.Shutdown();
    EXPECT_FALSE(mHandler.IsInitialized());

    {
        BindingHandler other;
        EXPECT_EQ(InitHandler(other), CHIP_NO_ERROR);
        EXPECT_EQ(InitHandler(mHandler), CHIP_ERROR_INCORRECT_STATE);
        // `other` is destroyed while initialized; the destructor must unregister it.
    }

    EXPECT_EQ(InitHandler(mHandler), CHIP_NO_ERROR);
}

TEST_F(TestBindingHandlerWithManager, DispatchFailsAfterShutdown)
{
    mHandler.Shutdown();

    int destructorCount = 0;
    auto context        = MakeTestContext(destructorCount);
    ASSERT_NE(context, nullptr);

    // The manager is still initialized, but the handler is not: the context must be destroyed by
    // Dispatch without being handed to the manager.
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(context)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchWithoutMatchingBindingReleasesContextOnce)
{
    int destructorCount = 0;
    int dispatchCount   = 0;
    auto context        = MakeTestContext(destructorCount, &dispatchCount);
    ASSERT_NE(context, nullptr);

    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 0);
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
    OperationalDeviceProxy * recordedPeer            = reinterpret_cast<OperationalDeviceProxy *>(0x1);
    Messaging::ExchangeManager * recordedExchangeMgr = nullptr;
    auto context = MakeTestContext(destructorCount, &dispatchCount, &recordedEntry, &recordedPeer, &recordedExchangeMgr);
    ASSERT_NE(context, nullptr);

    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 1);
    EXPECT_EQ(recordedEntry.type, Clusters::Binding::MATTER_MULTICAST_BINDING);
    EXPECT_EQ(recordedEntry.groupId, kGroupId);
    EXPECT_EQ(recordedPeer, nullptr);
    EXPECT_EQ(recordedExchangeMgr, &GetExchangeManager());
    EXPECT_EQ(destructorCount, 1);
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

    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}), CHIP_NO_ERROR);
}

// NotifyBoundClusterChanged queues the context for the unicast binding *before* establishing the
// session. When session establishment then fails synchronously (here: the binding refers to a fabric
// the device does not know, so the address lookup cannot start), the call returns an error while the
// pending notification still references the context. Dispatch must not destroy the context in that case.
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

    EXPECT_NE(mHandler.Dispatch(kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);

    // The pending notification still owns the context: it must not have been destroyed.
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 0);

    // Removing the binding drops the last reference; the release handler destroys the context exactly once.
    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, PendingContextIsReleasedAfterShutdown)
{
    constexpr FabricIndex kUnknownFabricIndex = 200;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(Clusters::Binding::TableEntry::ForNode(
                  kUnknownFabricIndex, 0x1234 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    const uint8_t bindingIndex = mBindingManager.GetBindingTable().begin().GetIndex();

    int destructorCount = 0;
    int dispatchCount   = 0;
    auto context        = MakeTestContext(destructorCount, &dispatchCount);
    ASSERT_NE(context, nullptr);
    EXPECT_NE(mHandler.Dispatch(kLocalEndpoint, std::move(context)), CHIP_NO_ERROR);

    mHandler.Shutdown();

    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(dispatchCount, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandHandlesNullPeerGracefully)
{
    Clusters::Binding::TableEntry entry =
        Clusters::Binding::TableEntry::ForNode(GetAliceFabricIndex(), 0x1234 /* node */, 1 /* local */, 2 /* remote */,
                                               Clusters::Chime::Id);

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    chimeCmd.chimeID.SetValue(1);
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(chimeCmd);

    cmd.DispatchCommand(entry, nullptr, GetExchangeManager());
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandSupportsMultipleClusters)
{
    Clusters::Binding::TableEntry entry;
    entry.type   = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local  = 1;
    entry.remote = 2;

    entry.clusterId = Clusters::OnOff::Id;
    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> onOffCmd(Clusters::OnOff::Commands::Toggle::Type{});
    EXPECT_EQ(onOffCmd.GetClusterId(), Clusters::OnOff::Id);
    onOffCmd.DispatchCommand(entry, nullptr, GetExchangeManager());

    entry.clusterId = Clusters::Identify::Id;
    Clusters::Identify::Commands::Identify::Type identifyCmd;
    identifyCmd.identifyTime = 5;
    SimpleBindingCommand<Clusters::Identify::Commands::Identify::Type> identifyBindingCmd(std::move(identifyCmd));
    EXPECT_EQ(identifyBindingCmd.GetClusterId(), Clusters::Identify::Id);
    identifyBindingCmd.DispatchCommand(entry, nullptr, GetExchangeManager());

    entry.clusterId = Clusters::Chime::Id;
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> chimeBindingCmd(
        Clusters::Chime::Commands::PlayChimeSound::Type{});
    EXPECT_EQ(chimeBindingCmd.GetClusterId(), Clusters::Chime::Id);
    chimeBindingCmd.DispatchCommand(entry, nullptr, GetExchangeManager());
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandHandlesClusterMismatch)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::OnOff::Id;

    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(Clusters::Chime::Commands::PlayChimeSound::Type{});
    cmd.DispatchCommand(entry, nullptr, GetExchangeManager());
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandHandlesWildcardBinding)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNICAST_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = std::nullopt;

    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> cmd(Clusters::OnOff::Commands::Toggle::Type{});
    cmd.DispatchCommand(entry, nullptr, GetExchangeManager());
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandHandlesUnsupportedBindingType)
{
    Clusters::Binding::TableEntry entry;
    entry.type      = Clusters::Binding::MATTER_UNUSED_BINDING;
    entry.local     = 1;
    entry.remote    = 2;
    entry.clusterId = Clusters::Chime::Id;

    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> cmd(Clusters::Chime::Commands::PlayChimeSound::Type{});
    cmd.DispatchCommand(entry, nullptr, GetExchangeManager());
}

} // namespace
