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
#include <support/ScopedBuffer.h>
#include <support/UnitTestRegistration.h>

#include <lib/support/ZclString.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

static void TestZclStringWhenBufferIsZero(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[1];
    MutableByteSpan zclString(bufferMemory);
    char * cString1 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString1, 'A', 1);
    NL_TEST_ASSERT(inSuite, cString1 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString1);

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);

    chip::Platform::MemoryFree(cString1);
}

static void TestZclStringLessThanMaximumSize_Length_64(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    char * cString64 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString64, 'A', 64);
    NL_TEST_ASSERT(inSuite, cString64 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString64);

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 64);

    chip::Platform::MemoryFree(cString64);
}

static void TestZclStringEqualsMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    char * cString254 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString254, 'A', 254);
    NL_TEST_ASSERT(inSuite, cString254 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString254);

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString254);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_255(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[255];
    MutableByteSpan zclString(bufferMemory);
    char * cString255 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString255, 'A', 255);
    NL_TEST_ASSERT(inSuite, cString255 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString255);

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);

    chip::Platform::MemoryFree(cString255);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_256(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    char * cString256 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString256, 'A', 256);
    NL_TEST_ASSERT(inSuite, cString256 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString256);

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);

    chip::Platform::MemoryFree(cString256);
}

static void TestZclStringBiggerThanMaximumSize_Length_257(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[257];
    MutableByteSpan zclString(bufferMemory);
    char * cString257 = static_cast<char *>(MemoryCalloc(1024, 1));
    memset(cString257, 'A', 257);
    NL_TEST_ASSERT(inSuite, cString257 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString257);

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);

    chip::Platform::MemoryFree(cString257);
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
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestZclStringWhenBufferIsZero),
                                 NL_TEST_DEF_FN(TestZclStringLessThanMaximumSize_Length_64),
                                 NL_TEST_DEF_FN(TestZclStringEqualsMaximumSize),
                                 NL_TEST_DEF_FN(TestSizeZclStringBiggerThanMaximumSize_Length_255),
                                 NL_TEST_DEF_FN(TestSizeZclStringBiggerThanMaximumSize_Length_256),
                                 NL_TEST_DEF_FN(TestZclStringBiggerThanMaximumSize_Length_257),
                                 NL_TEST_SENTINEL() };

int TestZclString(void)
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestZclString_Setup, TestZclString_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestZclString)
