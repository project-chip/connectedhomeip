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

using TestThreadScanStartFailure = ThreadScanTest;

struct DeviceState
{
    otDeviceRole role;
    bool commissioned;
    bool ip6AfterScan;
};

constexpr DeviceState kDeviceStates[] = {
    { OT_DEVICE_ROLE_DISABLED, false, false },
    { OT_DEVICE_ROLE_DISABLED, true, true },
    { OT_DEVICE_ROLE_CHILD, true, true },
    { OT_DEVICE_ROLE_DETACHED, false, true },
};

constexpr otError kDiscoverErrors[] = { OT_ERROR_FAILED, OT_ERROR_BUSY, OT_ERROR_INVALID_STATE, OT_ERROR_INVALID_ARGS,
                                        OT_ERROR_NO_BUFS };

TEST_F(TestThreadScanStartFailure, FailedStartTurnsIp6BackOffBeforeCommissioning)
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

TEST_F(TestThreadScanStartFailure, FailedStartLeavesIp6LikeACompletedScan)
{
    for (const DeviceState & state : kDeviceStates)
    {
        ResetFakeOpenThread();
        FakeOpenThread().role         = state.role;
        FakeOpenThread().commissioned = state.commissioned;
        FakeOpenThread().ip6Enabled   = false;
        ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
        CompleteScan();
        DrainEvents();
        EXPECT_EQ(FakeOpenThread().ip6Enabled, state.ip6AfterScan)
            << "role " << static_cast<int>(state.role) << " commissioned " << state.commissioned;

        ResetFakeOpenThread();
        FakeOpenThread().role           = state.role;
        FakeOpenThread().commissioned   = state.commissioned;
        FakeOpenThread().ip6Enabled     = false;
        FakeOpenThread().discoverResult = OT_ERROR_BUSY;
        EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
        DrainEvents();

        EXPECT_EQ(FakeOpenThread().ip6Enabled, state.ip6AfterScan)
            << "role " << static_cast<int>(state.role) << " commissioned " << state.commissioned;
    }
}

TEST_F(TestThreadScanStartFailure, FailedStartLeavesIp6ThatWasAlreadyUp)
{
    unsigned discoverCalls = 0;
    for (otError error : kDiscoverErrors)
    {
        MakeUncommissionedSleepyEndDevice();
        FakeOpenThread().ip6Enabled     = true;
        FakeOpenThread().discoverResult = error;

        EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
        EXPECT_EQ(FakeOpenThread().discoverCalls, ++discoverCalls);
        DrainEvents();
        EXPECT_TRUE(FakeOpenThread().ip6Enabled) << "otError " << static_cast<int>(error);
    }
}

TEST_F(TestThreadScanStartFailure, RetryAfterFailedStartKeepsIp6UpForTheNewScan)
{
    MakeUncommissionedSleepyEndDevice();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    FakeOpenThread().discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScan();
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
}

TEST_F(TestThreadScanStartFailure, CompletedScanTurnsIp6BackOffBeforeCommissioning)
{
    MakeUncommissionedSleepyEndDevice();
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);

    CompleteScan();
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER

TEST_F(TestThreadScanStartFailure, FailedStartRestoresRxOffWithOtherBitsIntact)
{
    unsigned discoverCalls = 0;
    for (otError error : kDiscoverErrors)
    {
        MakeUncommissionedSleepyEndDevice();
        FakeOpenThread().setLinkModeLog.clear();
        FakeOpenThread().setLinkModeLockDepth.clear();
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

TEST_F(TestThreadScanStartFailure, FailedStartLeavesRxOnLikeACompletedScan)
{
    MakeUncommissionedSleepyEndDevice();
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    CompleteScan();
    DrainEvents();
    const std::vector<bool> completedScanWrites = RxOnWrites();

    ResetFakeOpenThread();
    MakeUncommissionedSleepyEndDevice();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();

    EXPECT_EQ(completedScanWrites, (std::vector<bool>{ true, false }));
    EXPECT_EQ(RxOnWrites(), completedScanWrites);
}

TEST_F(TestThreadScanStartFailure, CompletedScanRestoresRxOffBeforeAnyEventRuns)
{
    MakeUncommissionedSleepyEndDevice();
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
}

TEST_F(TestThreadScanStartFailure, NextScanReappliesTheOverrideAfterAFailedStart)
{
    MakeUncommissionedSleepyEndDevice();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);

    FakeOpenThread().discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    ScanCallbackSpy rejected;
    EXPECT_EQ(StartScan(&rejected), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    CompleteScan();
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, true, false }));
    EXPECT_EQ(mCallback.FinishedCount(), 1u);
    EXPECT_EQ(rejected.FinishedCount(), 0u);
}

TEST_F(TestThreadScanStartFailure, LateCompletionAfterAFailedStartWritesNothing)
{
    MakeUncommissionedSleepyEndDevice();
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();

    FakeOpenThread().setLinkModeLog.clear();
    CompleteScan();
    DrainEvents();
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_EQ(mCallback.FinishedCount(), 0u);
}

TEST_F(TestThreadScanStartFailure, FailureOnARxOnDeviceNeverTouchesLinkMode)
{
    FakeOpenThread().linkMode       = MakeLinkMode(/* rxOnWhenIdle = */ true, /* deviceType = */ true, /* networkData = */ true);
    FakeOpenThread().discoverResult = OT_ERROR_BUSY;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);
}

TEST_F(TestThreadScanStartFailure, Ip6BringUpFailureNeverTouchesLinkMode)
{
    MakeUncommissionedSleepyEndDevice();
    FakeOpenThread().ip6EnableResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().discoverCalls, 0u);
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

} // namespace
