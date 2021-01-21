/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a unit test suite for CHIP BufferReader
 *
 */

#include <support/BufferReader.h>
#include <support/UnitTestRegistration.h>
#include <type_traits>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Encoding::LittleEndian;

static const uint8_t test_buffer[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };

struct TestReader : public Reader
{
    TestReader() : Reader(test_buffer, std::extent<decltype(test_buffer)>::value) {}
};

static void TestBufferReader_Basic(nlTestSuite * inSuite, void * inContext)
{
    TestReader reader;
    uint8_t first;
    uint16_t second;
    uint32_t third;
    uint64_t fourth;
    CHIP_ERROR err = reader.Read8(&first).Read16(&second).Read32(&third).Read64(&fourth).StatusCode();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, first == 0x01);
    NL_TEST_ASSERT(inSuite, second == 0x0302);
    NL_TEST_ASSERT(inSuite, third == 0x07060504);
    NL_TEST_ASSERT(inSuite, fourth == 0x0f0e0d0c0b0a0908);
    NL_TEST_ASSERT(inSuite, reader.OctetsRead() == 15);
    NL_TEST_ASSERT(inSuite, reader.Remaining() == 3);
    NL_TEST_ASSERT(inSuite, reader.HasAtLeast(2));
    NL_TEST_ASSERT(inSuite, reader.HasAtLeast(3));
    NL_TEST_ASSERT(inSuite, !reader.HasAtLeast(4));

    uint32_t fourMore;
    err = reader.Read32(&fourMore).StatusCode();
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

static void TestBufferReader_Saturation(nlTestSuite * inSuite, void * inContext)
{
    TestReader reader;
    uint64_t temp;
    // Read some bytes out so we can get to the end of the buffer.
    CHIP_ERROR err = reader.Read64(&temp).StatusCode();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = reader.Read64(&temp).StatusCode();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, reader.HasAtLeast(2));
    NL_TEST_ASSERT(inSuite, !reader.HasAtLeast(3));
    uint32_t tooBig;
    err = reader.Read32(&tooBig).StatusCode();
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !reader.HasAtLeast(1));

    // Check that even though we only really read out 16 bytes, we can't read
    // out one more bytes, because our previous read failed.
    uint8_t small;
    err = reader.Read8(&small).StatusCode();
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestBufferReader_Basic), NL_TEST_DEF_FN(TestBufferReader_Saturation),
                                 NL_TEST_SENTINEL() };

int TestBufferReader(void)
{
    nlTestSuite theSuite = { "CHIP BufferReader tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBufferReader)
