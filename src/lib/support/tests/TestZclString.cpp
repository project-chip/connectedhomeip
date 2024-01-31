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
 *  This file implements a unit test suite for ZclString
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <lib/support/ZclString.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Platform;

bool allCharactersSame(uint8_t zclString[])
{
    int n = zclString[0];
    for (int i = 1; i < n; i++)
        if (zclString[i] != zclString[1])
            return false;

    return true;
}

static void TestZclStringWhenBufferIsZero(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[1];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString1;
    NL_TEST_ASSERT(inSuite, cString1.Calloc(1024));
    memset(cString1.Get(), 'A', 1);

    CHIP_ERROR err = MakeZclCharString(zclString, cString1.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);
}

static void TestZclStringLessThanMaximumSize_Length_64(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString64;
    NL_TEST_ASSERT(inSuite, cString64.Calloc(1024));
    memset(cString64.Get(), 'A', 64);

    CHIP_ERROR err = MakeZclCharString(zclString, cString64.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 64);
    NL_TEST_ASSERT(inSuite, allCharactersSame(zclString.data()) == true);
}

static void TestZclStringEqualsMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString254;
    NL_TEST_ASSERT(inSuite, cString254.Calloc(1024));
    memset(cString254.Get(), 'A', 254);

    CHIP_ERROR err = MakeZclCharString(zclString, cString254.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);
    NL_TEST_ASSERT(inSuite, allCharactersSame(zclString.data()) == true);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_255(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString255;
    NL_TEST_ASSERT(inSuite, cString255.Calloc(1024));
    memset(cString255.Get(), 'A', 255);

    CHIP_ERROR err = MakeZclCharString(zclString, cString255.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_256(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString256;
    NL_TEST_ASSERT(inSuite, cString256.Calloc(1024));
    memset(cString256.Get(), 'A', 256);

    CHIP_ERROR err = MakeZclCharString(zclString, cString256.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);
}

static void TestZclStringBiggerThanMaximumSize_Length_257(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[257];
    MutableByteSpan zclString(bufferMemory);
    chip::Platform::ScopedMemoryBuffer<char> cString257;
    NL_TEST_ASSERT(inSuite, cString257.Calloc(1024));
    memset(cString257.Get(), 'A', 257);

    CHIP_ERROR err = MakeZclCharString(zclString, cString257.Get());

    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);
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

int TestZclString()
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestZclString_Setup, TestZclString_Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestZclString)
