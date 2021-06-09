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

#include <string.h>

#include <support/StringCircularBuffer.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestPushPop(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    const unsigned char s[] = { 'T', 'e', 's', 't' };
    unsigned char o[sizeof(s)];

    StringCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Push a string
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, sizeof(s))) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());

    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == sizeof(s));
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s, o, sizeof(s)) == 0);

    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Push a 0-length string
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 0)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());

    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 0);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
}

void TestPushInvalid(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    StringCircularBuffer buffer(storage, sizeof(storage));
    const unsigned char s[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 8)) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 9)) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 7)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
}

void TestPushWrap(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    const unsigned char s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const unsigned char s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    unsigned char o[sizeof(s1)];

    // Total space = 9
    StringCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, o, 4) == 0);

    // Used = 6, Free = 3, s1 was discarded due to lack of storage
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, o, 4) == 0);

    // Used = 9, Free = 0
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, o, 4) == 0);

    // Used = 3, Free = 6
    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 1);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(o) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, o, 1) == 0);
}

int Setup(void * inContext)
{
    return SUCCESS;
}

int Teardown(void * inContext)
{
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestPushPop), NL_TEST_DEF_FN(TestPushInvalid), NL_TEST_DEF_FN(TestPushWrap),
                                 NL_TEST_SENTINEL() };

int TestStringCircularBuffer()
{
    nlTestSuite theSuite = { "CHIP StringCircularBuffer tests", &sTests[0], Setup, Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStringCircularBuffer);
