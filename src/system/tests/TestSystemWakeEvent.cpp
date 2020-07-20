/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This is a unit test suite for <tt>chip::System::SystemWakeEvent</tt>
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
// config
#include <system/SystemConfig.h>

// module header
#include "TestSystemLayer.h"

#include <nlunit-test.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/TestUtils.h>
#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemWakeEvent.h>

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

using chip::ErrorStr;
using namespace chip::System;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
struct TestContext
{
    SystemWakeEvent mWakeEvent;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;

    TestContext() { mWakeEvent.Open(); }
    ~TestContext() { mWakeEvent.Close(); }

    int SelectWakeEvent(timeval timeout = {})
    {
        FD_ZERO(&mReadSet);
        FD_ZERO(&mWriteSet);
        FD_ZERO(&mErrorSet);
        FD_SET(mWakeEvent.GetNotifFD(), &mReadSet);
        return select(mWakeEvent.GetNotifFD() + 1, &mReadSet, &mWriteSet, &mErrorSet, &timeout);
    }
};

// Test input data.

static void TestOpen(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    NL_TEST_ASSERT(inSuite, lContext.mWakeEvent.GetNotifFD() >= 0);
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);
}

static void TestNotify(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);

    // Check that select() succeeds after Notify() has been called
    lContext.mWakeEvent.Notify();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(lContext.mWakeEvent.GetNotifFD(), &lContext.mReadSet));

    // ...and state of the event is not cleared automatically
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(lContext.mWakeEvent.GetNotifFD(), &lContext.mReadSet));
}

static void TestConfirm(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);

    // Check that select() succeeds after Notify() has been called
    lContext.mWakeEvent.Notify();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 1);
    NL_TEST_ASSERT(inSuite, FD_ISSET(lContext.mWakeEvent.GetNotifFD(), &lContext.mReadSet));

    // Check that Confirm() clears state of the event
    lContext.mWakeEvent.Confirm();
    NL_TEST_ASSERT(inSuite, lContext.SelectWakeEvent() == 0);
}

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
static void * WaitForEvent(void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    // wait 5 seconds
    return reinterpret_cast<void *>(lContext.SelectWakeEvent(timeval{ 5, 0 }));
}

static void TestBlockingSelect(nlTestSuite * inSuite, void * aContext)
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
static void TestBlockingSelect(nlTestSuite *, void *) {}
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

static void TestClose(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    lContext.mWakeEvent.Close();

    const auto notifFD = lContext.mWakeEvent.GetNotifFD();

    // Check that Close() has cleaned up itself and reopen is possible
    NL_TEST_ASSERT(inSuite, lContext.mWakeEvent.Open() == CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, notifFD < 0);
}

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

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-wake-event",
    sTests
};
// clang-format on

int TestSystemWakeEvent(void)
{
    TestContext context;

    // Run test suit againt one lContext.
    nlTestRunner(&kTheSuite, &context);

    return nlTestRunnerStats(&kTheSuite);
}
#else  // CHIP_SYSTEM_CONFIG_USE_SOCKETS
int TestSystemWakeEvent(void)
{
    return SUCCESS;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

static void __attribute__((constructor)) TestSystemWakeEventCtor(void)
{
    VerifyOrDie(chip::RegisterUnitTests(&TestSystemWakeEvent) == CHIP_NO_ERROR);
}
