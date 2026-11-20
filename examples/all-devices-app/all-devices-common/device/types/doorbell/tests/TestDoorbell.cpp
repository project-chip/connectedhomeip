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

#include <device/types/doorbell/Doorbell.h>
#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <binding-handler/BindingHandler.h>
#include <clusters/Chime/Ids.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimerDelegateMock.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kDoorbellEndpoint = 1;

class MockIdentifyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override {}
    void OnIdentifyStop(IdentifyCluster & cluster) override {}
    void OnTriggerEffect(IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return false; }
};

/**
 * Doorbell fixture with a BindingHandler initialized on a real Binding::Manager, backed by a
 * loopback messaging stack and a real CASESessionManager.
 */
class TestDoorbell : public chip::Testing::LoopbackMessagingContext
{
public:
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

        ASSERT_EQ(
            mBindingHandler.Init(mBindingManager, { &GetFabricTable(), &mCASESessionManager, &mStorage }, GetExchangeManager()),
            CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mBindingHandler.Shutdown();
        GetFabricTable().RemoveFabricDelegate(&mBindingManager);
        mCASESessionManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

protected:
    Doorbell::Config MakeConfig(BindingHandler & bindingHandler)
    {
        return Doorbell::Config{
            .timerDelegate    = mTimerDelegate,
            .platformManager  = DeviceLayer::PlatformMgr(),
            .bindingTable     = mBindingManager.GetBindingTable(),
            .bindingManager   = mBindingManager,
            .bindingHandler   = bindingHandler,
            .identifyDelegate = mIdentifyDelegate,
        };
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    MockIdentifyDelegate mIdentifyDelegate;

    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    CASEClientPool<2> mCASEClientPool;
    OperationalSessionSetupPool<2> mSessionSetupPool;
    CASESessionManager mCASESessionManager;
    Binding::Manager mBindingManager;
    BindingHandler mBindingHandler;
};

TEST_F(TestDoorbell, UnregisteredActionsFail)
{
    Doorbell doorbell(MakeConfig(mBindingHandler));

    // Prior to registration there is no endpoint to send from, even though the binding handler is initialized.
    EXPECT_EQ(doorbell.TriggerChime(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestDoorbell, RegistrationExposesChimeClientCluster)
{
    Doorbell doorbell(MakeConfig(mBindingHandler));

    ASSERT_EQ(doorbell.Register(kDoorbellEndpoint, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.GetEndpointId(), kDoorbellEndpoint);

    ReadOnlyBufferBuilder<ClusterId> clientClusters;
    EXPECT_EQ(doorbell.ClientClusters(clientClusters), CHIP_NO_ERROR);
    auto buffer = clientClusters.TakeBuffer();
    ASSERT_EQ(buffer.size(), 1U);
    EXPECT_EQ(buffer[0], Chime::Id);

    doorbell.Unregister(mProvider);
}

TEST_F(TestDoorbell, ShortPressWithoutBindingSucceeds)
{
    Doorbell doorbell(MakeConfig(mBindingHandler));
    ASSERT_EQ(doorbell.Register(kDoorbellEndpoint, mProvider), CHIP_NO_ERROR);

    // With no binding configured there is nothing to send: the chime dispatch is a successful no-op.
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.SwitchCluster().GetCurrentPosition(), 0U);

    doorbell.Unregister(mProvider);
}

TEST_F(TestDoorbell, TriggerChimeDispatchesToGroupBinding)
{
    Doorbell doorbell(MakeConfig(mBindingHandler));
    ASSERT_EQ(doorbell.Register(kDoorbellEndpoint, mProvider), CHIP_NO_ERROR);

    // No group keys are provisioned, so the group send itself fails and is only logged.
    ASSERT_EQ(mBindingManager.GetBindingTable().Add(
                  Binding::TableEntry::ForGroup(GetAliceFabricIndex(), 0x0101 /* group */, kDoorbellEndpoint, Chime::Id)),
              CHIP_NO_ERROR);

    EXPECT_EQ(doorbell.TriggerChime(), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.TriggerChime(3), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.SwitchCluster().GetCurrentPosition(), 0U);

    doorbell.Unregister(mProvider);
}

TEST_F(TestDoorbell, ShortPressResetsPositionWhenBindingHandlerIsNotInitialized)
{
    BindingHandler uninitializedHandler;
    Doorbell doorbell(MakeConfig(uninitializedHandler));
    ASSERT_EQ(doorbell.Register(kDoorbellEndpoint, mProvider), CHIP_NO_ERROR);

    // The chime dispatch error is reported, but the switch position must still be restored to 0.
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(doorbell.SwitchCluster().GetCurrentPosition(), 0U);

    doorbell.Unregister(mProvider);
}

} // namespace
