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

#include "FakeOpenThread.h"

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <openthread/error.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace {

using namespace chip;
using namespace chip::Testing;
namespace NetworkCommissioning = DeviceLayer::NetworkCommissioning;
using NetworkCommissioning::Status;

class ScanCallbackSpy : public NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    void OnFinished(Status status, CharSpan, NetworkCommissioning::ThreadScanResponseIterator *) override
    {
        mStatuses.push_back(status);
    }

    size_t FinishedCount() const { return mStatuses.size(); }

    std::vector<Status> mStatuses;
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

otLinkModeConfig MakeLinkMode(bool rxOnWhenIdle, bool deviceType, bool networkData)
{
    otLinkModeConfig mode = {};
    mode.mRxOnWhenIdle    = rxOnWhenIdle;
    mode.mDeviceType      = deviceType;
    mode.mNetworkData     = networkData;
    return mode;
}

bool SameLinkMode(const otLinkModeConfig & a, const otLinkModeConfig & b)
{
    return a.mRxOnWhenIdle == b.mRxOnWhenIdle && a.mDeviceType == b.mDeviceType && a.mNetworkData == b.mNetworkData;
}

std::vector<bool> RxOnWrites()
{
    std::vector<bool> bits;
    for (const otLinkModeConfig & config : FakeOpenThread().setLinkModeLog)
    {
        bits.push_back(config.mRxOnWhenIdle);
    }
    return bits;
}

long Ip6DisableCalls()
{
    const std::vector<bool> & log = FakeOpenThread().ip6SetEnabledLog;
    return std::count(log.begin(), log.end(), false);
}

void ClearLogs()
{
    FakeOpenThread().setLinkModeLog.clear();
    FakeOpenThread().setLinkModeLockDepth.clear();
    FakeOpenThread().ip6SetEnabledLog.clear();
    FakeOpenThread().ip6SetEnabledLockDepth.clear();
}

void MakeUncommissionedSleepyEndDevice()
{
    FakeOpenThread().linkMode     = MakeLinkMode(/* rxOnWhenIdle = */ false, /* deviceType = */ false, /* networkData = */ true);
    FakeOpenThread().ip6Enabled   = false;
    FakeOpenThread().role         = OT_DEVICE_ROLE_DISABLED;
    FakeOpenThread().commissioned = false;
}

CHIP_ERROR StartScan(TestThreadStackManager & manager, ScanCallbackSpy * callback)
{
    DeviceLayer::PlatformMgr().LockChipStack();
    CHIP_ERROR err = manager.StartScan(callback);
    DeviceLayer::PlatformMgr().UnlockChipStack();
    return err;
}

void CompleteScan(TestThreadStackManager & manager)
{
    DeviceLayer::PlatformMgr().LockChipStack();
    manager.CompleteScan();
    DeviceLayer::PlatformMgr().UnlockChipStack();
}

void DrainEvents()
{
    DeviceLayer::PlatformMgr().LockChipStack();
    CHIP_ERROR err =
        DeviceLayer::SystemLayer().ScheduleLambda([] { EXPECT_SUCCESS(DeviceLayer::PlatformMgr().StopEventLoopTask()); });
    DeviceLayer::PlatformMgr().UnlockChipStack();
    ASSERT_EQ(err, CHIP_NO_ERROR);
    DeviceLayer::PlatformMgr().RunEventLoop();
}

class TestThreadScanStateRestore : public ::testing::Test
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

protected:
    void SetUp() override
    {
        ResetFakeOpenThread();
        ASSERT_EQ(mManager.Init(), CHIP_NO_ERROR);
        MakeUncommissionedSleepyEndDevice();
    }

    void TearDown() override { DrainEvents(); }

    CHIP_ERROR StartScan(ScanCallbackSpy * callback) { return ::StartScan(mManager, callback); }
    void CompleteScan() { ::CompleteScan(mManager); }
    void CompleteScanAndDrain()
    {
        CompleteScan();
        DrainEvents();
    }
    CHIP_ERROR SetThreadEnabled(bool enabled)
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        CHIP_ERROR err = mManager.SetThreadEnabled(enabled);
        DeviceLayer::PlatformMgr().UnlockChipStack();
        return err;
    }
    void ErasePersistentInfo()
    {
        DeviceLayer::PlatformMgr().LockChipStack();
        mManager.ErasePersistentInfo();
        DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    void ExpectRetryAccepted()
    {
        FakeOpenThread().discoverResult   = OT_ERROR_NONE;
        FakeOpenThread().ip6EnableResult  = OT_ERROR_NONE;
        FakeOpenThread().ip6DisableResult = OT_ERROR_NONE;
        ScanCallbackSpy retry;
        ASSERT_EQ(StartScan(&retry), CHIP_NO_ERROR);
        CompleteScanAndDrain();
        EXPECT_EQ(retry.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    }

    TestThreadStackManager mManager;
    ScanCallbackSpy mCallback;
};

constexpr otError kDiscoverErrors[] = { OT_ERROR_FAILED, OT_ERROR_BUSY, OT_ERROR_INVALID_STATE, OT_ERROR_INVALID_ARGS,
                                        OT_ERROR_NO_BUFS };

// ===== A failed or completed scan on an uncommissioned sleepy end device

TEST_F(TestThreadScanStateRestore, FailedStartTurnsIp6BackOffBeforeCommissioning)
{
    unsigned discoverCalls = 0;
    for (otError error : kDiscoverErrors)
    {
        MakeUncommissionedSleepyEndDevice();
        FakeOpenThread().discoverResult = error;

        EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
        EXPECT_EQ(FakeOpenThread().discoverCalls, ++discoverCalls);
        DrainEvents();
        EXPECT_FALSE(FakeOpenThread().ip6Enabled) << "otError " << static_cast<int>(error);
        EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    }
    EXPECT_EQ(mCallback.FinishedCount(), 0u);
}

TEST_F(TestThreadScanStateRestore, FailedStartRestoresIp6BeforeAnyEventRuns)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLockDepth, (std::vector<int>{ 1, 1 }));
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);

    DrainEvents();
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_EQ(mCallback.FinishedCount(), 0u);
}

TEST_F(TestThreadScanStateRestore, CompletedScanTurnsIp6BackOffUnderTheLock)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScan();
    const unsigned locksBeforeEvents = FakeOpenThread().threadStackLockCount;
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().threadStackLockCount, locksBeforeEvents + 1);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLockDepth, (std::vector<int>{ 1, 1 }));
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
}

TEST_F(TestThreadScanStateRestore, RetryAfterFailedStartKeepsIp6UpForTheNewScan)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    FakeOpenThread().discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScanAndDrain();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
}

TEST_F(TestThreadScanStateRestore, SecondScanWhileRunningIsRejectedWithoutTouchingOpenThread)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    const auto linkModeWrites = FakeOpenThread().setLinkModeLog.size();
    const auto ip6Writes      = FakeOpenThread().ip6SetEnabledLog;

    ScanCallbackSpy rejected;
    EXPECT_EQ(StartScan(&rejected), CHIP_ERROR_INCORRECT_STATE);
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().discoverCalls, 1u);
    EXPECT_EQ(FakeOpenThread().setLinkModeLog.size(), linkModeWrites);
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, ip6Writes);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_EQ(rejected.FinishedCount(), 0u);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, RestartBeforeEventsDrainIsRejectedAndNextScanKeepsIp6Up)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    ScanCallbackSpy early;
    EXPECT_EQ(StartScan(&early), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(FakeOpenThread().discoverCalls, 1u);
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);

    ScanCallbackSpy next;
    ASSERT_EQ(StartScan(&next), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_EQ(early.FinishedCount(), 0u);
    EXPECT_EQ(next.FinishedCount(), 1u);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

// ===== Who owns IPv6 between the scan and its deferred restore

TEST_F(TestThreadScanStateRestore, ScanWithNothingToRestoreSchedulesNoDeferredRestore)
{
    FakeOpenThread().ip6Enabled   = true;
    FakeOpenThread().role         = OT_DEVICE_ROLE_CHILD;
    FakeOpenThread().commissioned = true;
    FakeOpenThread().linkMode     = MakeLinkMode(/* rxOnWhenIdle = */ true, /* deviceType = */ false, /* networkData = */ true);
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    const unsigned locksBeforeEvents = FakeOpenThread().threadStackLockCount;
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().threadStackLockCount, locksBeforeEvents);
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, CommissioningBeforeDeferredRestoreKeepsIp6)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    FakeOpenThread().commissioned = true;
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true }));
}

TEST_F(TestThreadScanStateRestore, AttachingBeforeDeferredRestoreKeepsIp6)
{
    for (otDeviceRole attached : { OT_DEVICE_ROLE_DETACHED, OT_DEVICE_ROLE_CHILD, OT_DEVICE_ROLE_ROUTER })
    {
        ResetFakeOpenThread();
        MakeUncommissionedSleepyEndDevice();
        ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
        CompleteScan();

        FakeOpenThread().role = attached;
        DrainEvents();
        EXPECT_TRUE(FakeOpenThread().ip6Enabled) << "role " << static_cast<int>(attached);
    }
}

TEST_F(TestThreadScanStateRestore, Ip6KeptForADatasetIsNotTurnedOffByALaterScan)
{
    FakeOpenThread().commissioned = true;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    FakeOpenThread().commissioned = false;
    ClearLogs();
    ScanCallbackSpy next;
    ASSERT_EQ(StartScan(&next), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanStateRestore, Ip6KeptForAnAttachIsNotTurnedOffByALaterFailedStart)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    FakeOpenThread().role = OT_DEVICE_ROLE_CHILD;
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    FakeOpenThread().role = OT_DEVICE_ROLE_DISABLED;
    ClearLogs();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    ScanCallbackSpy next;
    EXPECT_NE(StartScan(&next), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanStateRestore, FailedThreadEnableLeavesThePendingRestoreInPlace)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    FakeOpenThread().threadSetEnabledResult = OT_ERROR_INVALID_STATE;
    EXPECT_NE(SetThreadEnabled(true), CHIP_NO_ERROR);
    ClearLogs();
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ false }));
}

TEST_F(TestThreadScanStateRestore, ThreadDisableLeavesNothingForThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    EXPECT_EQ(SetThreadEnabled(false), CHIP_NO_ERROR);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    ClearLogs();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanStateRestore, Ip6DisableFailureInThreadDisableIsRetriedByThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    FakeOpenThread().ip6DisableResult = OT_ERROR_FAILED;
    EXPECT_NE(SetThreadEnabled(false), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    FakeOpenThread().ip6DisableResult = OT_ERROR_NONE;
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, EraseLeavesNothingForThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    ErasePersistentInfo();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    ClearLogs();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanStateRestore, Ip6DisableFailureInEraseIsRetriedByThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    FakeOpenThread().ip6DisableResult = OT_ERROR_FAILED;
    ErasePersistentInfo();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    FakeOpenThread().ip6DisableResult = OT_ERROR_NONE;
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

// ===== OpenThread write failures

TEST_F(TestThreadScanStateRestore, Ip6BringUpFailureFailsTheStartWithoutTouchingLinkMode)
{
    FakeOpenThread().ip6EnableResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().discoverCalls, 0u);
    EXPECT_EQ(Ip6DisableCalls(), 0);
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    EXPECT_TRUE(mCallback.mStatuses.empty());

    ExpectRetryAccepted();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, Ip6DisableFailureOnFailedStartIsRetriedByTheNextScan)
{
    FakeOpenThread().discoverResult   = OT_ERROR_BUSY;
    FakeOpenThread().ip6DisableResult = OT_ERROR_FAILED;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(Ip6DisableCalls(), 1);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    EXPECT_TRUE(mCallback.mStatuses.empty());

    ExpectRetryAccepted();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, Ip6DisableFailureAfterCompletedScanIsRetriedByTheNextScan)
{
    FakeOpenThread().ip6DisableResult = OT_ERROR_FAILED;

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    EXPECT_EQ(Ip6DisableCalls(), 1);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    ExpectRetryAccepted();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER

// ===== The rx-on-when-idle override on a sleepy end device

TEST_F(TestThreadScanStateRestore, FailedStartRestoresRxOffWithOtherBitsIntact)
{
    unsigned discoverCalls = 0;
    for (otError error : kDiscoverErrors)
    {
        MakeUncommissionedSleepyEndDevice();
        ClearLogs();
        FakeOpenThread().discoverResult = error;

        EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
        EXPECT_EQ(FakeOpenThread().discoverCalls, ++discoverCalls);
        EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false })) << "otError " << static_cast<int>(error);
        EXPECT_EQ(FakeOpenThread().setLinkModeLockDepth, (std::vector<int>{ 1, 1 }));
        EXPECT_FALSE(FakeOpenThread().linkMode.mDeviceType);
        EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
        DrainEvents();
    }
}

TEST_F(TestThreadScanStateRestore, CompletedScanRestoresRxOffBeforeAnyEventRuns)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
}

TEST_F(TestThreadScanStateRestore, NextScanReappliesTheOverrideAfterAFailedStart)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    FakeOpenThread().discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    CompleteScanAndDrain();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, true, false }));
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
}

TEST_F(TestThreadScanStateRestore, LateCompletionAfterAFailedStartWritesNothing)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();

    ClearLogs();
    CompleteScanAndDrain();
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_EQ(mCallback.FinishedCount(), 0u);
}

TEST_F(TestThreadScanStateRestore, LinkModeApplyFailureDoesNotAbortTheScan)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_INVALID_ARGS };

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(FakeOpenThread().discoverCalls, 1u);
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanStateRestore, LinkModeRestoreFailureOnFailedStartStillTurnsIp6Off)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS };
    FakeOpenThread().discoverResult     = OT_ERROR_BUSY;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
}

TEST_F(TestThreadScanStateRestore, LinkModeRestoreFailureAfterCompletedScanIsRetriedByTheDeferredRestore)
{
    for (bool ip6WasUp : { false, true })
    {
        ResetFakeOpenThread();
        MakeUncommissionedSleepyEndDevice();
        FakeOpenThread().ip6Enabled         = ip6WasUp;
        FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS };

        ScanCallbackSpy callback;
        ASSERT_EQ(StartScan(&callback), CHIP_NO_ERROR);
        CompleteScan();
        EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

        DrainEvents();
        EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, false })) << "ip6 was up " << ip6WasUp;
        EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
        EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
        EXPECT_EQ(FakeOpenThread().ip6Enabled, ip6WasUp);
        EXPECT_EQ(callback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    }
}

TEST_F(TestThreadScanStateRestore, LinkModeRestoreFailureIsRepairedByTheNextScan)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS, OT_ERROR_INVALID_ARGS };
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    ASSERT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    ClearLogs();
    ExpectRetryAccepted();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
}

TEST_F(TestThreadScanStateRestore, EveryWriteFailingOnAFailedStartStillUnwindsCleanly)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_INVALID_ARGS, OT_ERROR_INVALID_ARGS };
    FakeOpenThread().discoverResult     = OT_ERROR_NO_BUFS;
    FakeOpenThread().ip6DisableResult   = OT_ERROR_FAILED;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_EQ(Ip6DisableCalls(), 1);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);

    ClearLogs();
    ExpectRetryAccepted();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
}

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

// ===== Every reachable device state against every otThreadDiscover() outcome

struct DeviceState
{
    otDeviceRole role;
    bool commissioned;
    bool ip6;
};

// Both outcomes leave these states as they were before the scan.
constexpr DeviceState kStatesKeptByBothOutcomes[] = {
    { OT_DEVICE_ROLE_DISABLED, false, false }, { OT_DEVICE_ROLE_DISABLED, false, true }, { OT_DEVICE_ROLE_DISABLED, true, true },
    { OT_DEVICE_ROLE_DETACHED, false, true },  { OT_DEVICE_ROLE_DETACHED, true, true },  { OT_DEVICE_ROLE_CHILD, true, true },
    { OT_DEVICE_ROLE_ROUTER, true, true },     { OT_DEVICE_ROLE_LEADER, true, true },
};

constexpr DeviceState kAllStates[] = {
    kStatesKeptByBothOutcomes[0], kStatesKeptByBothOutcomes[1], kStatesKeptByBothOutcomes[2],
    kStatesKeptByBothOutcomes[3], kStatesKeptByBothOutcomes[4], kStatesKeptByBothOutcomes[5],
    kStatesKeptByBothOutcomes[6], kStatesKeptByBothOutcomes[7], { OT_DEVICE_ROLE_DISABLED, true, false },
};

const otLinkModeConfig kRxOnFtd     = MakeLinkMode(true, true, true);
const otLinkModeConfig kLinkModes[] = { kRxOnFtd, MakeLinkMode(true, false, false), MakeLinkMode(false, false, false),
                                        MakeLinkMode(false, false, true) };

std::vector<otError> AllDiscoverErrors()
{
    std::vector<otError> errors;
    for (int e = OT_ERROR_FAILED; e < OT_NUM_ERRORS; ++e)
    {
        errors.push_back(static_cast<otError>(e));
    }
    errors.push_back(OT_ERROR_GENERIC);
    return errors;
}

std::vector<otError> AllOutcomes()
{
    std::vector<otError> outcomes = AllDiscoverErrors();
    outcomes.push_back(OT_ERROR_NONE);
    return outcomes;
}

std::string Describe(const DeviceState & state, const otLinkModeConfig & mode, otError error)
{
    return "role " + std::to_string(state.role) + " commissioned " + std::to_string(state.commissioned) + " ip6 " +
        std::to_string(state.ip6) + " rx " + std::to_string(mode.mRxOnWhenIdle) + " ftd " + std::to_string(mode.mDeviceType) +
        " netdata " + std::to_string(mode.mNetworkData) + " otError " + std::to_string(error);
}

struct Outcome
{
    otLinkModeConfig linkMode;
    bool ip6;
    std::vector<otLinkModeConfig> linkModeWrites;
    std::vector<int> linkModeWriteLockDepth;
    std::vector<bool> ip6Writes;
    int lockDepth;
    size_t finishedCount;
};

// OT_ERROR_NONE runs the scan to completion; any other value fails the start.
Outcome RunScan(const DeviceState & state, const otLinkModeConfig & mode, otError discoverResult)
{
    ResetFakeOpenThread();
    auto manager = std::make_unique<TestThreadStackManager>();
    EXPECT_EQ(manager->Init(), CHIP_NO_ERROR);

    FakeOpenThread().role           = state.role;
    FakeOpenThread().commissioned   = state.commissioned;
    FakeOpenThread().ip6Enabled     = state.ip6;
    FakeOpenThread().linkMode       = mode;
    FakeOpenThread().discoverResult = discoverResult;
    ScanCallbackSpy callback;

    CHIP_ERROR err = StartScan(*manager, &callback);
    EXPECT_EQ(err == CHIP_NO_ERROR, discoverResult == OT_ERROR_NONE) << Describe(state, mode, discoverResult);
    if (err == CHIP_NO_ERROR)
    {
        CompleteScan(*manager);
    }
    DrainEvents();

    return Outcome{ FakeOpenThread().linkMode,         FakeOpenThread().ip6Enabled,
                    FakeOpenThread().setLinkModeLog,   FakeOpenThread().setLinkModeLockDepth,
                    FakeOpenThread().ip6SetEnabledLog, FakeOpenThread().threadStackLockDepth,
                    callback.FinishedCount() };
}

template <size_t N, typename Check>
void ForEachCell(const DeviceState (&states)[N], const std::vector<otError> & outcomes, Check check)
{
    for (const DeviceState & state : states)
    {
        for (const otLinkModeConfig & mode : kLinkModes)
        {
            const bool isRouter = state.role == OT_DEVICE_ROLE_ROUTER || state.role == OT_DEVICE_ROLE_LEADER;
            if (isRouter && !SameLinkMode(mode, kRxOnFtd))
            {
                continue;
            }
            for (otError error : outcomes)
            {
                check(state, mode, error);
            }
        }
    }
}

void ExpectPreScanState(const DeviceState & state, const otLinkModeConfig & mode, otError error)
{
    const Outcome out      = RunScan(state, mode, error);
    const std::string cell = Describe(state, mode, error);
    EXPECT_TRUE(SameLinkMode(out.linkMode, mode)) << cell;
    EXPECT_EQ(out.ip6, state.ip6) << cell;
    EXPECT_TRUE(!state.ip6 || out.ip6Writes.empty()) << cell;
    EXPECT_EQ(out.lockDepth, 0) << cell;
    EXPECT_EQ(out.finishedCount, (error == OT_ERROR_NONE ? 1u : 0u)) << cell;
}

TEST_F(TestThreadScanStateRestore, FailedStartLeavesEveryDeviceStateAsBeforeTheScan)
{
    ForEachCell(kStatesKeptByBothOutcomes, AllDiscoverErrors(), ExpectPreScanState);
}

TEST_F(TestThreadScanStateRestore, CompletedScanLeavesEveryDeviceStateAsBeforeTheScan)
{
    ForEachCell(kStatesKeptByBothOutcomes, { OT_ERROR_NONE }, ExpectPreScanState);
}

TEST_F(TestThreadScanStateRestore, FailedStartEndsWhereACompletedScanEnds)
{
    ForEachCell(kAllStates, AllDiscoverErrors(), [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
        const Outcome completed = RunScan(state, mode, OT_ERROR_NONE);
        const Outcome failed    = RunScan(state, mode, error);
        const std::string cell  = Describe(state, mode, error);
        EXPECT_TRUE(SameLinkMode(failed.linkMode, completed.linkMode)) << cell;
        EXPECT_EQ(failed.ip6, completed.ip6) << cell;
        EXPECT_EQ(failed.linkModeWrites.size(), completed.linkModeWrites.size()) << cell;
    });
}

TEST_F(TestThreadScanStateRestore, RxOnDeviceLinkModeIsNeverWritten)
{
    ForEachCell(kAllStates, AllOutcomes(), [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        if (!mode.mRxOnWhenIdle)
        {
            return;
        }
#endif
        EXPECT_TRUE(RunScan(state, mode, error).linkModeWrites.empty()) << Describe(state, mode, error);
    });
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
TEST_F(TestThreadScanStateRestore, SleepyDeviceGetsExactlyOneOverrideAndOneRestoreUnderTheLock)
{
    ForEachCell(kStatesKeptByBothOutcomes, AllOutcomes(),
                [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
                    if (mode.mRxOnWhenIdle)
                    {
                        return;
                    }
                    const Outcome out      = RunScan(state, mode, error);
                    const std::string cell = Describe(state, mode, error);
                    ASSERT_EQ(out.linkModeWrites.size(), 2u) << cell;
                    otLinkModeConfig overridden = mode;
                    overridden.mRxOnWhenIdle    = true;
                    EXPECT_TRUE(SameLinkMode(out.linkModeWrites[0], overridden)) << cell;
                    EXPECT_TRUE(SameLinkMode(out.linkModeWrites[1], mode)) << cell;
                    EXPECT_EQ(out.linkModeWriteLockDepth, (std::vector<int>{ 1, 1 })) << cell;
                });
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

} // namespace
