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
#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/StringBuilderAdapters.h>
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

TEST_F(TestBindingManager, OnFabricRemovedNotifiesBindingClustersForAffectedEndpoints)
{
    constexpr EndpointId kOtherEndpoint = 2;
    const ConcreteClusterPath path1(kLocalEndpoint, Binding::Id);
    const ConcreteClusterPath path2(kOtherEndpoint, Binding::Id);

    Binding::Table & table = mManager.GetBindingTable();
    BindingCluster cluster1(
        BindingCluster::Context{ .bindingTable = table, .bindingManager = mManager, .platformManager = DeviceLayer::PlatformMgr() },
        kLocalEndpoint);
    BindingCluster cluster2(
        BindingCluster::Context{ .bindingTable = table, .bindingManager = mManager, .platformManager = DeviceLayer::PlatformMgr() },
        kOtherEndpoint);

    chip::Testing::ClusterTester tester1(cluster1);
    chip::Testing::ClusterTester tester2(cluster2);
    ASSERT_SUCCESS(cluster1.Startup(tester1.GetServerClusterContext()));
    ASSERT_SUCCESS(cluster2.Startup(tester2.GetServerClusterContext()));

    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101, kLocalEndpoint, kTestClusterId)));
    ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(GetBobFabricIndex(), 0x0102, kOtherEndpoint, kTestClusterId)));

    FabricTable::Delegate & fabricDelegate = mManager;

    // Removing a fabric with no bindings must not notify any cluster.
    tester1.GetDirtyList().clear();
    tester2.GetDirtyList().clear();
    DataVersion version1Before = cluster1.GetDataVersion(path1);
    DataVersion version2Before = cluster2.GetDataVersion(path2);

    fabricDelegate.OnFabricRemoved(GetFabricTable(), kUnknownFabric);
    EXPECT_TRUE(tester1.GetDirtyList().empty());
    EXPECT_EQ(cluster1.GetDataVersion(path1), version1Before);
    EXPECT_TRUE(tester2.GetDirtyList().empty());
    EXPECT_EQ(cluster2.GetDataVersion(path2), version2Before);
    EXPECT_EQ(table.Size(), 2u);

    // Removing Alice's fabric removes only Alice's entry and notifies only endpoint 1.
    fabricDelegate.OnFabricRemoved(GetFabricTable(), GetAliceFabricIndex());
    EXPECT_TRUE(tester1.IsAttributeDirty(Binding::Attributes::Binding::Id));
    EXPECT_NE(cluster1.GetDataVersion(path1), version1Before);
    EXPECT_TRUE(tester2.GetDirtyList().empty());
    EXPECT_EQ(cluster2.GetDataVersion(path2), version2Before);
    ASSERT_EQ(table.Size(), 1u);
    EXPECT_EQ(table.begin()->fabricIndex, GetBobFabricIndex());

    cluster1.Shutdown(ClusterShutdownType::kClusterShutdown);
    cluster2.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
