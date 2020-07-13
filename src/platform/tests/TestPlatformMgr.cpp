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

#include "TestPlatformMgr.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nlunit-test.h>
#include <support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_Init(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestPlatformMgr_StartEventLoopTask(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = PlatformMgr().StartEventLoopTask();
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

    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
    NL_TEST_DEF("Test PlatformMgr::StartEventLoopTask", TestPlatformMgr_StartEventLoopTask),
    NL_TEST_DEF("Test PlatformMgr::TryLockChipStack", TestPlatformMgr_TryLockChipStack),
    NL_TEST_DEF("Test PlatformMgr::AddEventHandler", TestPlatformMgr_AddEventHandler),

    NL_TEST_SENTINEL()
};

int TestPlatformMgr(void)
{
    nlTestSuite theSuite = { "CHIP DeviceLayer time tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
