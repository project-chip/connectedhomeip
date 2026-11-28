/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/clusters/bindings/BindingManager.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/Defer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kLocalEndpoint  = 1;
constexpr ClusterId kTestClusterId   = 0x0006;
constexpr FabricIndex kUnknownFabric = 200;

// The handlers are plain function pointers, so the observations are kept in file-scope state.
struct HandlerRecord
{
    int changedCount        = 0;
    int releaseCount        = 0;
    void * lastContext      = nullptr;
    void * lastReleased     = nullptr;
    bool lastPeerWasNullptr = false;
};

HandlerRecord gRecord;

void TestBoundDeviceChangedHandler(const Binding::TableEntry &, OperationalDeviceProxy * peer_device, void * context)
{
    gRecord.changedCount++;
    gRecord.lastContext        = context;
    gRecord.lastPeerWasNullptr = (peer_device == nullptr);
}

void TestContextReleaseHandler(void * context)
{
    gRecord.releaseCount++;
    gRecord.lastReleased = context;
}

/**
 * Fixture with a Binding::Manager initialized against a loopback messaging stack and a real
 * CASESessionManager. No peer is reachable, so unicast session setup never completes in these tests.
 */
class TestBindingManager : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        gRecord = HandlerRecord{};

        CASESessionManagerConfig config;
        config.sessionInitParams.sessionManager    = &GetSecureSessionManager();
        config.sessionInitParams.exchangeMgr       = &GetExchangeManager();
        config.sessionInitParams.fabricTable       = &GetFabricTable();
        config.sessionInitParams.groupDataProvider = &mGroupDataProvider;
        config.clientPool                          = &mCASEClientPool;
        config.sessionSetupPool                    = &mSessionSetupPool;
        ASSERT_EQ(mCASESessionManager.Init(&GetSystemLayer(), config), CHIP_NO_ERROR);

        mManager.RegisterBoundDeviceChangedHandler(TestBoundDeviceChangedHandler);
        mManager.RegisterBoundDeviceContextReleaseHandler(TestContextReleaseHandler);
        ASSERT_EQ(mManager.Init({ &GetFabricTable(), &mCASESessionManager, &mStorage, false /* establishConnectionOnInit */ }),
                  CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        GetFabricTable().RemoveFabricDelegate(&mManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Binding::Manager mManager;
    int mContext = 0;
};

class PendingResolver : public Dnssd::Resolver
{
public:
    PendingResolver() : mPrevious(Dnssd::Resolver::Instance()) { Dnssd::Resolver::SetInstance(*this); }
    ~PendingResolver() override { Dnssd::Resolver::SetInstance(mPrevious); }
    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> *) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(Dnssd::OperationalResolveDelegate *) override {}
    CHIP_ERROR ResolveNodeId(const PeerId &) override
    {
        ++lookups;
        return CHIP_NO_ERROR;
    }
    void NodeIdResolutionNoLongerNeeded(const PeerId &) override {}
    CHIP_ERROR StartDiscovery(Dnssd::DiscoveryType, Dnssd::DiscoveryFilter, Dnssd::DiscoveryContext &) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(Dnssd::DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char *, Inet::IPAddress, Inet::InterfaceId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    int lookups = 0;

private:
    Dnssd::Resolver & mPrevious;
};

struct ConnectionRecord
{
    int connected    = 0;
    int failed       = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;
    static void Connected(void * context, Messaging::ExchangeManager &, const SessionHandle &)
    {
        ++static_cast<ConnectionRecord *>(context)->connected;
    }
    static void Failed(void * context, const ScopedNodeId &, CHIP_ERROR error)
    {
        auto & record = *static_cast<ConnectionRecord *>(context);
        ++record.failed;
        record.error = error;
    }
};

TEST_F(TestBindingManager, SharedCaseSetupCanCompleteAfterLocalManagerDestruction)
{
    for (bool succeed : { false, true })
    {
        PendingResolver resolver;
        TestPersistentStorageDelegate storage;
        const ScopedNodeId peer(GetAliceFabric()->GetNodeId(), GetBobFabricIndex());
        auto manager = Platform::MakeUnique<Binding::Manager>();
        ASSERT_NE(manager, nullptr);
        auto cleanup = MakeDefer([&] {
            if (manager != nullptr)
            {
                GetFabricTable().RemoveFabricDelegate(manager.get());
                manager.reset();
            }
            mSessionSetupPool.ReleaseAllSessionSetup();
        });
        ASSERT_SUCCESS(manager->Init({ &GetFabricTable(), &mCASESessionManager, &storage, false }));
        manager->RegisterBoundDeviceChangedHandler(TestBoundDeviceChangedHandler);
        manager->RegisterBoundDeviceContextReleaseHandler(TestContextReleaseHandler);
        ASSERT_SUCCESS(manager->GetBindingTable().Add(
            Binding::TableEntry::ForNode(peer.GetFabricIndex(), peer.GetNodeId(), kLocalEndpoint, 2, kTestClusterId)));
        const int releasedBefore = gRecord.releaseCount;
        ASSERT_SUCCESS(manager->NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext));
        ASSERT_SUCCESS(manager->UnicastBindingCreated(peer.GetFabricIndex(), peer.GetNodeId()));
        ASSERT_SUCCESS(manager->UnicastBindingCreated(peer.GetFabricIndex(), peer.GetNodeId()));
        ASSERT_NE(mSessionSetupPool.FindSessionSetup(peer, false), nullptr);
        EXPECT_EQ(resolver.lookups, 1);

        ConnectionRecord record;
        Callback::Callback<OnDeviceConnected> connected(ConnectionRecord::Connected, &record);
        Callback::Callback<OnDeviceConnectionFailure> failed(ConnectionRecord::Failed, &record);
        mCASESessionManager.FindOrEstablishSession(peer, &connected, &failed);
        EXPECT_EQ(record.connected, 0);
        EXPECT_EQ(record.failed, 0);
        GetFabricTable().RemoveFabricDelegate(manager.get());
        manager.reset();
        EXPECT_EQ(gRecord.releaseCount, releasedBefore + 1);
        auto * setup = mSessionSetupPool.FindSessionSetup(peer, false);
        ASSERT_NE(setup, nullptr);
        if (succeed)
        {
            ExpireSessionBobToAlice();
            ExpireSessionAliceToBob();
            ASSERT_SUCCESS(CreateCASESessionBobToAlice());
            ASSERT_SUCCESS(CreateCASESessionAliceToBob());
            setup->Connect(nullptr, static_cast<Callback::Callback<OnDeviceConnectionFailure> *>(nullptr));
            EXPECT_EQ(record.connected, 1);
            EXPECT_EQ(record.failed, 0);
        }
        else
        {
            // Releasing an unfinished setup reports cancellation to its remaining owner.
            mSessionSetupPool.ReleaseAllSessionSetup();
            EXPECT_EQ(record.connected, 0);
            EXPECT_EQ(record.failed, 1);
            EXPECT_EQ(record.error, CHIP_ERROR_CANCELLED);
        }
        EXPECT_EQ(mSessionSetupPool.FindSessionSetup(peer, false), nullptr);
        EXPECT_EQ(gRecord.changedCount, 0);
        EXPECT_EQ(gRecord.releaseCount, releasedBefore + 1);
    }
}

TEST_F(TestBindingManager, TeardownRejectsCaseRequestsFromContextRelease)
{
    PendingResolver resolver;
    TestPersistentStorageDelegate storage;
    struct Record
    {
        Binding::Manager * manager;
        ScopedNodeId peer;
        int releases         = 0;
        CHIP_ERROR admission = CHIP_NO_ERROR;
    } record{ nullptr, ScopedNodeId(GetAliceFabric()->GetNodeId(), GetBobFabricIndex()) };
    auto local = Platform::MakeUnique<Binding::Manager>();
    ASSERT_NE(local, nullptr);
    ASSERT_SUCCESS(local->Init({ &GetFabricTable(), &mCASESessionManager, &storage, false }));
    record.manager = local.get();
    auto cleanup   = MakeDefer([&] {
        if (local != nullptr)
        {
            GetFabricTable().RemoveFabricDelegate(local.get());
            local.reset();
        }
        mSessionSetupPool.ReleaseAllSessionSetup();
    });
    local->RegisterBoundDeviceChangedHandler(TestBoundDeviceChangedHandler);
    local->RegisterBoundDeviceContextReleaseHandler([](void * context) {
        auto & result = *static_cast<Record *>(context);
        ++result.releases;
        result.admission = result.manager->UnicastBindingCreated(result.peer.GetFabricIndex(), result.peer.GetNodeId());
    });
    ASSERT_SUCCESS(local->GetBindingTable().Add(
        Binding::TableEntry::ForNode(record.peer.GetFabricIndex(), record.peer.GetNodeId(), kLocalEndpoint, 2, kTestClusterId)));
    ASSERT_SUCCESS(local->NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &record));
    GetFabricTable().RemoveFabricDelegate(local.get());
    local.reset();
    EXPECT_EQ(record.releases, 1);
    EXPECT_EQ(record.admission, CHIP_ERROR_INCORRECT_STATE);
}

class FailingTimerLayer : public System::Layer
{
public:
    CriticalFailure Init() override { return CHIP_NO_ERROR; }
    void Shutdown() override {}
    bool IsInitialized() const override { return true; }
    CriticalFailure StartTimer(System::Clock::Timeout, System::TimerCompleteCallback, void *) override
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    CHIP_ERROR ExtendTimerTo(System::Clock::Timeout, System::TimerCompleteCallback, void *) override
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    bool IsTimerActive(System::TimerCompleteCallback, void *) override { return false; }
    System::Clock::Timeout GetRemainingTime(System::TimerCompleteCallback, void *) override { return System::Clock::kZero; }
    void CancelTimer(System::TimerCompleteCallback, void *) override {}
    CriticalFailure ScheduleWork(System::TimerCompleteCallback, void *) override { return CHIP_ERROR_NO_MEMORY; }
};

TEST_F(TestBindingManager, TimerAllocationFailureRejectsBeforeSendingAndDoesNotTakeContext)
{
    FailingTimerLayer layer;
    auto & table = mManager.GetBindingTable();
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(kUnknownFabric, 1, kLocalEndpoint, kTestClusterId)));
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForNode(kUnknownFabric, 0x1234, kLocalEndpoint, 2, kTestClusterId)));
    auto failure = +[](const Binding::TableEntry &, void *, CHIP_ERROR) { ADD_FAILURE(); };
    EXPECT_EQ(mManager.DispatchCommand(kLocalEndpoint, kTestClusterId, &mContext, failure, layer), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 0);
    // Removing the binding must not release a rejected context either.
    EXPECT_SUCCESS(mManager.UnicastBindingRemoved((++table.begin()).GetIndex()));
    EXPECT_EQ(gRecord.releaseCount, 0);
}

TEST_F(TestBindingManager, NotInitializedDoesNotTakeContext)
{
    Binding::Manager manager;
    manager.RegisterBoundDeviceChangedHandler(TestBoundDeviceChangedHandler);
    manager.RegisterBoundDeviceContextReleaseHandler(TestContextReleaseHandler);

    EXPECT_EQ(manager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 0);
}

TEST_F(TestBindingManager, MissingChangedHandlerDoesNotTakeContext)
{
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);
    mManager.RegisterBoundDeviceChangedHandler(nullptr);

    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_ERROR_HANDLER_NOT_SET);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 0);
}

TEST_F(TestBindingManager, NoMatchingBindingReleasesContextBeforeReturning)
{
    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 1);
    EXPECT_EQ(gRecord.lastReleased, &mContext);
}

TEST_F(TestBindingManager, GroupBindingIsNotifiedAndContextReleasedOnce)
{
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 1);
    EXPECT_EQ(gRecord.lastContext, &mContext);
    EXPECT_TRUE(gRecord.lastPeerWasNullptr);
    EXPECT_EQ(gRecord.releaseCount, 1);
    EXPECT_EQ(gRecord.lastReleased, &mContext);
}

// Session setup to a binding on an unknown fabric fails synchronously after the notification was queued.
// The manager has taken the context at that point, so it must report success and release the context
// exactly once, when the queued notification is dropped.
TEST_F(TestBindingManager, SessionSetupFailureAfterQueueingKeepsContextOwnedByManager)
{
    ASSERT_EQ(GetFabricTable().FindFabricWithIndex(kUnknownFabric), nullptr);
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForNode(kUnknownFabric, 0x1234 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    const uint8_t bindingIndex = mManager.GetBindingTable().begin().GetIndex();

    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 0);

    EXPECT_EQ(mManager.UnicastBindingRemoved(bindingIndex), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 0);
    EXPECT_EQ(gRecord.releaseCount, 1);
    EXPECT_EQ(gRecord.lastReleased, &mContext);
}

TEST_F(TestBindingManager, MixedBindingsReleaseContextOnceAllConsumersAreDone)
{
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForNode(kUnknownFabric, 0x1234 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 1);
    EXPECT_EQ(gRecord.releaseCount, 0);

    mManager.FabricRemoved(kUnknownFabric);
    EXPECT_EQ(gRecord.releaseCount, 1);
    EXPECT_EQ(gRecord.lastReleased, &mContext);
}

// A unicast binding whose session setup fails synchronously must not stop the notification of the
// bindings that follow it in the table.
TEST_F(TestBindingManager, SessionSetupFailureDoesNotSkipLaterBindings)
{
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForNode(kUnknownFabric, 0x1234 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForNode(kUnknownFabric, 0x5678 /* node */, kLocalEndpoint, 2 /* remote */, kTestClusterId)),
              CHIP_NO_ERROR);
    ASSERT_EQ(mManager.GetBindingTable().Add(
                  Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101, kLocalEndpoint, kTestClusterId)),
              CHIP_NO_ERROR);

    auto iter                        = mManager.GetBindingTable().begin();
    const uint8_t firstUnicastIndex  = iter.GetIndex();
    const uint8_t secondUnicastIndex = (++iter).GetIndex();

    EXPECT_EQ(mManager.NotifyBoundClusterChanged(kLocalEndpoint, kTestClusterId, &mContext), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.changedCount, 1);
    EXPECT_TRUE(gRecord.lastPeerWasNullptr);
    EXPECT_EQ(gRecord.releaseCount, 0);

    // Both unicast notifications were queued, so the context is released only after both are removed.
    EXPECT_EQ(mManager.UnicastBindingRemoved(firstUnicastIndex), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.releaseCount, 0);
    EXPECT_EQ(mManager.UnicastBindingRemoved(secondUnicastIndex), CHIP_NO_ERROR);
    EXPECT_EQ(gRecord.releaseCount, 1);
    EXPECT_EQ(gRecord.lastReleased, &mContext);
}

} // namespace
