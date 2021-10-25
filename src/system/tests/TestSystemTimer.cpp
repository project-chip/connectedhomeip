/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This is a unit test suite for <tt>chip::System::Timer</tt>,
 *      the part of the CHIP System Layer that implements timers.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemError.h>
#include <system/SystemLayerImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <errno.h>
#include <stdint.h>
#include <string.h>

using chip::ErrorStr;
using namespace chip::System;

static void ServiceEvents(Layer & aLayer)
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    static_cast<LayerSocketsLoop &>(aLayer).PrepareEvents();
    static_cast<LayerSocketsLoop &>(aLayer).WaitForEvents();
    static_cast<LayerSocketsLoop &>(aLayer).HandleEvents();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (aLayer.IsInitialized())
    {
        static_cast<LayerImplLwIP &>(aLayer).HandlePlatformTimer();
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

// Test input vector format.
static const uint32_t MAX_NUM_TIMERS = 1000;

class TestContext
{
public:
    Layer * mLayer;
    nlTestSuite * mTestSuite;
    chip::Callback::Callback<> mGreedyTimer; // for greedy timer
    uint32_t mNumTimersHandled;

    void GreedyTimer()
    {
        NL_TEST_ASSERT(mTestSuite, mNumTimersHandled < MAX_NUM_TIMERS);

        if (mNumTimersHandled >= MAX_NUM_TIMERS)
        {
            return;
        }

        mNumTimersHandled++;
    }
    static void GreedyTimer(void * p)
    {
        TestContext * lContext = static_cast<TestContext *>(p);
        lContext->GreedyTimer();
    }

    TestContext() : mGreedyTimer(GreedyTimer, this), mNumTimersHandled(0) {}
};

// Test input data.

static volatile bool sOverflowTestDone;

void TimerFailed(void * aState)
{
    TestContext * lContext = static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext->mTestSuite, false);
    sOverflowTestDone = true;
}

void HandleTimerFailed(Layer * inetLayer, void * aState)
{
    (void) inetLayer;
    TimerFailed(aState);
}

void HandleTimer10Success(Layer * inetLayer, void * aState)
{
    TestContext & lContext = *static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext.mTestSuite, true);
    sOverflowTestDone = true;
}

static void CheckOverflow(nlTestSuite * inSuite, void * aContext)
{
    chip::System::Clock::Milliseconds32 timeout_overflow_0ms{ 652835029 };
    chip::System::Clock::Milliseconds32 timeout_10ms{ 10 };

    TestContext & lContext = *static_cast<TestContext *>(aContext);
    Layer & lSys           = *lContext.mLayer;

    sOverflowTestDone = false;

    lSys.StartTimer(timeout_overflow_0ms, HandleTimerFailed, aContext);
    lSys.StartTimer(timeout_10ms, HandleTimer10Success, aContext);

    while (!sOverflowTestDone)
    {
        ServiceEvents(lSys);
    }

    lSys.CancelTimer(HandleTimerFailed, aContext);
    // cb  timer is cancelled by destructor
    lSys.CancelTimer(HandleTimer10Success, aContext);
}

void HandleGreedyTimer(Layer * aLayer, void * aState)
{
    static uint32_t sNumTimersHandled = 0;
    TestContext & lContext            = *static_cast<TestContext *>(aState);
    NL_TEST_ASSERT(lContext.mTestSuite, sNumTimersHandled < MAX_NUM_TIMERS);

    if (sNumTimersHandled >= MAX_NUM_TIMERS)
    {
        return;
    }

    aLayer->StartTimer(chip::System::Clock::Zero, HandleGreedyTimer, aState);
    sNumTimersHandled++;
}

static void CheckStarvation(nlTestSuite * inSuite, void * aContext)
{
    TestContext & lContext = *static_cast<TestContext *>(aContext);
    Layer & lSys           = *lContext.mLayer;

    lSys.StartTimer(chip::System::Clock::Zero, HandleGreedyTimer, aContext);

    ServiceEvents(lSys);
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Timer::TestOverflow",             CheckOverflow),
    NL_TEST_DEF("Timer::TestTimerStarvation",      CheckStarvation),
    NL_TEST_SENTINEL()
};
// clang-format on

static int TestSetup(void * aContext);
static int TestTeardown(void * aContext);

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-timer",
    &sTests[0],
    TestSetup,
    TestTeardown
};
// clang-format on

static LayerImpl sLayer;

/**
 *  Set up the test suite.
 */
static int TestSetup(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_VERSION_MAJOR <= 2 && LWIP_VERSION_MINOR < 1
    static sys_mbox_t * sLwIPEventQueue = NULL;

    sys_mbox_new(sLwIPEventQueue, 100);
    tcpip_init(NULL, NULL);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_VERSION_MAJOR <= 2 && LWIP_VERSION_MINOR < 1

    sLayer.Init();

    lContext.mLayer     = &sLayer;
    lContext.mTestSuite = &kTheSuite;

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * aContext)
{
    TestContext & lContext = *reinterpret_cast<TestContext *>(aContext);

    lContext.mLayer->Shutdown();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !(LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
    tcpip_finish(NULL, NULL);
#endif
#endif

    return (SUCCESS);
}

int TestSystemTimer(void)
{
    TestContext context;

    // Run test suit againt one lContext.
    nlTestRunner(&kTheSuite, &context);

    return nlTestRunnerStats(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemTimer)
