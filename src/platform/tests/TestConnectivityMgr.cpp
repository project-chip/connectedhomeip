/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements a unit test suite for the Connectivity Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_Init(nlTestSuite * inSuite, void * inContext)
{
    // ConfigurationManager is initialized from PlatformManager indirectly
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestConnectivityMgr_GetNetworkInterfaces(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NetworkInterface * netifs = nullptr;

    err = GetDiagnosticDataProvider().GetNetworkInterfaces(&netifs);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, netifs != nullptr);

    GetDiagnosticDataProvider().ReleaseNetworkInterfaces(netifs);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
    NL_TEST_DEF("Test ConfigurationMgr::GetNetworkInterfaces", TestConnectivityMgr_GetNetworkInterfaces), NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int TestConnectivityMgr_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestConnectivityMgr_Teardown(void * inContext)
{
    PlatformMgr().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestConnectivityMgr()
{
    nlTestSuite theSuite = { "ConfigurationMgr tests", &sTests[0], TestConnectivityMgr_Setup, TestConnectivityMgr_Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestConnectivityMgr)
