/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <system/SystemConfig.h>

#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>

static void IncrementIntCounter(chip::System::Layer *, void * state)
{
    ++(*static_cast<int *>(state));
}

static void StopEventLoop(chip::System::Layer *, void *)
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

static void CheckScheduleWorkTwice(nlTestSuite * inSuite, void * aContext)
{
    int * callCount = new int(0);
    NL_TEST_ASSERT(inSuite, chip::DeviceLayer::SystemLayer().ScheduleWork(IncrementIntCounter, callCount) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, chip::DeviceLayer::SystemLayer().ScheduleWork(IncrementIntCounter, callCount) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, chip::DeviceLayer::SystemLayer().ScheduleWork(StopEventLoop, nullptr) == CHIP_NO_ERROR);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    NL_TEST_ASSERT(inSuite, *callCount == 2);
    delete callCount;
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("System::TestScheduleWorkTwice", CheckScheduleWorkTwice),
    NL_TEST_SENTINEL()
};
// clang-format on

static int TestSetup(void * aContext);
static int TestTeardown(void * aContext);

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-schedule-work",
    &sTests[0],
    TestSetup,
    TestTeardown
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * aContext)
{
    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

    if (chip::DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
        return FAILURE;

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * aContext)
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
    chip::Platform::MemoryShutdown();
    return (SUCCESS);
}

int TestSystemScheduleWork()
{
    // Run test suit againt one lContext.
    nlTestRunner(&kTheSuite, nullptr);

    return nlTestRunnerStats(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemScheduleWork)
