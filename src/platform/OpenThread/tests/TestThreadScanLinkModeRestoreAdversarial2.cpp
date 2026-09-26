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

// No host device layer supplies a ThreadStackManagerImpl, so this file re-includes itself
// through EXTERNAL_THREADSTACKMANAGERIMPL_HEADER to define one.
#ifdef CHIP_THREAD_SCAN_CYCLE_TEST_IMPL_PASS

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>
{
    friend ThreadStackManager;

public:
    CHIP_ERROR _InitThreadStack() { return CHIP_NO_ERROR; }

protected:
    CHIP_ERROR _StartThreadTask() { return CHIP_NO_ERROR; }
    void _LockThreadStack() {}
    bool _TryLockThreadStack() { return true; }
    void _UnlockThreadStack() {}
    void _ProcessThreadActivity() {}
};

} // namespace DeviceLayer
} // namespace chip

#else // CHIP_THREAD_SCAN_CYCLE_TEST_IMPL_PASS

#define CHIP_THREAD_SCAN_CYCLE_TEST_IMPL_PASS 1
#define EXTERNAL_THREADSTACKMANAGERIMPL_HEADER "platform/OpenThread/tests/TestThreadScanLinkModeRestoreAdversarial2.cpp"

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

struct FakeOpenThread
{
    otLinkModeConfig linkMode   = {};
    bool ip6Enabled             = true;
    otError ip6SetEnabledResult = OT_ERROR_NONE;
    otError discoverResult      = OT_ERROR_NONE;
    otDeviceRole role           = OT_DEVICE_ROLE_CHILD;
    bool commissioned           = true;
    unsigned discoverCalls      = 0;
    unsigned ip6SetEnabledCalls = 0;

    std::vector<otLinkModeConfig> setLinkModeLog;
};

FakeOpenThread gOt;
int gLockDepth = 0;

void ResetFake()
{
    gOt        = FakeOpenThread();
    gLockDepth = 0;
}

otInstance * FakeInstance()
{
    static uint8_t storage = 0;
    return reinterpret_cast<otInstance *>(&storage);
}

} // namespace

extern "C" {

otLinkModeConfig otThreadGetLinkMode(otInstance *)
{
    return gOt.linkMode;
}

otError otThreadSetLinkMode(otInstance *, otLinkModeConfig aConfig)
{
    gOt.setLinkModeLog.push_back(aConfig);
    gOt.linkMode = aConfig;
    return OT_ERROR_NONE;
}

bool otIp6IsEnabled(otInstance *)
{
    return gOt.ip6Enabled;
}

otError otIp6SetEnabled(otInstance *, bool aEnabled)
{
    gOt.ip6SetEnabledCalls++;
    if (gOt.ip6SetEnabledResult == OT_ERROR_NONE)
    {
        gOt.ip6Enabled = aEnabled;
    }
    return gOt.ip6SetEnabledResult;
}

otError otThreadDiscover(otInstance *, uint32_t, uint16_t, bool, bool, otHandleActiveScanResult, void *)
{
    gOt.discoverCalls++;
    return gOt.discoverResult;
}

otDeviceRole otThreadGetDeviceRole(otInstance *)
{
    return gOt.role;
}

bool otDatasetIsCommissioned(otInstance *)
{
    return gOt.commissioned;
}

otError otSetStateChangedCallback(otInstance *, otStateChangedCallback, void *)
{
    return OT_ERROR_NONE;
}

otError otThreadSetEnabled(otInstance *, bool)
{
    return OT_ERROR_NONE;
}

const char * otThreadErrorToString(otError)
{
    return "fake";
}

} // extern "C"

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Supplied by OpenThreadUtils.cpp on targets that link the real OpenThread stack.
void RegisterOpenThreadErrorFormatter() {}

CHIP_ERROR MapOpenThreadError(otError otErr)
{
    return (otErr == OT_ERROR_NONE) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

namespace {

using namespace chip;
using namespace chip::DeviceLayer;

class ScanCallbackSpy : public NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    void OnFinished(NetworkCommissioning::Status, CharSpan, NetworkCommissioning::ThreadScanResponseIterator *) override
    {
        mFinishedCount++;
    }

    unsigned mFinishedCount = 0;
};

// _OnNetworkScanFinished() posts work to the CHIP event queue, so the stack stays up for the binary.
struct PlatformStackFixture
{
    PlatformStackFixture() : mInitResult(PlatformMgr().InitChipStack()) {}
    ~PlatformStackFixture() { PlatformMgr().Shutdown(); }
    CHIP_ERROR mInitResult;
};

CHIP_ERROR EnsurePlatformStack()
{
    static PlatformStackFixture fixture;
    return fixture.mInitResult;
}

class TestThreadStackManager : public Internal::GenericThreadStackManagerImpl_OpenThread<TestThreadStackManager>
{
public:
    CHIP_ERROR Init() { return ConfigureThreadStack(FakeInstance()); }

    CHIP_ERROR StartScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback) { return _StartThreadScan(callback); }

    void CompleteScan() { _OnNetworkScanFinished(nullptr, this); }

    static void OnOpenThreadStateChange(uint32_t, void *) {}

    void LockThreadStack() { gLockDepth++; }
    void UnlockThreadStack() { gLockDepth--; }
};

class ThreadScanLinkModeCycleAdversarial2 : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(EnsurePlatformStack(), CHIP_NO_ERROR); }

protected:
    void SetUp() override
    {
        ResetFake();
        mManager = std::make_unique<TestThreadStackManager>();
        ASSERT_EQ(mManager->Init(), CHIP_NO_ERROR);
        gOt.setLinkModeLog.clear();
    }

    void TearDown() override { DrainPlatformEvents(); }

    void MakeSleepyEndDevice()
    {
        gOt.linkMode.mRxOnWhenIdle = false;
        gOt.linkMode.mDeviceType   = false;
        gOt.linkMode.mNetworkData  = true;
    }

    void MakeRouter()
    {
        gOt.linkMode.mRxOnWhenIdle = true;
        gOt.linkMode.mDeviceType   = true;
        gOt.linkMode.mNetworkData  = true;
    }

    // Runs the lambdas _OnNetworkScanFinished() posts, then quiesces the work queue again.
    void DrainPlatformEvents()
    {
        ASSERT_EQ(PlatformMgr().StartEventLoopTask(), CHIP_NO_ERROR);
        ASSERT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR);
    }

    std::vector<bool> RxOnWrites() const
    {
        std::vector<bool> bits;
        for (const otLinkModeConfig & config : gOt.setLinkModeLog)
        {
            bits.push_back(config.mRxOnWhenIdle);
        }
        return bits;
    }

    void ExpectAdvertisedAsSleepy() const
    {
        EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle);
        EXPECT_FALSE(gOt.linkMode.mDeviceType);
        EXPECT_TRUE(gOt.linkMode.mNetworkData);
        EXPECT_EQ(gLockDepth, 0);
    }

    std::unique_ptr<TestThreadStackManager> mManager;
    ScanCallbackSpy mCallback;
};

using ThreadScanLinkModeCycleGuardAdversarial2 = ThreadScanLinkModeCycleAdversarial2;

TEST_F(ThreadScanLinkModeCycleAdversarial2, RxOnAdvertisedOnlyWhileDiscoverOutstanding)
{
    MakeSleepyEndDevice();

    gOt.discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    ExpectAdvertisedAsSleepy();

    gOt.ip6Enabled     = false;
    gOt.discoverResult = OT_ERROR_INVALID_STATE;
    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    ExpectAdvertisedAsSleepy();

    gOt.discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);

    mManager->CompleteScan();
    DrainPlatformEvents();
    ExpectAdvertisedAsSleepy();
    EXPECT_EQ(mCallback.mFinishedCount, 1u);
}

TEST_F(ThreadScanLinkModeCycleAdversarial2, EveryFailedAttemptTakesAndReleasesTheOverrideOnce)
{
    const otError errors[] = { OT_ERROR_BUSY,   OT_ERROR_INVALID_STATE, OT_ERROR_NO_BUFS,
                               OT_ERROR_FAILED, OT_ERROR_BUSY,          OT_ERROR_INVALID_ARGS };
    MakeSleepyEndDevice();

    unsigned attempt = 0;
    for (otError error : errors)
    {
        attempt++;
        gOt.setLinkModeLog.clear();
        gOt.discoverResult = error;

        EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "attempt " << attempt;
        EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false })) << "attempt " << attempt;
        ExpectAdvertisedAsSleepy();
        EXPECT_EQ(gOt.discoverCalls, attempt);
    }
}

TEST_F(ThreadScanLinkModeCycleAdversarial2, FailureThenFullScanCycleRepeats)
{
    MakeSleepyEndDevice();

    for (unsigned cycle = 1; cycle <= 3; cycle++)
    {
        gOt.setLinkModeLog.clear();

        gOt.discoverResult = OT_ERROR_BUSY;
        EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "cycle " << cycle;
        ExpectAdvertisedAsSleepy();

        gOt.discoverResult = OT_ERROR_NONE;
        ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "cycle " << cycle;
        EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle) << "cycle " << cycle;

        mManager->CompleteScan();
        DrainPlatformEvents();
        ExpectAdvertisedAsSleepy();
        EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, true, false })) << "cycle " << cycle;
        EXPECT_EQ(mCallback.mFinishedCount, cycle);
    }
}

TEST_F(ThreadScanLinkModeCycleAdversarial2, LateCompletionAfterFailedStartWritesNothing)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    gOt.setLinkModeLog.clear();
    mManager->CompleteScan();
    DrainPlatformEvents();

    EXPECT_TRUE(RxOnWrites().empty());
    ExpectAdvertisedAsSleepy();
    EXPECT_EQ(mCallback.mFinishedCount, 0u);
}

TEST_F(ThreadScanLinkModeCycleAdversarial2, FailedStartDoesNotStrandTheOverrideAcrossAReject)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    gOt.discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    ScanCallbackSpy rejected;
    EXPECT_EQ(mManager->StartScan(&rejected), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, true }));
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);

    mManager->CompleteScan();
    DrainPlatformEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false, true, false }));
    ExpectAdvertisedAsSleepy();
    EXPECT_EQ(rejected.mFinishedCount, 0u);
}

TEST_F(ThreadScanLinkModeCycleGuardAdversarial2, StartedScanHoldsTheOverrideUntilItsCompletion)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_NONE;

    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true }));
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
    DrainPlatformEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true }));
    EXPECT_EQ(mCallback.mFinishedCount, 0u);

    mManager->CompleteScan();
    DrainPlatformEvents();
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true, false }));
    ExpectAdvertisedAsSleepy();
    EXPECT_EQ(mCallback.mFinishedCount, 1u);
}

TEST_F(ThreadScanLinkModeCycleGuardAdversarial2, RepeatedFailuresOnARouterNeverTouchLinkMode)
{
    MakeRouter();
    const otError errors[] = { OT_ERROR_BUSY, OT_ERROR_INVALID_STATE, OT_ERROR_NO_BUFS, OT_ERROR_FAILED };

    for (otError error : errors)
    {
        gOt.discoverResult = error;
        EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "otError " << static_cast<int>(error);
        EXPECT_TRUE(RxOnWrites().empty()) << "otError " << static_cast<int>(error);
        EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
        EXPECT_TRUE(gOt.linkMode.mDeviceType);
    }
    EXPECT_EQ(gLockDepth, 0);
}

TEST_F(ThreadScanLinkModeCycleGuardAdversarial2, RouterScanCycleNeverTouchesLinkMode)
{
    MakeRouter();
    gOt.discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    mManager->CompleteScan();
    DrainPlatformEvents();
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(gOt.linkMode.mDeviceType);
    EXPECT_EQ(mCallback.mFinishedCount, 1u);
}

TEST_F(ThreadScanLinkModeCycleGuardAdversarial2, Ip6BringUpFailureBeforeTheOverrideRestoresNothing)
{
    MakeSleepyEndDevice();
    gOt.ip6Enabled          = false;
    gOt.ip6SetEnabledResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(RxOnWrites().empty());
    EXPECT_EQ(gOt.discoverCalls, 0u);
    ExpectAdvertisedAsSleepy();

    gOt.ip6SetEnabledResult = OT_ERROR_NONE;
    gOt.discoverResult      = OT_ERROR_NONE;
    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(RxOnWrites(), (std::vector<bool>{ true }));

    mManager->CompleteScan();
    DrainPlatformEvents();
    ExpectAdvertisedAsSleepy();
}

} // namespace

#endif // CHIP_THREAD_SCAN_CYCLE_TEST_IMPL_PASS
