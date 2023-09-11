/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements a unit test suite for the Configuration Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <app/FailSafeContext.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::DeviceLayer;

namespace {

constexpr FabricIndex kTestAccessingFabricIndex1 = 1;
constexpr FabricIndex kTestAccessingFabricIndex2 = 2;

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_Init(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestFailSafeContext_ArmFailSafe(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::app::FailSafeContext failSafeContext;

    err = failSafeContext.ArmFailSafe(kTestAccessingFabricIndex1, System::Clock::Seconds16(1));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, failSafeContext.IsFailSafeArmed() == true);
    NL_TEST_ASSERT(inSuite, failSafeContext.GetFabricIndex() == kTestAccessingFabricIndex1);
    NL_TEST_ASSERT(inSuite, failSafeContext.IsFailSafeArmed(kTestAccessingFabricIndex1) == true);
    NL_TEST_ASSERT(inSuite, failSafeContext.IsFailSafeArmed(kTestAccessingFabricIndex2) == false);

    failSafeContext.DisarmFailSafe();
    NL_TEST_ASSERT(inSuite, failSafeContext.IsFailSafeArmed() == false);
}

static void TestFailSafeContext_NocCommandInvoked(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::app::FailSafeContext failSafeContext;

    err = failSafeContext.ArmFailSafe(kTestAccessingFabricIndex1, System::Clock::Seconds16(1));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, failSafeContext.GetFabricIndex() == kTestAccessingFabricIndex1);

    failSafeContext.SetAddNocCommandInvoked(kTestAccessingFabricIndex2);
    NL_TEST_ASSERT(inSuite, failSafeContext.NocCommandHasBeenInvoked() == true);
    NL_TEST_ASSERT(inSuite, failSafeContext.AddNocCommandHasBeenInvoked() == true);
    NL_TEST_ASSERT(inSuite, failSafeContext.GetFabricIndex() == kTestAccessingFabricIndex2);

    failSafeContext.SetUpdateNocCommandInvoked();
    NL_TEST_ASSERT(inSuite, failSafeContext.NocCommandHasBeenInvoked() == true);
    NL_TEST_ASSERT(inSuite, failSafeContext.UpdateNocCommandHasBeenInvoked() == true);

    failSafeContext.DisarmFailSafe();
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
    NL_TEST_DEF("Test FailSafeContext::ArmFailSafe", TestFailSafeContext_ArmFailSafe),
    NL_TEST_DEF("Test FailSafeContext::NocCommandInvoked", TestFailSafeContext_NocCommandInvoked),

    NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int TestFailSafeContext_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestFailSafeContext_Teardown(void * inContext)
{
    PlatformMgr().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestFailSafeContext()
{
    nlTestSuite theSuite = { "FailSafeContext tests", &sTests[0], TestFailSafeContext_Setup, TestFailSafeContext_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFailSafeContext)
