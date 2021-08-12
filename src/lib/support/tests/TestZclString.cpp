/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements a unit test suite for ZclString
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

#include <lib/support/ZclString.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

static void TestZclStringLessThanMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString = MutableByteSpan(bufferMemory);
    // Init c string
    char * cString64 = static_cast<char *>(MemoryCalloc(1024, true));
    // Add random chars to c string
    for (size_t i = 0; i < 64; i++)
        cString64[i] = 'A';

    MakeZclCharString(zclString, cString64);

    // Memmory assert
    NL_TEST_ASSERT(inSuite, cString64 != nullptr);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 64);

    chip::Platform::MemoryFree(cString64);
}

static void TestZclStringEqualsMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString = MutableByteSpan(bufferMemory);
    // Init c string
    char * cString254 = static_cast<char *>(MemoryCalloc(1024, true));
    // Add random chars to c string
    for (size_t i = 0; i < 254; i++)
        cString254[i] = 'A';

    MakeZclCharString(zclString, cString254);

    // Memmory assert
    NL_TEST_ASSERT(inSuite, cString254 != nullptr);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString254);
}

static void TestZclStringBiggerThanMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString = MutableByteSpan(bufferMemory);
    // Init c string
    char * cString255 = static_cast<char *>(MemoryCalloc(1024, true));
    // Add random chars to c string
    for (size_t i = 0; i < 255; i++)
        cString255[i] = 'A';

    MakeZclCharString(zclString, cString255);

    // Memmory assert
    NL_TEST_ASSERT(inSuite, cString255 != nullptr);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString255);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)

/**
 *  Set up the test suite.
 */
int TestZclString_Setup(void * inContext)
{
    CHIP_ERROR error = MemoryInit();
    if (error != CHIP_NO_ERROR)
        return (FAILURE);
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
int TestZclString_Teardown(void * inContext)
{
    MemoryShutdown();
    return (SUCCESS);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestZclStringLessThanMaximumSize), NL_TEST_DEF_FN(TestZclStringEqualsMaximumSize),
                                 NL_TEST_DEF_FN(TestZclStringBiggerThanMaximumSize), NL_TEST_SENTINEL() };

int TestZclString(void)
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestZclString_Setup, TestZclString_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestZclString)
