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
#ifdef CHIP_THREAD_SCAN_TEST_IMPL_PASS

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

#else // CHIP_THREAD_SCAN_TEST_IMPL_PASS

#define CHIP_THREAD_SCAN_TEST_IMPL_PASS 1
#define EXTERNAL_THREADSTACKMANAGERIMPL_HEADER "platform/OpenThread/tests/TestThreadScanLinkModeRestoreAdversarial1.cpp"

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
    otDeviceRole role           = OT_DEVICE_ROLE_DISABLED;
    bool commissioned           = false;
    unsigned discoverCalls      = 0;
    unsigned ip6SetEnabledCalls = 0;

    std::vector<otLinkModeConfig> setLinkModeLog;
    std::vector<int> setLinkModeLockDepth;
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
    gOt.setLinkModeLockDepth.push_back(gLockDepth);
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

class TestThreadStackManager : public Internal::GenericThreadStackManagerImpl_OpenThread<TestThreadStackManager>
{
public:
    CHIP_ERROR Init() { return ConfigureThreadStack(FakeInstance()); }

    CHIP_ERROR StartScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback) { return _StartThreadScan(callback); }

    static void OnOpenThreadStateChange(uint32_t, void *) {}

    void LockThreadStack() { gLockDepth++; }
    void UnlockThreadStack() { gLockDepth--; }
};

class ThreadScanStartFailureAdversarial1 : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ResetFake();
        mManager = std::make_unique<TestThreadStackManager>();
        ASSERT_EQ(mManager->Init(), CHIP_NO_ERROR);
        gOt.setLinkModeLog.clear();
        gOt.setLinkModeLockDepth.clear();
    }

    void MakeSleepyEndDevice(bool deviceType = false, bool networkData = true)
    {
        gOt.linkMode.mRxOnWhenIdle = false;
        gOt.linkMode.mDeviceType   = deviceType;
        gOt.linkMode.mNetworkData  = networkData;
    }

    std::unique_ptr<TestThreadStackManager> mManager;
    ScanCallbackSpy mCallback;
};

using ThreadScanLinkModeGuardAdversarial1 = ThreadScanStartFailureAdversarial1;

TEST_F(ThreadScanStartFailureAdversarial1, RestoresRxOffOnSleepyEndDevice)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle);
}

TEST_F(ThreadScanStartFailureAdversarial1, PreservesOtherLinkModeBitsWhileRestoring)
{
    MakeSleepyEndDevice(/* deviceType */ false, /* networkData */ true);
    gOt.discoverResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle);
    EXPECT_FALSE(gOt.linkMode.mDeviceType);
    EXPECT_TRUE(gOt.linkMode.mNetworkData);
}

TEST_F(ThreadScanStartFailureAdversarial1, RestoresForEveryDiscoverError)
{
    const otError errors[] = { OT_ERROR_FAILED, OT_ERROR_BUSY, OT_ERROR_INVALID_STATE, OT_ERROR_INVALID_ARGS, OT_ERROR_NO_BUFS };

    for (otError err : errors)
    {
        ResetFake();
        MakeSleepyEndDevice();
        gOt.discoverResult = err;

        EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "otError " << static_cast<int>(err);
        EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle) << "otError " << static_cast<int>(err);
    }
}

TEST_F(ThreadScanStartFailureAdversarial1, RestoresAfterBringingIp6Up)
{
    MakeSleepyEndDevice();
    gOt.ip6Enabled     = false;
    gOt.discoverResult = OT_ERROR_NO_BUFS;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(gOt.ip6SetEnabledCalls, 1u);
    EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle);
}

TEST_F(ThreadScanStartFailureAdversarial1, RestoresWhileThreadStackLockHeld)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    ASSERT_GE(gOt.setLinkModeLockDepth.size(), 2u);
    EXPECT_GE(gOt.setLinkModeLockDepth.back(), 1);
    EXPECT_EQ(gLockDepth, 0);
}

TEST_F(ThreadScanStartFailureAdversarial1, RestoresOnEveryFailureInARow)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;

    for (unsigned attempt = 1; attempt <= 5; attempt++)
    {
        EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR) << "attempt " << attempt;
        EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle) << "attempt " << attempt;
        EXPECT_EQ(gOt.setLinkModeLog.size(), 2u * attempt) << "attempt " << attempt;
    }
}

TEST_F(ThreadScanStartFailureAdversarial1, LetsTheNextScanReApplyTheOverride)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_BUSY;
    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    gOt.setLinkModeLog.clear();
    gOt.discoverResult = OT_ERROR_NONE;
    EXPECT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    ASSERT_EQ(gOt.setLinkModeLog.size(), 1u);
    EXPECT_TRUE(gOt.setLinkModeLog.front().mRxOnWhenIdle);
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
}

TEST_F(ThreadScanLinkModeGuardAdversarial1, StartedScanKeepsRxOnUntilCompletion)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_NONE;

    EXPECT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
    EXPECT_EQ(gOt.setLinkModeLog.size(), 1u);
    EXPECT_EQ(gOt.discoverCalls, 1u);
    EXPECT_EQ(mCallback.mFinishedCount, 0u);
}

TEST_F(ThreadScanLinkModeGuardAdversarial1, FailureLeavesNonSleepyDeviceUntouched)
{
    gOt.linkMode.mRxOnWhenIdle = true;
    gOt.linkMode.mDeviceType   = true;
    gOt.discoverResult         = OT_ERROR_BUSY;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_TRUE(gOt.setLinkModeLog.empty());
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
    EXPECT_TRUE(gOt.linkMode.mDeviceType);
}

TEST_F(ThreadScanLinkModeGuardAdversarial1, Ip6EnableFailureNeverTouchesLinkMode)
{
    MakeSleepyEndDevice();
    gOt.ip6Enabled          = false;
    gOt.ip6SetEnabledResult = OT_ERROR_INVALID_STATE;

    EXPECT_NE(mManager->StartScan(&mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(gOt.discoverCalls, 0u);
    EXPECT_TRUE(gOt.setLinkModeLog.empty());
    EXPECT_FALSE(gOt.linkMode.mRxOnWhenIdle);
}

TEST_F(ThreadScanLinkModeGuardAdversarial1, SecondScanWhileOneIsInFlightTouchesNothing)
{
    MakeSleepyEndDevice();
    gOt.discoverResult = OT_ERROR_NONE;
    ASSERT_EQ(mManager->StartScan(&mCallback), CHIP_NO_ERROR);

    gOt.setLinkModeLog.clear();
    ScanCallbackSpy second;
    EXPECT_EQ(mManager->StartScan(&second), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(gOt.discoverCalls, 1u);
    EXPECT_TRUE(gOt.setLinkModeLog.empty());
    EXPECT_TRUE(gOt.linkMode.mRxOnWhenIdle);
}

} // namespace

#endif // CHIP_THREAD_SCAN_TEST_IMPL_PASS
