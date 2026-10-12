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

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Chime/Ids.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

class MockIdentifyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override {}
    void OnIdentifyStop(IdentifyCluster & cluster) override {}
    void OnTriggerEffect(IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return false; }
};

class TestDoorbell : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    MockIdentifyDelegate mIdentifyDelegate;
};

TEST_F(TestDoorbell, UnregisteredActionsFail)
{
    Doorbell doorbell(Doorbell::Config{
        .timerDelegate    = mTimerDelegate,
        .platformManager  = DeviceLayer::PlatformMgr(),
        .bindingTable     = Binding::Table::GetInstance(),
        .bindingManager   = Binding::Manager::GetInstance(),
        .identifyDelegate = mIdentifyDelegate,
    });

    // Prior to registration, TriggerChime and HandleShortPress must return CHIP_ERROR_INCORRECT_STATE
    EXPECT_EQ(doorbell.TriggerChime(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestDoorbell, RegistrationAndShortPressResetsPosition)
{
    Doorbell doorbell(Doorbell::Config{
        .timerDelegate    = mTimerDelegate,
        .platformManager  = DeviceLayer::PlatformMgr(),
        .bindingTable     = Binding::Table::GetInstance(),
        .bindingManager   = Binding::Manager::GetInstance(),
        .identifyDelegate = mIdentifyDelegate,
    });

    EXPECT_EQ(doorbell.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(doorbell.GetEndpointId(), 1);

    ReadOnlyBufferBuilder<ClusterId> clientClusters;
    EXPECT_EQ(doorbell.ClientClusters(clientClusters), CHIP_NO_ERROR);
    auto buffer = clientClusters.TakeBuffer();
    ASSERT_EQ(buffer.size(), 1U);
    EXPECT_EQ(buffer[0], Chime::Id);

    // Even when Binding::Manager is uninitialized (returning CHIP_ERROR_INCORRECT_STATE from TriggerChime),
    // HandleShortPress must still restore SwitchCluster's current position back to 0.
    EXPECT_EQ(doorbell.HandleShortPress(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(doorbell.SwitchCluster().GetCurrentPosition(), 0U);

    doorbell.Unregister(mProvider);
}

} // namespace
