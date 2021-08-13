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

static void TestZclStringWhenBufferIsZero(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[1];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString64 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 64; i++)
        cString64[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString64 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString64);

    // ZCL String is created error of inbound message too big
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 0);

    chip::Platform::MemoryFree(cString64);
}

static void TestZclStringLessThanMaximumSize_Length_64(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString64 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 64; i++)
        cString64[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString64 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString64);

    // ZCL String is created with no errors
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 64);

    chip::Platform::MemoryFree(cString64);
}

static void TestZclStringEqualsMaximumSize(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString254 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 254; i++)
        cString254[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString254 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString254);

    // ZCL String is created with no errors
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString254);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_255(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[255];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString255 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 255; i++)
        cString255[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString255 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString255);

    // ZCL String is created error of inbound message too big
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString255);
}

static void TestSizeZclStringBiggerThanMaximumSize_Length_256(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[256];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString256 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 256; i++)
        cString256[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString256 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString256);

    // ZCL String is created error of inbound message too big
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

    chip::Platform::MemoryFree(cString256);
}

static void TestZclStringBiggerThanMaximumSize_Length_257(nlTestSuite * inSuite, void * inContext)
{
    uint8_t bufferMemory[257];
    // Init zcl string
    MutableByteSpan zclString(bufferMemory);
    // Init c string
    char * cString257 = static_cast<char *>(MemoryCalloc(1024, 1));
    // Add random chars to c string
    for (size_t i = 0; i < 257; i++)
        cString257[i] = 'A';

    // Verify cString is properly initialized
    NL_TEST_ASSERT(inSuite, cString257 != nullptr);

    CHIP_ERROR err = MakeZclCharString(zclString, cString257);

    // ZCL String is created error of inbound message too big
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    // ZCL String data is not null
    NL_TEST_ASSERT(inSuite, zclString.data() != NULL);
    // ZCL String length assert
    NL_TEST_ASSERT(inSuite, zclString.data()[0] == 254);

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
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestZclStringWhenBufferIsZero), NL_TEST_DEF_FN(TestZclStringLessThanMaximumSize_Length_64), 
                                 NL_TEST_DEF_FN(TestZclStringEqualsMaximumSize), NL_TEST_DEF_FN(TestSizeZclStringBiggerThanMaximumSize_Length_255), 
                                 NL_TEST_DEF_FN(TestSizeZclStringBiggerThanMaximumSize_Length_256), NL_TEST_DEF_FN(TestZclStringBiggerThanMaximumSize_Length_257), 
                                 NL_TEST_SENTINEL() };

int TestZclString(void)
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestZclString_Setup, TestZclString_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestZclString)
