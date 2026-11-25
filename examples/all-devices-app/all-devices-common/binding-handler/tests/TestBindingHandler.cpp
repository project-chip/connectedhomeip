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
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/StatusResponse.h>
#include <binding-handler/BindingCommand.h>
#include <binding-handler/BindingHandler.h>
#include <binding-handler/SimpleBindingCommand.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Ids.h>
#include <clusters/ClosureControl/Commands.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Defer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/interaction_model/Constants.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <optional>

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

class GatedSessionSetupPool : public OperationalSessionSetupPool<2>
{
public:
    bool enabled = true;
    OperationalSessionSetup * Allocate(const CASEClientInitParams & params, CASEClientPoolDelegate * clients, ScopedNodeId peer,
                                       OperationalSessionReleaseDelegate * delegate) override
    {
        return enabled ? OperationalSessionSetupPool<2>::Allocate(params, clients, peer, delegate) : nullptr;
    }
};

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
    GatedSessionSetupPool mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Clusters::Binding::Manager mBindingManager;
    BindingHandler mHandler;
};

struct CompletionRecord
{
    int count        = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;
    Clusters::Binding::TableEntry target;

    BindingCommandCallback Callback()
    {
        return { [](void * context, const Clusters::Binding::TableEntry & binding, CHIP_ERROR result) {
                    auto & record = *static_cast<CompletionRecord *>(context);
                    ++record.count;
                    record.error  = result;
                    record.target = binding;
                },
                 this };
    }
};

TEST_F(TestBindingHandlerWithManager, ConflictingCommandRejectsAllTargetsAndPreservesFirst)
{
    auto & table = mBindingManager.GetBindingTable();
    // The group precedes the busy unicast entry, so preflight must happen before iterating sends.
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForGroup(200, 1, kLocalEndpoint, kTestClusterId)));
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(200, 0x1234, kLocalEndpoint, 2, kTestClusterId)));
    const uint8_t unicastIndex = (++table.begin()).GetIndex();
    int destroyed              = 0;
    DispatchRecord first;
    DispatchRecord rejected;
    auto cleanup = MakeDefer([&] { EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(unicastIndex)); });
    ASSERT_SUCCESS(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destroyed, &first)));
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(destroyed, 0);
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destroyed, &rejected)), CHIP_ERROR_BUSY);
    EXPECT_EQ(rejected.count, 0);
    EXPECT_EQ(destroyed, 1);
    EXPECT_EQ(first.count, 1);
}

TEST_F(TestBindingHandlerWithManager, ExpiredCommandReportsTimeoutOnceAndFreesBinding)
{
    System::Clock::Internal::RAIIMockClock clock;
    auto & table = mBindingManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(200, 0x1234, kLocalEndpoint, 2, Clusters::OnOff::Id)));
    const uint8_t index = table.begin().GetIndex();
    CompletionRecord first;
    CompletionRecord second;
    auto cleanup = MakeDefer([&] { EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(index)); });
    BindingCommandOptions options{ first.Callback(), System::Clock::Milliseconds32(1) };
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, options));
    options.onComplete = second.Callback();
    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, options), CHIP_ERROR_BUSY);
    EXPECT_EQ(second.count, 0);
    clock.AdvanceMonotonic(System::Clock::Milliseconds32(2));
    DrainAndServiceIO();
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(first.error, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(first.target.nodeId, 0x1234u);
    EXPECT_EQ(first.target.remote, 2);
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, options));
    EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(index));
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(second.count, 1);
    EXPECT_EQ(second.error, CHIP_ERROR_CANCELLED);
    clock.AdvanceMonotonic(System::Clock::Milliseconds32(2));
    DrainAndServiceIO();
    EXPECT_EQ(second.count, 1);
}

TEST_F(TestBindingHandlerWithManager, CompletionReportsEachRemovedTarget)
{
    auto & table = mBindingManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(200, 0x1234, kLocalEndpoint, 2, Clusters::OnOff::Id)));
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(200, 0x5678, kLocalEndpoint, 3, Clusters::OnOff::Id)));
    const uint8_t first  = table.begin().GetIndex();
    const uint8_t second = (++table.begin()).GetIndex();
    CompletionRecord record;
    auto cleanup = MakeDefer([&] {
        EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(first));
        EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(second));
    });
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, { record.Callback() }));
    EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(first));
    EXPECT_EQ(record.count, 1);
    EXPECT_EQ(record.target.nodeId, 0x1234u);
    EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(second));
    EXPECT_EQ(record.count, 2);
    EXPECT_EQ(record.target.nodeId, 0x5678u);
    EXPECT_EQ(record.error, CHIP_ERROR_CANCELLED);
}

TEST_F(TestBindingHandlerWithManager, BindingAndFabricRemovalRejectReentrantSubmission)
{
    auto & table = mBindingManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(200, 0x1234, kLocalEndpoint, 2, Clusters::OnOff::Id)));
    struct Record
    {
        BindingHandler & handler;
        int count            = 0;
        CHIP_ERROR error     = CHIP_NO_ERROR;
        CHIP_ERROR admission = CHIP_NO_ERROR;
    } record{ mHandler };
    BindingCommandOptions options;
    options.onComplete = {
        [](void * context, const Clusters::Binding::TableEntry & binding, CHIP_ERROR error) {
            auto & result = *static_cast<Record *>(context);
            ++result.count;
            result.error = error;
            EXPECT_EQ(binding.nodeId, 0x1234u);
            result.admission = result.handler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{});
        },
        &record,
    };
    auto cleanup = MakeDefer([&] { EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(table.begin().GetIndex())); });
    for (bool removeFabric : { false, true })
    {
        record.count = 0;
        ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, options));
        if (removeFabric)
        {
            mBindingManager.FabricRemoved(200);
        }
        else
        {
            EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(table.begin().GetIndex()));
        }
        EXPECT_EQ(record.count, 1);
        EXPECT_EQ(record.error, CHIP_ERROR_CANCELLED);
        EXPECT_EQ(record.admission, CHIP_ERROR_INCORRECT_STATE);
    }
}

TEST_F(TestBindingHandlerWithManager, ManagerDestructionCancelsPendingCommandAndItsTimer)
{
    System::Clock::Internal::RAIIMockClock clock;
    CompletionRecord record;
    TestPersistentStorageDelegate storage;
    {
        Clusters::Binding::Manager manager;
        BindingHandler handler;
        auto removeDelegate = MakeDefer([&] { GetFabricTable().RemoveFabricDelegate(&manager); });
        ASSERT_SUCCESS(handler.Init(manager, { &GetFabricTable(), &mCASESessionManager, &storage }, GetExchangeManager()));
        ASSERT_SUCCESS(manager.GetBindingTable().Add(
            Clusters::Binding::TableEntry::ForNode(200, 0x1234, kLocalEndpoint, 2, Clusters::OnOff::Id)));
        ASSERT_SUCCESS(handler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{},
                                              { record.Callback(), System::Clock::Milliseconds32(1) }));
        EXPECT_EQ(record.count, 0);
        handler.Shutdown();
    }
    EXPECT_EQ(record.count, 1);
    EXPECT_EQ(record.error, CHIP_ERROR_CANCELLED);
    clock.AdvanceMonotonic(System::Clock::Milliseconds32(2));
    DrainAndServiceIO();
    EXPECT_EQ(record.count, 1);
}

TEST_F(TestBindingHandlerWithManager, GroupCompletionReportsLocalSendFailure)
{
    ASSERT_SUCCESS(mBindingManager.GetBindingTable().Add(
        Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 7, kLocalEndpoint, Clusters::OnOff::Id)));
    CompletionRecord record;
    // Without group keys the local send fails; acceptance still succeeds and reports the target error.
    EXPECT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, { record.Callback() }));
    EXPECT_EQ(record.count, 1);
    EXPECT_NE(record.error, CHIP_NO_ERROR);
    EXPECT_EQ(record.target.groupId, 7);
    EXPECT_EQ(record.target.fabricIndex, GetAliceFabricIndex());
}

TEST_F(TestBindingHandlerWithManager, InvalidTimeoutRejectsWithoutCompletion)
{
    CompletionRecord record;
    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{},
                                      { record.Callback(), System::Clock::kZero }),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{},
                                      { record.Callback(), System::Clock::Seconds16(30), System::Clock::kZero }),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(record.count, 0);
}

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
// the device does not know, so the address lookup cannot start), the manager keeps the queued command
// and reports success, so Dispatch must not destroy the command.
TEST_F(TestBindingHandlerWithManager, CommandStaysQueuedWhenSessionSetupFails)
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

    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);

    // The pending notification still owns the command: it must not have been destroyed.
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 0);

    // Removing the binding drops the last reference; the release handler destroys the command exactly once.
    EXPECT_EQ(mBindingManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(record.count, 0);
    EXPECT_EQ(destructorCount, 1);
}

TEST_F(TestBindingHandlerWithManager, CommandIsDestroyedOnceWhenManagerRejectsIt)
{
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Clusters::Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0106, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);

    // Without a BoundDeviceChangedHandler the manager rejects the notification and does not take the context.
    mBindingManager.RegisterBoundDeviceChangedHandler(nullptr);

    int destructorCount = 0;
    DispatchRecord record;
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, MakeTestCommand(destructorCount, &record)), CHIP_ERROR_HANDLER_NOT_SET);
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
    EXPECT_EQ(mHandler.Dispatch(kLocalEndpoint, std::move(command)), CHIP_NO_ERROR);
    EXPECT_EQ(destructorCount, 0);

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

/** Records the first Interaction Model message type the peer receives and rejects it with a failure status. */
class InteractionModelMessageRecorder : public Messaging::UnsolicitedMessageHandler, public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        if (!firstMessageType.has_value())
        {
            firstMessageType = payloadHeader.GetMessageType();
        }
        return StatusResponse::Send(Protocols::InteractionModel::Status::Failure, ec, false /* aExpectResponse */);
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}

    std::optional<uint8_t> firstMessageType;
};

class TogglePeer : public Messaging::UnsolicitedMessageHandler, public Messaging::ExchangeDelegate
{
public:
    int received = 0;
    bool on      = false;
    bool respond = true;

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader &, ExchangeDelegate *& delegate) override
    {
        delegate = this;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchange, const PayloadHeader & header,
                                 System::PacketBufferHandle && payload) override
    {
        VerifyOrReturnError(header.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest),
                            CHIP_ERROR_INVALID_MESSAGE_TYPE);
        System::PacketBufferTLVReader reader;
        reader.Init(std::move(payload));
        InvokeRequestMessage::Parser request;
        ReturnErrorOnFailure(request.Init(reader));
        InvokeRequests::Parser commands;
        ReturnErrorOnFailure(request.GetInvokeRequests(&commands));
        ReturnErrorOnFailure(commands.Next());
        commands.GetReader(&reader);
        CommandDataIB::Parser command;
        ReturnErrorOnFailure(command.Init(reader));
        CommandPathIB::Parser path;
        ReturnErrorOnFailure(command.GetPath(&path));
        EndpointId endpoint;
        ClusterId cluster;
        CommandId id;
        ReturnErrorOnFailure(path.GetEndpointId(&endpoint));
        ReturnErrorOnFailure(path.GetClusterId(&cluster));
        ReturnErrorOnFailure(path.GetCommandId(&id));
        VerifyOrReturnError(cluster == Clusters::OnOff::Id && id == Clusters::OnOff::Commands::Toggle::Id,
                            CHIP_ERROR_INVALID_ARGUMENT);
        ++received;
        on = !on;
        VerifyOrReturnError(respond, CHIP_NO_ERROR);
        auto buffer = System::PacketBufferHandle::New(256);
        VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);
        System::PacketBufferTLVWriter writer;
        writer.Init(std::move(buffer));
        InvokeResponseMessage::Builder response;
        ReturnErrorOnFailure(response.Init(&writer));
        response.SuppressResponse(true);
        auto & responses = response.CreateInvokeResponses();
        auto & item      = responses.CreateInvokeResponse();
        auto & status    = item.CreateStatus();
        ReturnErrorOnFailure(status.CreatePath().EndpointId(endpoint).ClusterId(cluster).CommandId(id).EndOfCommandPathIB());
        ReturnErrorOnFailure(
            status.CreateErrorStatus().EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::Success)).GetError());
        ReturnErrorOnFailure(status.EndOfCommandStatusIB());
        ReturnErrorOnFailure(item.EndOfInvokeResponseIB());
        ReturnErrorOnFailure(responses.EndOfInvokeResponses());
        ReturnErrorOnFailure(response.EndOfInvokeResponseMessage());
        ReturnErrorOnFailure(writer.Finalize(&buffer));
        return exchange->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse, std::move(buffer));
    }
    void OnResponseTimeout(Messaging::ExchangeContext *) override {}
};

TEST_F(TestBindingHandlerWithManager, DeferredToggleSurvivesRejectionAndRetryReachesPeer)
{
    TogglePeer peer;
    ASSERT_SUCCESS(GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, &peer));
    auto unregister = MakeDefer([&] {
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id));
    });
    auto & table    = mBindingManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(GetBobFabricIndex(), GetAliceFabric()->GetNodeId(),
                                                                    kLocalEndpoint, 2, Clusters::OnOff::Id)));
    CompletionRecord first;
    CompletionRecord second;
    const uint8_t index       = table.begin().GetIndex();
    auto cleanup              = MakeDefer([&] { EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(index)); });
    mSessionSetupPool.enabled = false;
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, { first.Callback() }));
    EXPECT_EQ(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, { second.Callback() }),
              CHIP_ERROR_BUSY);
    DrainAndServiceIO();
    EXPECT_EQ(peer.received, 0);
    EXPECT_EQ(first.count, 0);
    EXPECT_EQ(second.count, 0);
    // Resource exhaustion and legacy notifications must not discard the accepted command.
    int legacyContext = 0;
    EXPECT_EQ(mBindingManager.NotifyBoundClusterChanged(kLocalEndpoint, Clusters::OnOff::Id, &legacyContext), CHIP_ERROR_BUSY);
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    ASSERT_SUCCESS(CreateCASESessionBobToAlice());
    ASSERT_SUCCESS(CreateCASESessionAliceToBob());
    mSessionSetupPool.enabled = true;
    ASSERT_SUCCESS(mBindingManager.UnicastBindingCreated(GetBobFabricIndex(), GetAliceFabric()->GetNodeId()));
    DrainAndServiceIO();
    EXPECT_EQ(peer.received, 1);
    EXPECT_TRUE(peer.on);
    EXPECT_EQ(first.count, 1);
    EXPECT_SUCCESS(first.error);
    EXPECT_EQ(first.target, table.GetAt(index));
    EXPECT_EQ(second.count, 0);
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{}, { second.Callback() }));
    DrainAndServiceIO();
    EXPECT_EQ(peer.received, 2);
    EXPECT_FALSE(peer.on);
    EXPECT_EQ(first.count, 1);
    EXPECT_EQ(second.count, 1);
    EXPECT_SUCCESS(second.error);
}

TEST_F(TestBindingHandlerWithManager, LateSessionCannotSendExpiredCommandBeforeTimerRuns)
{
    System::Clock::Internal::RAIIMockClock clock;
    CompletionRecord record;
    auto & table = mBindingManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Clusters::Binding::TableEntry::ForNode(GetBobFabricIndex(), GetAliceFabric()->GetNodeId(),
                                                                    kLocalEndpoint, 2, Clusters::OnOff::Id)));
    const uint8_t index       = table.begin().GetIndex();
    auto cleanup              = MakeDefer([&] { EXPECT_SUCCESS(mBindingManager.UnicastBindingRemoved(index)); });
    mSessionSetupPool.enabled = false;
    ASSERT_SUCCESS(mHandler.DispatchSimple(kLocalEndpoint, Clusters::OnOff::Commands::Toggle::Type{},
                                           { record.Callback(), System::Clock::Milliseconds32(1) }));
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    ASSERT_SUCCESS(CreateCASESessionBobToAlice());
    ASSERT_SUCCESS(CreateCASESessionAliceToBob());
    clock.AdvanceMonotonic(System::Clock::Milliseconds32(2));
    mSessionSetupPool.enabled = true;
    ASSERT_SUCCESS(mBindingManager.UnicastBindingCreated(GetBobFabricIndex(), GetAliceFabric()->GetNodeId()));
    EXPECT_EQ(record.count, 1);
    EXPECT_EQ(record.error, CHIP_ERROR_TIMEOUT);
    DrainAndServiceIO();
    EXPECT_EQ(record.count, 1);
}

TEST_F(TestBindingHandlerWithManager, ResponseTimeoutReportsFailureWithoutReplayingToggle)
{
    System::Clock::Internal::RAIIMockClock clock;
    TogglePeer peer;
    peer.respond = false;
    ASSERT_SUCCESS(GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, &peer));
    auto unregister = MakeDefer([&] {
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id));
    });
    CompletionRecord record;
    const SessionHandle session = GetSessionBobToAlice();
    {
        SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> command(
            Clusters::OnOff::Commands::Toggle::Type{},
            { record.Callback(), System::Clock::Seconds16(30), System::Clock::Milliseconds32(5) });
        command.Dispatch(UnicastBindingTarget{ GetExchangeManager(), session, 2 });
    }
    DrainAndServiceIO();
    EXPECT_EQ(peer.received, 1);
    clock.AdvanceMonotonic(System::Clock::Milliseconds32(6));
    DrainAndServiceIO();
    EXPECT_EQ(record.count, 1);
    EXPECT_EQ(record.error, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(peer.received, 1);
    EXPECT_TRUE(peer.on);
}

TEST_F(TestBindingHandlerWithManager, TimedSimpleCommandSendsTimedRequestToUnicastTarget)
{
    InteractionModelMessageRecorder recorder;
    ASSERT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, &recorder),
              CHIP_NO_ERROR);
    auto unregisterRecorder = MakeDefer([&] {
        EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id),
                  CHIP_NO_ERROR);
    });

    const SessionHandle session = GetSessionBobToAlice();
    const UnicastBindingTarget target{
        .exchangeManager = GetExchangeManager(),
        .session         = session,
        .endpoint        = 1,
    };

    SimpleBindingCommand<Clusters::ClosureControl::Commands::Calibrate::Type> command(
        Clusters::ClosureControl::Commands::Calibrate::Type{});
    command.Dispatch(target);
    DrainAndServiceIO();

    EXPECT_EQ(recorder.firstMessageType, to_underlying(Protocols::InteractionModel::MsgType::TimedRequest));
}

TEST_F(TestBindingHandlerWithManager, ResponseCompletionSurvivesCommandDestruction)
{
    InteractionModelMessageRecorder recorder;
    ASSERT_SUCCESS(GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, &recorder));
    auto unregister = MakeDefer([&] {
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id));
    });
    CompletionRecord record;
    const SessionHandle session = GetSessionBobToAlice();
    {
        SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> command(Clusters::OnOff::Commands::Toggle::Type{},
                                                                              { record.Callback() });
        command.Dispatch(UnicastBindingTarget{ GetExchangeManager(), session, 2 });
        EXPECT_EQ(record.count, 0);
    }
    // The peer deliberately rejects the Invoke with Failure; completion must preserve that failure.
    DrainAndServiceIO();
    EXPECT_EQ(record.count, 1);
    EXPECT_NE(record.error, CHIP_NO_ERROR);
    EXPECT_EQ(record.target.remote, 2);
}

TEST_F(TestBindingHandlerWithManager, TimedSimpleCommandSkipsGroupTarget)
{
    // Group keys let an untimed group command reach the transport, so a send count that stays unchanged
    // means the timed command was skipped rather than failing later in the send path.
    TestPersistentStorageDelegate groupStorage;
    Crypto::DefaultSessionKeystore keystore;
    Credentials::GroupDataProviderImpl groupDataProvider;
    groupDataProvider.SetStorageDelegate(&groupStorage);
    groupDataProvider.SetSessionKeystore(&keystore);
    ASSERT_EQ(groupDataProvider.Init(), CHIP_NO_ERROR);
    Credentials::SetGroupDataProvider(&groupDataProvider);
    auto clearGroupDataProvider = MakeDefer([&] {
        Credentials::SetGroupDataProvider(nullptr);
        groupDataProvider.Finish();
    });

    uint8_t compressedFabricIdBuffer[sizeof(CompressedFabricId)];
    MutableByteSpan compressedFabricId(compressedFabricIdBuffer);
    ASSERT_EQ(GetBobFabric()->GetCompressedFabricIdBytes(compressedFabricId), CHIP_NO_ERROR);
    ASSERT_EQ(GroupTesting::InitData(&groupDataProvider, GetBobFabricIndex(), compressedFabricId), CHIP_NO_ERROR);

    const GroupBindingTarget target{
        .exchangeManager = GetExchangeManager(),
        .fabricIndex     = GetBobFabricIndex(),
        .groupId         = GetFriendsGroupId(),
    };
    auto & loopback = GetLoopback();

    SimpleBindingCommand<Clusters::OnOff::Commands::Toggle::Type> untimedCommand(Clusters::OnOff::Commands::Toggle::Type{});
    const uint32_t sentBeforeUntimed = loopback.mSentMessageCount;
    untimedCommand.Dispatch(target);
    ASSERT_GT(loopback.mSentMessageCount, sentBeforeUntimed);

    SimpleBindingCommand<Clusters::ClosureControl::Commands::Calibrate::Type> timedCommand(
        Clusters::ClosureControl::Commands::Calibrate::Type{});
    const uint32_t sentBeforeTimed = loopback.mSentMessageCount;
    timedCommand.Dispatch(target);
    EXPECT_EQ(loopback.mSentMessageCount, sentBeforeTimed);

    DrainAndServiceIO();
}

} // namespace
