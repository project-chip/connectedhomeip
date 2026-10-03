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

#include <openthread/error.h>

#include <memory>
#include <string>

namespace {

using namespace chip;
using namespace chip::Testing;

struct DeviceState
{
    otDeviceRole role;
    bool commissioned;
    bool ip6;
};

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

bool IsRouterRole(otDeviceRole role)
{
    return role == OT_DEVICE_ROLE_ROUTER || role == OT_DEVICE_ROLE_LEADER;
}

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

void ExpectInCell(bool ok, const std::string & cell)
{
    EXPECT_TRUE(ok) << cell;
    if (!ok)
    {
        ChipLogError(DeviceLayer, "failing cell: %s", cell.c_str());
    }
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

class TestThreadScanStateMatrix : public ThreadScanTest
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    // OT_ERROR_NONE runs the scan to completion; any other value fails the start.
    static Outcome RunScan(const DeviceState & state, const otLinkModeConfig & mode, otError discoverResult)
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
        ExpectInCell((err == CHIP_NO_ERROR) == (discoverResult == OT_ERROR_NONE), Describe(state, mode, discoverResult));
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
    static void ForEachCell(const DeviceState (&states)[N], const std::vector<otError> & outcomes, Check check)
    {
        for (const DeviceState & state : states)
        {
            for (const otLinkModeConfig & mode : kLinkModes)
            {
                if (IsRouterRole(state.role) && !SameLinkMode(mode, kRxOnFtd))
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

    static void ExpectPreScanState(const DeviceState & state, const otLinkModeConfig & mode, otError error)
    {
        const Outcome out      = RunScan(state, mode, error);
        const std::string cell = Describe(state, mode, error);
        ExpectInCell(SameLinkMode(out.linkMode, mode), cell);
        ExpectInCell(out.ip6 == state.ip6, cell);
        ExpectInCell(!state.ip6 || out.ip6Writes.empty(), cell);
        ExpectInCell(out.lockDepth == 0, cell);
        ExpectInCell(out.finishedCount == (error == OT_ERROR_NONE ? 1u : 0u), cell);
    }
};

TEST_F(TestThreadScanStateMatrix, FailedStartLeavesEveryDeviceStateAsBeforeTheScan)
{
    ForEachCell(kStatesKeptByBothOutcomes, AllDiscoverErrors(), ExpectPreScanState);
}

TEST_F(TestThreadScanStateMatrix, CompletedScanLeavesEveryDeviceStateAsBeforeTheScan)
{
    ForEachCell(kStatesKeptByBothOutcomes, { OT_ERROR_NONE }, ExpectPreScanState);
}

TEST_F(TestThreadScanStateMatrix, FailedStartEndsWhereACompletedScanEnds)
{
    ForEachCell(kAllStates, AllDiscoverErrors(), [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
        const Outcome completed = RunScan(state, mode, OT_ERROR_NONE);
        const Outcome failed    = RunScan(state, mode, error);
        const std::string cell  = Describe(state, mode, error);
        ExpectInCell(SameLinkMode(failed.linkMode, completed.linkMode), cell);
        ExpectInCell(failed.ip6 == completed.ip6, cell);
        ExpectInCell(failed.linkModeWrites.size() == completed.linkModeWrites.size(), cell);
    });
}

TEST_F(TestThreadScanStateMatrix, RxOnDeviceLinkModeIsNeverWritten)
{
    ForEachCell(kAllStates, AllOutcomes(), [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        if (!mode.mRxOnWhenIdle)
        {
            return;
        }
#endif
        ExpectInCell(RunScan(state, mode, error).linkModeWrites.empty(), Describe(state, mode, error));
    });
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
TEST_F(TestThreadScanStateMatrix, SleepyDeviceGetsExactlyOneOverrideAndOneRestoreUnderTheLock)
{
    ForEachCell(kStatesKeptByBothOutcomes, AllOutcomes(),
                [](const DeviceState & state, const otLinkModeConfig & mode, otError error) {
                    if (mode.mRxOnWhenIdle)
                    {
                        return;
                    }
                    const Outcome out      = RunScan(state, mode, error);
                    const std::string cell = Describe(state, mode, error);
                    ExpectInCell(out.linkModeWrites.size() == 2u, cell);
                    if (out.linkModeWrites.size() != 2u)
                    {
                        return;
                    }
                    otLinkModeConfig overridden = mode;
                    overridden.mRxOnWhenIdle    = true;
                    ExpectInCell(SameLinkMode(out.linkModeWrites[0], overridden), cell);
                    ExpectInCell(SameLinkMode(out.linkModeWrites[1], mode), cell);
                    ExpectInCell(out.linkModeWriteLockDepth[0] == 1, cell);
                    ExpectInCell(out.linkModeWriteLockDepth[1] == 1, cell);
                });
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

} // namespace
