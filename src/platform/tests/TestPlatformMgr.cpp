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
 *      This file implements a unit test suite for the Platform Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nlunit-test.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_InitShutdown(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = PlatformMgr().Shutdown();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestPlatformMgr_BasicEventLoopTask(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = PlatformMgr().StartEventLoopTask();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = PlatformMgr().StopEventLoopTask();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = PlatformMgr().Shutdown();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static bool stopRan;

static void StopTheLoop(intptr_t)
{
    // Testing the return value here would involve multi-threaded access to the
    // nlTestSuite, and it's not clear whether that's OK.
    stopRan = true;
    PlatformMgr().StopEventLoopTask();
}

static void TestPlatformMgr_BasicRunEventLoop(nlTestSuite * inSuite, void * inContext)
{
    stopRan = false;

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork(StopTheLoop);

    PlatformMgr().RunEventLoop();
    NL_TEST_ASSERT(inSuite, stopRan);

    err = PlatformMgr().Shutdown();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static bool sleepRan;

static void SleepSome(intptr_t)
{
    sleep(1);
    sleepRan = true;
}

static void TestPlatformMgr_RunEventLoopTwoTasks(nlTestSuite * inSuite, void * inContext)
{
    stopRan  = false;
    sleepRan = false;

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork(SleepSome);
    PlatformMgr().ScheduleWork(StopTheLoop);

    PlatformMgr().RunEventLoop();
    NL_TEST_ASSERT(inSuite, stopRan);
    NL_TEST_ASSERT(inSuite, sleepRan);

    err = PlatformMgr().Shutdown();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void StopAndSleep(intptr_t arg)
{
    // Ensure that we don't proceed after stopping until the sleep is done too.
    StopTheLoop(arg);
    SleepSome(arg);
}

static void TestPlatformMgr_RunEventLoopStopBeforeSleep(nlTestSuite * inSuite, void * inContext)
{
    stopRan  = false;
    sleepRan = false;

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork(StopAndSleep);

    PlatformMgr().RunEventLoop();
    NL_TEST_ASSERT(inSuite, stopRan);
    NL_TEST_ASSERT(inSuite, sleepRan);

    err = PlatformMgr().Shutdown();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestPlatformMgr_TryLockChipStack(nlTestSuite * inSuite, void * inContext)
{
    bool locked = PlatformMgr().TryLockChipStack();
    if (locked)
        PlatformMgr().UnlockChipStack();
}

static int sEventRecieved = 0;

void DeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    // NL_TEST_ASSERT(inSuite, arg == 12345);
    sEventRecieved++;
}

static void TestPlatformMgr_AddEventHandler(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error;
    sEventRecieved = 0;
    error          = PlatformMgr().AddEventHandler(DeviceEventHandler, 12345);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

#if 0
    while (sEventRecieved == 0)
    {
    }

    NL_TEST_ASSERT(inSuite, sEventRecieved > 0);
#endif
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test PlatformMgr::Init/Shutdown", TestPlatformMgr_InitShutdown),
    NL_TEST_DEF("Test basic PlatformMgr::StartEventLoopTask", TestPlatformMgr_BasicEventLoopTask),
    NL_TEST_DEF("Test basic PlatformMgr::RunEventLoop", TestPlatformMgr_BasicRunEventLoop),
    NL_TEST_DEF("Test PlatformMgr::RunEventLoop with two tasks", TestPlatformMgr_RunEventLoopTwoTasks),
    NL_TEST_DEF("Test PlatformMgr::RunEventLoop with stop before sleep", TestPlatformMgr_RunEventLoopStopBeforeSleep),
    NL_TEST_DEF("Test PlatformMgr::TryLockChipStack", TestPlatformMgr_TryLockChipStack),
    NL_TEST_DEF("Test PlatformMgr::AddEventHandler", TestPlatformMgr_AddEventHandler),

    NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int TestPlatformMgr_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestPlatformMgr_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestPlatformMgr()
{
    nlTestSuite theSuite = { "PlatformMgr tests", &sTests[0], TestPlatformMgr_Setup, TestPlatformMgr_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPlatformMgr);
