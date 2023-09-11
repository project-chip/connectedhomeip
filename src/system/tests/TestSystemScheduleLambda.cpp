/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>

// Test input data.

static void CheckScheduleLambda(nlTestSuite * inSuite, void * aContext)
{
    bool * called = new bool(false);
    chip::DeviceLayer::SystemLayer().ScheduleLambda([called] {
        *called = true;
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    });
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    NL_TEST_ASSERT(inSuite, *called);
    delete called;
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("System::TestScheduleLambda", CheckScheduleLambda),
    NL_TEST_SENTINEL()
};
// clang-format on

static int TestSetup(void * aContext);
static int TestTeardown(void * aContext);

// clang-format off
static nlTestSuite kTheSuite =
{
    "chip-system-schedule-lambda",
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
        return FAILURE;
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

int TestSystemScheduleLambda()
{
    // Run test suit againt one lContext.
    nlTestRunner(&kTheSuite, nullptr);

    return nlTestRunnerStats(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSystemScheduleLambda)
