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

#include "ThreadScanTestFixture.h"

namespace {

using namespace chip;
using namespace chip::Testing;

class TestThreadScanSequencing : public ThreadScanTest
{
protected:
    void SetUp() override
    {
        ThreadScanTest::SetUp();
        MakeUncommissionedSleepyEndDevice();
    }

    static void ClearLogs()
    {
        FakeOpenThread().setLinkModeLog.clear();
        FakeOpenThread().setLinkModeLockDepth.clear();
        FakeOpenThread().ip6SetEnabledLog.clear();
        FakeOpenThread().ip6SetEnabledLockDepth.clear();
    }
};

TEST_F(TestThreadScanSequencing, SecondScanWhileRunningIsRejectedWithoutTouchingOpenThread)
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
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);

    CompleteScan();
    DrainEvents();
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_EQ(rejected.FinishedCount(), 0u);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanSequencing, BusyStartReleasesTheScanSlotForTheNextCaller)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();

    ScanCallbackSpy next;
    FakeOpenThread().discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(StartScan(&next), CHIP_NO_ERROR);
    EXPECT_EQ(FakeOpenThread().discoverCalls, 2u);
    CompleteScan();
    DrainEvents();

    EXPECT_EQ(mCallback.FinishedCount(), 0u);
    EXPECT_EQ(next.FinishedCount(), 1u);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanSequencing, FailedStartRestoresIp6BeforeAnyEventRuns)
{
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);

    DrainEvents();
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_EQ(mCallback.FinishedCount(), 0u);
}

TEST_F(TestThreadScanSequencing, EveryIp6WriteHoldsTheThreadStackLock)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLockDepth, (std::vector<int>{ 1, 1 }));

    ClearLogs();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true, false }));
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLockDepth, (std::vector<int>{ 1, 1 }));
}

TEST_F(TestThreadScanSequencing, RestartBeforeEventsDrainIsRejectedAndNextScanKeepsIp6Up)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    ScanCallbackSpy early;
    EXPECT_EQ(StartScan(&early), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(FakeOpenThread().discoverCalls, 1u);
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    ScanCallbackSpy next;
    ASSERT_EQ(StartScan(&next), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScan();
    DrainEvents();
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_EQ(early.FinishedCount(), 0u);
    EXPECT_EQ(next.FinishedCount(), 1u);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanSequencing, CommissioningBeforeDeferredRestoreKeepsIp6)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    FakeOpenThread().commissioned = true;
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().ip6SetEnabledLog, (std::vector<bool>{ true }));
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
}

TEST_F(TestThreadScanSequencing, ScanWithNothingToRestoreSchedulesNoDeferredRestore)
{
    FakeOpenThread().ip6Enabled   = true;
    FakeOpenThread().role         = OT_DEVICE_ROLE_CHILD;
    FakeOpenThread().commissioned = true;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    const unsigned locksBeforeEvents = FakeOpenThread().threadStackLockCount;
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().threadStackLockCount, locksBeforeEvents);
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
}

TEST_F(TestThreadScanSequencing, ScanThatTurnedIp6OnRunsTheDeferredRestoreUnderTheLock)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    const unsigned locksBeforeEvents = FakeOpenThread().threadStackLockCount;
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().threadStackLockCount, locksBeforeEvents + 1);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanSequencing, AttachingBeforeDeferredRestoreKeepsIp6)
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

TEST_F(TestThreadScanSequencing, Ip6KeptForADatasetIsNotTurnedOffByALaterScan)
{
    FakeOpenThread().commissioned = true;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    FakeOpenThread().commissioned = false;
    ClearLogs();
    ScanCallbackSpy next;
    ASSERT_EQ(StartScan(&next), CHIP_NO_ERROR);
    CompleteScan();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanSequencing, Ip6KeptForAnAttachIsNotTurnedOffByALaterFailedStart)
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

TEST_F(TestThreadScanSequencing, FailedThreadEnableLeavesThePendingRestoreInPlace)
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

TEST_F(TestThreadScanSequencing, ThreadDisableLeavesNothingForThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    EXPECT_EQ(SetThreadEnabled(false), CHIP_NO_ERROR);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    ClearLogs();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanSequencing, Ip6DisableFailureInThreadDisableIsRetriedByThePendingRestore)
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

TEST_F(TestThreadScanSequencing, EraseLeavesNothingForThePendingRestore)
{
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();

    ErasePersistentInfo();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    ClearLogs();
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6SetEnabledLog.empty());
}

TEST_F(TestThreadScanSequencing, Ip6DisableFailureInEraseIsRetriedByThePendingRestore)
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

} // namespace
