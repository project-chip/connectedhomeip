/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements a unit test suite for
 *      <tt>chip::Inet::InetTimer</tt>, a class that provides timers
 *      for the Inet library.
 *
 */

#include "TestInetLayer.h"

#include <inet/InetConfig.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <string.h>

#include <inet/InetLayer.h>
#include <inet/InetTimer.h>

#include <nlunit-test.h>

//#include "ToolCommon.h"

#define MEM_ALIGN_SIZE(SZ, ALIGN) (((SZ) + (ALIGN) -1) & ~((ALIGN) -1))
#define INET_BUF_SIZE 1536
#define INET_BUF_HEADER_SIZE MEM_ALIGN_SIZE(sizeof(InetBuffer), 4)
#define INET_TO_PBUF(x) (static_cast<struct pbuf *>(static_cast<void *>(x)))

using namespace chip::Inet;

// Test input vector format.

struct TestContext
{
    InetLayer * mInet;
    nlTestSuite * mTestSuite;
};

// Test input data.

static struct TestContext sContext;

static volatile bool sOverflowTestDone;

void HandleTimer0Failed(InetLayer * inetLayer, void * appState, INET_ERROR err)
{
    TestContext * context = static_cast<TestContext *>(appState);
    NL_TEST_ASSERT(context->mTestSuite, false);
    sOverflowTestDone = true;
}

void HandleTimer1Failed(InetLayer * inetLayer, void * appState, INET_ERROR err)
{
    TestContext * context = static_cast<TestContext *>(appState);
    NL_TEST_ASSERT(context->mTestSuite, false);
    sOverflowTestDone = true;
}

void HandleTimer10Success(InetLayer * inetLayer, void * appState, INET_ERROR err)
{
    TestContext * context = static_cast<TestContext *>(appState);
    NL_TEST_ASSERT(context->mTestSuite, true);
    sOverflowTestDone = true;
}

static void CheckOverflow(nlTestSuite * inSuite, void * inContext)
{
    uint32_t timeout_overflow_0ms = 652835029;
    uint32_t timeout_overflow_1ms = 1958505088;
    uint32_t timeout_10ms         = 10;

    TestContext * context = static_cast<TestContext *>(inContext);

    sOverflowTestDone = false;
    context->mInet->StartTimer(timeout_overflow_0ms, HandleTimer0Failed, inContext);
    context->mInet->StartTimer(timeout_overflow_1ms, HandleTimer1Failed, inContext);
    context->mInet->StartTimer(timeout_10ms, HandleTimer10Success, inContext);
    while (!sOverflowTestDone)
    {
        struct timeval sleepTime;
        sleepTime.tv_sec  = 0;
        sleepTime.tv_usec = 1000; // 1 ms tick
        ServiceNetwork(sleepTime);
    }
    context->mInet->CancelTimer(HandleTimer0Failed, inContext);
    context->mInet->CancelTimer(HandleTimer1Failed, inContext);
    context->mInet->CancelTimer(HandleTimer10Success, inContext);
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("InetTimer::TestOverflow", CheckOverflow), NL_TEST_SENTINEL() };

/**
 *  Set up the test suite.
 *  This is a work-around to initiate InetBuffer protected class instance's
 *  data and set it to a known state, before an instance is created.
 */
static int TestSetup(void * inContext)
{
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * inContext)
{
    return (SUCCESS);
}

int TestInetTimer(void)
{
#if INET_SOCKETS
    // clang-format off
    nlTestSuite theSuite =
    {
        "inet-timer",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    InitSystemLayer();
    InitNetwork();

    sContext.mInet      = &gInet;
    sContext.mTestSuite = &theSuite;

    // Run test suit againt one context.
    nlTestRunner(&theSuite, &sContext);

    return (nlTestRunnerStats(&theSuite));
#else  // !INET_SOCKETS
    return (0);
#endif // !INET_SOCKETS
}

static void __attribute__((constructor)) TestCHIPInetTimerCtor(void)
{
    VerifyOrDie(RegisterUnitTests(&TestInetTimer) == CHIP_NO_ERROR);
}

#else // !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

int TestInetTimer(void)
{
    return (0);
}

#endif // !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
