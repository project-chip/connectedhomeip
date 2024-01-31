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

#include <atomic>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <nlunit-test.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

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

    PlatformMgr().Shutdown();
}

static void TestPlatformMgr_BasicEventLoopTask(nlTestSuite * inSuite, void * inContext)
{
    std::atomic<int> counterRun{ 0 };

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Start/stop the event loop task a few times.
    for (size_t i = 0; i < 3; i++)
    {
        err = PlatformMgr().StartEventLoopTask();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        std::atomic<int> counterSync{ 2 };

        // Verify that the event loop will not exit until we tell it to by
        // scheduling few lambdas (for the test to pass, the event loop will
        // have to process more than one event).
        DeviceLayer::SystemLayer().ScheduleLambda([&]() {
            counterRun++;
            counterSync--;
        });

        // Sleep for a short time to allow the event loop to process the
        // scheduled event and go to idle state. Without this sleep, the
        // event loop may process both scheduled lambdas during single
        // iteration of the event loop which would defeat the purpose of
        // this test on POSIX platforms where the event loop is implemented
        // using a "do { ... } while (shouldRun)" construct.
        chip::test_utils::SleepMillis(10);

        DeviceLayer::SystemLayer().ScheduleLambda([&]() {
            counterRun++;
            counterSync--;
        });

        // Wait for the event loop to process the scheduled events.
        // Note, that we can not use any synchronization primitives like
        // condition variables or barriers, because the test has to compile
        // on all platforms. Instead we use a busy loop with a timeout.
        for (size_t t = 0; counterSync != 0 && t < 1000; t++)
            chip::test_utils::SleepMillis(1);

        err = PlatformMgr().StopEventLoopTask();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Sleep for a short time to allow the event loop to stop.
        // Note, in some platform implementations the event loop thread
        // is self-terminating. We need time to process the stopping event
        // inside event loop.
        chip::test_utils::SleepMillis(10);
    }

    NL_TEST_ASSERT(inSuite, counterRun == (3 * 2));

    PlatformMgr().Shutdown();
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

    PlatformMgr().Shutdown();
}

static bool sleepRan;

static void SleepSome(intptr_t)
{
    chip::test_utils::SleepMillis(1000);
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

    PlatformMgr().Shutdown();
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

    PlatformMgr().Shutdown();
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

class MockSystemLayer : public System::LayerImpl
{
public:
    CHIP_ERROR StartTimer(System::Clock::Timeout aDelay, System::TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_APPLICATION_ERROR(1);
    }
    CHIP_ERROR ScheduleWork(System::TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_APPLICATION_ERROR(2);
    }
};

static void TestPlatformMgr_MockSystemLayer(nlTestSuite * inSuite, void * inContext)
{
    MockSystemLayer systemLayer;

    DeviceLayer::SetSystemLayerForTesting(&systemLayer);
    NL_TEST_ASSERT(inSuite, &DeviceLayer::SystemLayer() == static_cast<chip::System::Layer *>(&systemLayer));

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, &DeviceLayer::SystemLayer() == static_cast<chip::System::Layer *>(&systemLayer));

    NL_TEST_ASSERT(
        inSuite, DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::kZero, nullptr, nullptr) == CHIP_APPLICATION_ERROR(1));
    NL_TEST_ASSERT(inSuite, DeviceLayer::SystemLayer().ScheduleWork(nullptr, nullptr) == CHIP_APPLICATION_ERROR(2));

    PlatformMgr().Shutdown();

    DeviceLayer::SetSystemLayerForTesting(nullptr);
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
    NL_TEST_DEF("Test mock System::Layer", TestPlatformMgr_MockSystemLayer),

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

    // Setup a fake commissionable data provider since required by internals of several
    // Device/SystemLayer components.
    static chip::DeviceLayer::TestOnlyCommissionableDataProvider commissionable_data_provider;
    chip::DeviceLayer::SetCommissionableDataProvider(&commissionable_data_provider);

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

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPlatformMgr);
