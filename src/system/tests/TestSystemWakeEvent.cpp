/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This is a unit test suite for <tt>chip::System::WakeEvent</tt>
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemError.h>
#include <system/SystemLayerImpl.h>

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

using namespace chip::System;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {
namespace System {
class WakeEventTest
{
public:
    static int GetReadFD(const WakeEvent & wakeEvent) { return wakeEvent.GetReadFD(); }
};
} // namespace System
} // namespace chip

namespace {

struct TestContext
{
    ::chip::System::LayerImpl mSystemLayer;
    WakeEvent mWakeEvent;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;

    TestContext()
    {
        mSystemLayer.Init();
        mWakeEvent.Open(mSystemLayer);
    }
    ~TestContext()
    {
        mWakeEvent.Close(mSystemLayer);
        mSystemLayer.Shutdown();
    }

    int SelectWakeEvent(timeval timeout = {})
    {
        // NOLINTBEGIN(clang-analyzer-security.insecureAPI.bzero)
        //
        // NOTE: darwin uses bzero to clear out FD sets. This is not a security concern.
        FD_ZERO(&mReadSet);
        FD_ZERO(&mWriteSet);
        FD_ZERO(&mErrorSet);
        // NOLINTEND(clang-analyzer-security.insecureAPI.bzero)

        FD_SET(WakeEventTest::GetReadFD(mWakeEvent), &mReadSet);
        return select(WakeEventTest::GetReadFD(mWakeEvent) + 1, &mReadSet, &mWriteSet, &mErrorSet, &timeout);
    }
};

void TestOpen(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    NL_TEST_ASSERT(inSuite, WakeEventTest::GetReadFD(lContext.mWakeEvent) >= 0);
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);
}

void TestNotify(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);

    // Check that select() succeeds after Notify() has been called
    lContext.mWakeEvent.Notify();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(WakeEventTest::GetReadFD(lContext.mWakeEvent), &lContext.mReadSet));

    // ...and state of the event is not cleared automatically
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(WakeEventTest::GetReadFD(lContext.mWakeEvent), &lContext.mReadSet));
}

void TestConfirm(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);

    // Check that select() succeeds after Notify() has been called
    lContext.mWakeEvent.Notify();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(WakeEventTest::GetReadFD(lContext.mWakeEvent), &lContext.mReadSet));

    // Check that Confirm() clears state of the event
    lContext.mWakeEvent.Confirm();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);
}

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
void * WaitForEvent(void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    // wait 5 seconds
    return reinterpret_cast<void *>(lContext.SelectWakeEvent(timeval{ 5, 0 }));
}

void TestBlockingSelect(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);

    // Spawn a thread waiting for the event
    pthread_t tid = 0;
    NL_TEST_ASSERT(inSuite, 0 == pthread_create(&tid, nullptr, WaitForEvent, aContext));

    lContext.mWakeEvent.Notify();
    void * selectResult = nullptr;
    NL_TEST_ASSERT(inSuite, 0 == pthread_join(tid, &selectResult));
    NL_TEST_ASSERT(inSuite, selectResult == reinterpret_cast<void *>(1));
}
#else  // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
void TestBlockingSelect(nlTestSuite *, void *) {}
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

void TestClose(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    lContext.mWakeEvent.Close(lContext.mSystemLayer);

    const auto notifFD = WakeEventTest::GetReadFD(lContext.mWakeEvent);

    // Check that Close() has cleaned up itself and reopen is possible
    NL_TEST_ASSERT(inSuite, lContext.mWakeEvent.Open(lContext.mSystemLayer) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, notifFD < 0);
}
} // namespace

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("WakeEvent::TestOpen",              TestOpen),
    NL_TEST_DEF("WakeEvent::TestNotify",            TestNotify),
    NL_TEST_DEF("WakeEvent::TestConfirm",           TestConfirm),
    NL_TEST_DEF("WakeEvent::TestBlockingSelect",    TestBlockingSelect),
    NL_TEST_DEF("WakeEvent::TestClose",             TestClose),
    NL_TEST_SENTINEL()
};
// clang-format on

static nlTestSuite kTheSuite = { "chip-system-wake-event", sTests };

int TestSystemWakeEvent(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemWakeEvent)
#else  // CHIP_SYSTEM_CONFIG_USE_SOCKETS
int TestSystemWakeEvent(void)
{
    return SUCCESS;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
