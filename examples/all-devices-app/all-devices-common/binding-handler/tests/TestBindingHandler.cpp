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
#include <binding-handler/BindingCommand.h>
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
#include <lib/support/Defer.h>
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

struct DispatchRecord
{
    int count                                    = 0;
    bool lastWasGroup                            = false;
    FabricIndex fabricIndex                      = kUndefinedFabricIndex;
    GroupId groupId                              = 0;
    Messaging::ExchangeManager * exchangeManager = nullptr;
};

class TestCustomCommand : public BindingCommand
{
public:
    explicit TestCustomCommand(int & destructorCount, DispatchRecord * record = nullptr) :
        mDestructorCount(destructorCount), mRecord(record)
    {}

    ~TestCustomCommand() override { mDestructorCount++; }

    ClusterId GetClusterId() const override { return 0xFFFF; }

    void Dispatch(const BindingTarget & target) override
    {
        VerifyOrReturn(mRecord != nullptr);
        mRecord->count++;
        if (const auto * group = std::get_if<GroupBindingTarget>(&target))
        {
            mRecord->lastWasGroup    = true;
            mRecord->fabricIndex     = group->fabricIndex;
            mRecord->groupId         = group->groupId;
            mRecord->exchangeManager = &group->exchangeManager;
        }
        else if (const auto * unicast = std::get_if<UnicastBindingTarget>(&target))
        {
            mRecord->lastWasGroup    = false;
            mRecord->exchangeManager = &unicast->exchangeManager;
        }
    }

private:
    int & mDestructorCount;
    DispatchRecord * mRecord = nullptr;
};

template <typename... Args>
Platform::UniquePtr<BindingCommand> MakeTestCommand(Args &&... args)
{
    return Platform::UniquePtr<BindingCommand>(Platform::New<TestCustomCommand>(std::forward<Args>(args)...));
}

TEST_F(TestBindingHandler, DispatchBeforeInitFailsAndDestroysCommand)
{
    BindingHandler handler;
    EXPECT_FALSE(handler.IsInitialized());

    EXPECT_EQ(handler.Dispatch(1, Platform::UniquePtr<BindingCommand>(nullptr)), CHIP_ERROR_INVALID_ARGUMENT);

    int destructorCount = 0;
    auto command        = MakeTestCommand(destructorCount);
    ASSERT_NE(command, nullptr);

    EXPECT_EQ(handler.Dispatch(1, std::move(command)), CHIP_ERROR_INCORRECT_STATE);
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
    static constexpr ClusterId kTestClusterId  = 0xFFFF; // Matches TestCustomCommand::GetClusterId()

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

TEST_F(TestBindingHandlerWithManager, InitTwiceFails)
{
    EXPECT_TRUE(mHandler.IsInitialized());
    EXPECT_EQ(InitHandler(mHandler), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mHandler.IsInitialized());
}

TEST_F(TestBindingHandlerWithManager, InitAfterShutdownFails)
{
    mHandler.Shutdown();
    EXPECT_FALSE(mHandler.IsInitialized());

    EXPECT_EQ(InitHandler(mHandler), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(mHandler.IsInitialized());
}

TEST_F(TestBindingHandlerWithManager, HandlersOnSeparateManagersAreIndependent)
{
    constexpr GroupId kGroupId = 0x0103;

    TestPersistentStorageDelegate otherStorage;
    Clusters::Binding::Manager otherManager;
    BindingHandler otherHandler;
    auto removeOtherManagerDelegate = MakeDefer([&] { GetFabricTable().RemoveFabricDelegate(&otherManager); });
    ASSERT_EQ(otherHandler.Init(otherManager, { &GetFabricTable(), &mCASESessionManager, &otherStorage }, GetExchangeManager()),
              CHIP_NO_ERROR);

    for (auto * manager : { &mBindingManager, &otherManager })
    {
        ASSERT_EQ(manager->GetBindingTable().Add(
                      Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kLocalEndpoint, kTestClusterId)),
                  CHIP_NO_ERROR);
    }

    int destructorCount = 0;
    DispatchRecord record;
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_NO_ERROR);
    EXPECT_EQ(otherHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 2);
    EXPECT_EQ(destructorCount, 2);

    // Shutting down one handler does not affect the other.
    otherHandler.Shutdown();
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 3);
    EXPECT_EQ(destructorCount, 3);
}

TEST_F(TestBindingHandlerWithManager, DispatchFailsAfterShutdown)
{
    mHandler.Shutdown();

    int destructorCount = 0;
    auto command        = MakeTestCommand(destructorCount);
    ASSERT_NE(command, nullptr);

    // The manager is still initialized, but the handler is not: the command must be destroyed by
    // Dispatch without being handed to the manager.
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchWithoutMatchingBindingReleasesCommandOnce)
{
    int destructorCount = 0;
    DispatchRecord record;
    auto command = MakeTestCommand(destructorCount, &record);
    ASSERT_NE(command, nullptr);

    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchToGroupBindingPassesGroupTargetAndReleasesCommand)
{
    constexpr GroupId kGroupId = 0x0101;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);

    int destructorCount = 0;
    DispatchRecord record;
    auto command = MakeTestCommand(destructorCount, &record);
    ASSERT_NE(command, nullptr);

    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 1);
    EXPECT_TRUE(record.lastWasGroup);
    EXPECT_EQ(record.fabricIndex, GetAliceFabricIndex());
    EXPECT_EQ(record.groupId, kGroupId);
    EXPECT_EQ(record.exchangeManager, &GetExchangeManager());
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchToWildcardGroupBinding)
{
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0104, kLocalEndpoint, std::nullopt)),
              CHIP_NO_ERROR);

    int destructorCount = 0;
    DispatchRecord record;
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 1);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, BindingOnOtherClusterIsNotServed)
{
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0105, kLocalEndpoint, Clusters::OnOff::Id)),
              CHIP_NO_ERROR);

    int destructorCount = 0;
    DispatchRecord record;
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, DispatchSimpleCommandToGroupBinding)
{
    // Exercises the real SimpleBindingCommand multicast path with the injected (loopback) exchange manager.
    // No group keys are provisioned, so the send itself fails and is logged; the dispatch must still
    // complete and release the command without touching any global Server state.
    constexpr GroupId kGroupId = 0x0102;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), kGroupId, kLocalEndpoint, Clusters::OnOff::Id)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}), CHIP_NO_ERROR);
}

// NotifyBoundClusterChanged queues the context for the unicast binding *before* establishing the
// session. When session establishment then fails synchronously (here: the binding refers to a fabric
// the device does not know, so the address lookup cannot start), the call returns an error while the
// pending notification still references the command. Dispatch must not destroy the command in that case.
TEST_F(TestBindingHandlerWithManager, CommandStaysAliveWhenNotifyFailsAfterQueueing)
{
    constexpr FabricIndex kUnknownFabricIndex = 200;
    constexpr NodeId kPeerNodeId              = 0x1234;
    ASSERT_EQ(GetFabricTable().FindFabricWithIndex(kUnknownFabricIndex), nullptr);
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(Clusters::Binding::TableEntry::ForNode(
                  kUnknownFabricIndex, kPeerNodeId, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    const uint8_t bindingIndex = mBindingManager.GetBindingTable().begin().GetIndex();

    int destructorCount = 0;
    DispatchRecord record;
    auto command = MakeTestCommand(destructorCount, &record);
    ASSERT_NE(command, nullptr);

    EXPECT_NE(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);

    // The pending notification still owns the command: it must not have been destroyed.
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 0);

    // Removing the binding drops the last reference; the release handler destroys the command exactly once.
    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, PendingCommandIsReleasedAfterShutdown)
{
    constexpr FabricIndex kUnknownFabricIndex = 200;
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(Clusters::Binding::TableEntry::ForNode(
                  kUnknownFabricIndex, 0x1234 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    const uint8_t bindingIndex = mBindingManager.GetBindingTable().begin().GetIndex();

    int destructorCount = 0;
    DispatchRecord record;
    auto command = MakeTestCommand(destructorCount, &record);
    ASSERT_NE(command, nullptr);
    EXPECT_NE(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);

    mHandler.Shutdown();

    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, SimpleCommandSendsToGroupTarget)
{
    // No group keys are provisioned, so each send fails and is logged; the calls must still return normally.
    const GroupBindingTarget target{
        .exchangeManager = GetExchangeManager(),
        .fabricIndex     = GetAliceFabricIndex(),
        .groupId         = 0x0101,
    };

    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> onOffCmd(Clusters::OnOff::Commands::Toggle::Type{});
    EXPECT_EQ(onOffCmd.GetClusterId(), Clusters::OnOff::Id);
    onOffCmd.Dispatch(target);

    Clusters::Identify::Commands::Identify::Type identifyCmd;
    identifyCmd.identifyTime = 5;
    SimpleBindingCommand<Clusters::Identify::Commands::Identify::Type> identifyBindingCmd(std::move(identifyCmd));
    EXPECT_EQ(identifyBindingCmd.GetClusterId(), Clusters::Identify::Id);
    identifyBindingCmd.Dispatch(target);

    Clusters::Chime::Commands::PlayChimeSound::Type chimeCmd;
    chimeCmd.chimeID.SetValue(1);
    SimpleBindingCommand<Clusters::Chime::Commands::PlayChimeSound::Type> chimeBindingCmd(chimeCmd);
    EXPECT_EQ(chimeBindingCmd.GetClusterId(), Clusters::Chime::Id);
    chimeBindingCmd.Dispatch(target);
}

} // namespace
