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

#pragma once

#include "FakeOpenThread.h"

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <vector>

namespace chip {
namespace Testing {

namespace NetworkCommissioning = DeviceLayer::NetworkCommissioning;

class ScanCallbackSpy : public NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    void OnFinished(NetworkCommissioning::Status status, CharSpan, NetworkCommissioning::ThreadScanResponseIterator *) override
    {
        mStatuses.push_back(status);
    }

    size_t FinishedCount() const { return mStatuses.size(); }

    std::vector<NetworkCommissioning::Status> mStatuses;
};

class TestThreadStackManager : public DeviceLayer::Internal::GenericThreadStackManagerImpl_OpenThread<TestThreadStackManager>
{
public:
    CHIP_ERROR Init() { return ConfigureThreadStack(FakeOpenThreadInstance()); }
    CHIP_ERROR StartScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback) { return _StartThreadScan(callback); }
    // OpenThread runs the discover callback from its tasklets, with the Thread stack lock held.
    void CompleteScan()
    {
        LockThreadStack();
        _OnNetworkScanFinished(nullptr, this);
        UnlockThreadStack();
    }
    CHIP_ERROR SetThreadEnabled(bool enabled) { return _SetThreadEnabled(enabled); }
    CHIP_ERROR SetThreadProvision(ByteSpan) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    void ErasePersistentInfo() { _ErasePersistentInfo(); }

    static void OnOpenThreadStateChange(uint32_t, void *) {}
    void LockThreadStack()
    {
        FakeOpenThread().threadStackLockDepth++;
        FakeOpenThread().threadStackLockCount++;
    }
    void UnlockThreadStack() { FakeOpenThread().threadStackLockDepth--; }
};

inline otLinkModeConfig MakeLinkMode(bool rxOnWhenIdle, bool deviceType, bool networkData)
{
    otLinkModeConfig mode = {};
    mode.mRxOnWhenIdle    = rxOnWhenIdle;
    mode.mDeviceType      = deviceType;
    mode.mNetworkData     = networkData;
    return mode;
}

inline bool SameLinkMode(const otLinkModeConfig & a, const otLinkModeConfig & b)
{
    return a.mRxOnWhenIdle == b.mRxOnWhenIdle && a.mDeviceType == b.mDeviceType && a.mNetworkData == b.mNetworkData;
}

inline std::vector<bool> RxOnWrites()
{
    std::vector<bool> bits;
    for (const otLinkModeConfig & config : FakeOpenThread().setLinkModeLog)
    {
        bits.push_back(config.mRxOnWhenIdle);
    }
    return bits;
}

class ThreadScanTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        static DeviceLayer::TestOnlyCommissionableDataProvider commissionableDataProvider;
        DeviceLayer::SetCommissionableDataProvider(&commissionableDataProvider);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    static CHIP_ERROR StartScan(TestThreadStackManager & manager, NetworkCommissioning::ThreadDriver::ScanCallback * callback)
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        CHIP_ERROR err = manager.StartScan(callback);
        DeviceLayer::PlatformMgr().UnlockChipStack();
        return err;
    }

    static void CompleteScan(TestThreadStackManager & manager)
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        manager.CompleteScan();
        DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    static CHIP_ERROR SetThreadEnabled(TestThreadStackManager & manager, bool enabled)
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        CHIP_ERROR err = manager.SetThreadEnabled(enabled);
        DeviceLayer::PlatformMgr().UnlockChipStack();
        return err;
    }

    static void ErasePersistentInfo(TestThreadStackManager & manager)
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        manager.ErasePersistentInfo();
        DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    static void DrainEvents()
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        CHIP_ERROR err =
            DeviceLayer::SystemLayer().ScheduleLambda([] { EXPECT_SUCCESS(DeviceLayer::PlatformMgr().StopEventLoopTask()); });
        DeviceLayer::PlatformMgr().UnlockChipStack();
        ASSERT_EQ(err, CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

    static void MakeUncommissionedSleepyEndDevice()
    {
        FakeOpenThread().linkMode   = MakeLinkMode(/* rxOnWhenIdle = */ false, /* deviceType = */ false, /* networkData = */ true);
        FakeOpenThread().ip6Enabled = false;
        FakeOpenThread().role       = OT_DEVICE_ROLE_DISABLED;
        FakeOpenThread().commissioned = false;
    }

protected:
    void SetUp() override
    {
        ResetFakeOpenThread();
        ASSERT_EQ(mManager.Init(), CHIP_NO_ERROR);
    }

    void TearDown() override { DrainEvents(); }

    CHIP_ERROR StartScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback) { return StartScan(mManager, callback); }
    void CompleteScan() { CompleteScan(mManager); }
    CHIP_ERROR SetThreadEnabled(bool enabled) { return SetThreadEnabled(mManager, enabled); }
    void ErasePersistentInfo() { ErasePersistentInfo(mManager); }

    TestThreadStackManager mManager;
    ScanCallbackSpy mCallback;
};

} // namespace Testing
} // namespace chip
