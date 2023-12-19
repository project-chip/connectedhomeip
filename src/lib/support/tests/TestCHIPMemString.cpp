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

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Platform;

// =================================
//      Unit tests
// =================================

namespace {
template <size_t kTestBufLen>
struct TestBuffers
{
    char correctSizeBuf[kTestBufLen];
    char tooSmallBuf[kTestBufLen - 1];
    char wayTooSmallBuf[1];
    char tooBigBuf[kTestBufLen + 10];
    void Reset()
    {
        memset(correctSizeBuf, 1, sizeof(correctSizeBuf));
        memset(tooSmallBuf, 1, sizeof(tooSmallBuf));
        memset(wayTooSmallBuf, 1, sizeof(wayTooSmallBuf));
        memset(tooBigBuf, 1, sizeof(tooBigBuf));
    }
    void CheckCorrectness(nlTestSuite * inSuite, const char * testStr)
    {
        // correctSizeBuf and tooBigBuf should have the complete string.
        NL_TEST_ASSERT(inSuite, correctSizeBuf[kTestBufLen - 1] == '\0');
        NL_TEST_ASSERT(inSuite, tooBigBuf[kTestBufLen - 1] == '\0');
        NL_TEST_ASSERT(inSuite, strcmp(correctSizeBuf, testStr) == 0);
        NL_TEST_ASSERT(inSuite, strcmp(tooBigBuf, testStr) == 0);
        NL_TEST_ASSERT(inSuite, strlen(correctSizeBuf) == strlen(testStr));
        NL_TEST_ASSERT(inSuite, strlen(tooBigBuf) == strlen(testStr));

        // wayTooSmallBuf is tiny and thus will only have the null terminator
        NL_TEST_ASSERT(inSuite, wayTooSmallBuf[0] == '\0');

        // tooSmallBuf should still have a null terminator on the end
        NL_TEST_ASSERT(inSuite, tooSmallBuf[kTestBufLen - 2] == '\0');
        NL_TEST_ASSERT(inSuite, memcmp(tooSmallBuf, testStr, kTestBufLen - 2) == 0);
    }
};

} // namespace

static void TestCopyString(nlTestSuite * inSuite, void * inContext)
{
    static constexpr char testWord[] = "testytest";
    ByteSpan testWordSpan            = ByteSpan(reinterpret_cast<const uint8_t *>(testWord), sizeof(testWord) - 1);
    CharSpan testWordSpan2(testWord, sizeof(testWord) - 1);
    TestBuffers<sizeof(testWord)> testBuffers;

    // CopyString with explicit size
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, sizeof(testBuffers.correctSizeBuf), testWord);
    CopyString(testBuffers.tooSmallBuf, sizeof(testBuffers.tooSmallBuf), testWord);
    CopyString(testBuffers.wayTooSmallBuf, sizeof(testBuffers.wayTooSmallBuf), testWord);
    CopyString(testBuffers.tooBigBuf, sizeof(testBuffers.tooBigBuf), testWord);
    testBuffers.CheckCorrectness(inSuite, testWord);

    // CopyString with array size
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, testWord);
    CopyString(testBuffers.tooSmallBuf, testWord);
    CopyString(testBuffers.wayTooSmallBuf, testWord);
    CopyString(testBuffers.tooBigBuf, testWord);
    testBuffers.CheckCorrectness(inSuite, testWord);

    // CopyString with explicit size from ByteSpan
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, sizeof(testBuffers.correctSizeBuf), testWordSpan);
    CopyString(testBuffers.tooSmallBuf, sizeof(testBuffers.tooSmallBuf), testWordSpan);
    CopyString(testBuffers.wayTooSmallBuf, sizeof(testBuffers.wayTooSmallBuf), testWordSpan);
    CopyString(testBuffers.tooBigBuf, sizeof(testBuffers.tooBigBuf), testWordSpan);
    testBuffers.CheckCorrectness(inSuite, testWord);

    // CopyString with array size from ByteSpan
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, testWordSpan);
    CopyString(testBuffers.tooSmallBuf, testWordSpan);
    CopyString(testBuffers.wayTooSmallBuf, testWordSpan);
    CopyString(testBuffers.tooBigBuf, testWordSpan);
    testBuffers.CheckCorrectness(inSuite, testWord);

    // CopyString with explicit size from CharSpan
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, sizeof(testBuffers.correctSizeBuf), testWordSpan2);
    CopyString(testBuffers.tooSmallBuf, sizeof(testBuffers.tooSmallBuf), testWordSpan2);
    CopyString(testBuffers.wayTooSmallBuf, sizeof(testBuffers.wayTooSmallBuf), testWordSpan2);
    CopyString(testBuffers.tooBigBuf, sizeof(testBuffers.tooBigBuf), testWordSpan2);
    testBuffers.CheckCorrectness(inSuite, testWord);

    // CopyString with array size from CharSpan
    testBuffers.Reset();
    CopyString(testBuffers.correctSizeBuf, testWordSpan2);
    CopyString(testBuffers.tooSmallBuf, testWordSpan2);
    CopyString(testBuffers.wayTooSmallBuf, testWordSpan2);
    CopyString(testBuffers.tooBigBuf, testWordSpan2);
    testBuffers.CheckCorrectness(inSuite, testWord);
}

static void TestMemoryAllocString(nlTestSuite * inSuite, void * inContext)
{
    static constexpr char testStr[] = "testytestString";
    char * allocatedStr             = MemoryAllocString(testStr, sizeof(testStr));
    NL_TEST_ASSERT(inSuite, allocatedStr != nullptr);
    if (allocatedStr == nullptr)
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, strcmp(testStr, allocatedStr) == 0);
    MemoryFree(allocatedStr);
}

static void TestScopedBuffer(nlTestSuite * inSuite, void * inContext)
{
    // Scoped buffer has its own tests that check the memory properly. Here we are just testing that the string is copied in
    // properly.
    static constexpr char testStr[] = "testytestString";
    ScopedMemoryString scopedString = ScopedMemoryString(testStr, sizeof(testStr));
    NL_TEST_ASSERT(inSuite, strcmp(scopedString.Get(), testStr) == 0);
}

static const nlTest sTests[] = { NL_TEST_DEF("Test CopyString", TestCopyString),
                                 NL_TEST_DEF("Test MemoryAllocString", TestMemoryAllocString),
                                 NL_TEST_DEF("Test ScopedBuffer", TestScopedBuffer), NL_TEST_SENTINEL() };

int TestMemString_Setup(void * inContext)
{
    CHIP_ERROR error = MemoryInit();
    if (error != CHIP_NO_ERROR)
        return (FAILURE);
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
int TestMemString_Teardown(void * inContext)
{
    MemoryShutdown();
    return (SUCCESS);
}

int TestMemString()
{
    nlTestSuite theSuite = { "CHIP Memory Allocation tests", &sTests[0], TestMemString_Setup, TestMemString_Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMemString)
