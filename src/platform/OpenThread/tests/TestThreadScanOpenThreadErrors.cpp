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

#include <algorithm>

namespace {

using namespace chip;
using namespace chip::Testing;
using NetworkCommissioning::Status;

class TestThreadScanOpenThreadErrors : public ThreadScanTest
{
protected:
    void SetUp() override
    {
        ThreadScanTest::SetUp();
        MakeUncommissionedSleepyEndDevice();
    }

    void CompleteScanAndDrain()
    {
        CompleteScan();
        DrainEvents();
    }

    static long Ip6DisableCalls()
    {
        const std::vector<bool> & log = FakeOpenThread().ip6SetEnabledLog;
        return std::count(log.begin(), log.end(), false);
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
};

TEST_F(TestThreadScanOpenThreadErrors, Ip6BringUpFailureFailsTheStartAndFreesTheSlot)
{
    FakeOpenThread().ip6EnableResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(FakeOpenThread().discoverCalls, 0u);
    EXPECT_EQ(Ip6DisableCalls(), 0);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    EXPECT_TRUE(mCallback.mStatuses.empty());

    ExpectRetryAccepted();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanOpenThreadErrors, Ip6DisableFailureOnFailedStartKeepsTheStartError)
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

TEST_F(TestThreadScanOpenThreadErrors, Ip6DisableFailureAfterCompletedScanStillReportsSuccess)
{
    FakeOpenThread().ip6DisableResult = OT_ERROR_FAILED;

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    EXPECT_EQ(Ip6DisableCalls(), 1);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);

    ExpectRetryAccepted();
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER

TEST_F(TestThreadScanOpenThreadErrors, LinkModeApplyFailureDoesNotAbortTheScan)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_INVALID_ARGS };

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(FakeOpenThread().discoverCalls, 1u);
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    CompleteScanAndDrain();
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
}

TEST_F(TestThreadScanOpenThreadErrors, LinkModeApplyFailureThenFailedStartLeavesNoOverrideBehind)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_INVALID_ARGS };
    FakeOpenThread().discoverResult     = OT_ERROR_BUSY;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);

    FakeOpenThread().setLinkModeLog.clear();
    CompleteScanAndDrain();
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_TRUE(mCallback.mStatuses.empty());
}

TEST_F(TestThreadScanOpenThreadErrors, LinkModeRestoreFailureOnFailedStartStillTurnsIp6Off)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS };
    FakeOpenThread().discoverResult     = OT_ERROR_BUSY;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_EQ(FakeOpenThread().setLinkModeLockDepth, (std::vector<int>{ 1, 1 }));
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    EXPECT_TRUE(mCallback.mStatuses.empty());
}

TEST_F(TestThreadScanOpenThreadErrors, LinkModeRestoreFailureAfterCompletedScanIsRetriedByTheDeferredRestore)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS };

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    DrainEvents();
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
    EXPECT_FALSE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
}

TEST_F(TestThreadScanOpenThreadErrors, LinkModeRestoreFailureIsRetriedByTheDeferredRestoreWhenIp6WasAlreadyUp)
{
    FakeOpenThread().ip6Enabled         = true;
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS };

    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScan();
    EXPECT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(FakeOpenThread().ip6Enabled);
    EXPECT_EQ(mCallback.mStatuses, (std::vector<Status>{ Status::kSuccess }));
}

TEST_F(TestThreadScanOpenThreadErrors, EveryWriteFailingOnAFailedStartStillUnwindsCleanly)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_INVALID_ARGS, OT_ERROR_INVALID_ARGS };
    FakeOpenThread().discoverResult     = OT_ERROR_NO_BUFS;
    FakeOpenThread().ip6DisableResult   = OT_ERROR_FAILED;

    EXPECT_NE(StartScan(&mCallback), CHIP_NO_ERROR);
    DrainEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_EQ(Ip6DisableCalls(), 1);
    EXPECT_EQ(FakeOpenThread().threadStackLockDepth, 0);
    EXPECT_TRUE(mCallback.mStatuses.empty());

    FakeOpenThread().setLinkModeLog.clear();
    ExpectRetryAccepted();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
}

TEST_F(TestThreadScanOpenThreadErrors, LinkModeRestoreFailureIsRepairedByTheNextScan)
{
    FakeOpenThread().setLinkModeResults = { OT_ERROR_NONE, OT_ERROR_INVALID_ARGS, OT_ERROR_INVALID_ARGS };
    ASSERT_EQ(StartScan(&mCallback), CHIP_NO_ERROR);
    CompleteScanAndDrain();
    ASSERT_TRUE(FakeOpenThread().linkMode.mRxOnWhenIdle);

    FakeOpenThread().setLinkModeLog.clear();
    ExpectRetryAccepted();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ false }));
    EXPECT_FALSE(FakeOpenThread().linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(FakeOpenThread().linkMode.mNetworkData);
}

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

} // namespace
