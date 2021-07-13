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
 *      This file implements a unit test suite for CHIP SafeInt functions
 *
 */

#include <support/Span.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

static void TestByteSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };

    ByteSpan s0 = ByteSpan();
    NL_TEST_ASSERT(inSuite, s0.data() == nullptr);
    NL_TEST_ASSERT(inSuite, s0.size() == 0);
    NL_TEST_ASSERT(inSuite, s0.empty());
    NL_TEST_ASSERT(inSuite, s0.data_equal(s0));

    ByteSpan s1(arr, 2);
    NL_TEST_ASSERT(inSuite, s1.data() == arr);
    NL_TEST_ASSERT(inSuite, s1.size() == 2);
    NL_TEST_ASSERT(inSuite, !s1.empty());
    NL_TEST_ASSERT(inSuite, s1.data_equal(s1));
    NL_TEST_ASSERT(inSuite, !s1.data_equal(s0));

    ByteSpan s2(arr);
    NL_TEST_ASSERT(inSuite, s2.data() == arr);
    NL_TEST_ASSERT(inSuite, s2.size() == 3);
    NL_TEST_ASSERT(inSuite, s2.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s2.empty());
    NL_TEST_ASSERT(inSuite, s2.data_equal(s2));
    NL_TEST_ASSERT(inSuite, !s2.data_equal(s1));

    ByteSpan s3 = s2;
    NL_TEST_ASSERT(inSuite, s3.data() == arr);
    NL_TEST_ASSERT(inSuite, s3.size() == 3);
    NL_TEST_ASSERT(inSuite, s3.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s3.empty());
    NL_TEST_ASSERT(inSuite, s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    ByteSpan s4(arr2);
    NL_TEST_ASSERT(inSuite, !s4.data_equal(s2));

    ByteSpan s5(arr2, 0);
    NL_TEST_ASSERT(inSuite, s5.data() != nullptr);
    NL_TEST_ASSERT(inSuite, !s5.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s5.data_equal(s0));
    NL_TEST_ASSERT(inSuite, s0.data_equal(s5));

    ByteSpan s6(arr2);
    s6.reduce_size(2);
    NL_TEST_ASSERT(inSuite, s6.size() == 2);
    ByteSpan s7(arr2, 2);
    NL_TEST_ASSERT(inSuite, s6.data_equal(s7));
    NL_TEST_ASSERT(inSuite, s7.data_equal(s6));
}

static void TestMutableByteSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };

    MutableByteSpan s0 = MutableByteSpan();
    NL_TEST_ASSERT(inSuite, s0.data() == nullptr);
    NL_TEST_ASSERT(inSuite, s0.size() == 0);
    NL_TEST_ASSERT(inSuite, s0.empty());
    NL_TEST_ASSERT(inSuite, s0.data_equal(s0));

    MutableByteSpan s1(arr, 2);
    NL_TEST_ASSERT(inSuite, s1.data() == arr);
    NL_TEST_ASSERT(inSuite, s1.size() == 2);
    NL_TEST_ASSERT(inSuite, !s1.empty());
    NL_TEST_ASSERT(inSuite, s1.data_equal(s1));
    NL_TEST_ASSERT(inSuite, !s1.data_equal(s0));

    MutableByteSpan s2(arr);
    NL_TEST_ASSERT(inSuite, s2.data() == arr);
    NL_TEST_ASSERT(inSuite, s2.size() == 3);
    NL_TEST_ASSERT(inSuite, s2.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s2.empty());
    NL_TEST_ASSERT(inSuite, s2.data_equal(s2));
    NL_TEST_ASSERT(inSuite, !s2.data_equal(s1));

    MutableByteSpan s3 = s2;
    NL_TEST_ASSERT(inSuite, s3.data() == arr);
    NL_TEST_ASSERT(inSuite, s3.size() == 3);
    NL_TEST_ASSERT(inSuite, s3.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s3.empty());
    NL_TEST_ASSERT(inSuite, s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    MutableByteSpan s4(arr2);
    NL_TEST_ASSERT(inSuite, !s4.data_equal(s2));

    MutableByteSpan s5(arr2, 0);
    NL_TEST_ASSERT(inSuite, s5.data() != nullptr);
    NL_TEST_ASSERT(inSuite, !s5.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s5.data_equal(s0));
    NL_TEST_ASSERT(inSuite, s0.data_equal(s5));

    MutableByteSpan s6(arr2);
    s6.reduce_size(2);
    NL_TEST_ASSERT(inSuite, s6.size() == 2);
    MutableByteSpan s7(arr2, 2);
    NL_TEST_ASSERT(inSuite, s6.data_equal(s7));
    NL_TEST_ASSERT(inSuite, s7.data_equal(s6));

    uint8_t arr3[] = { 1, 2, 3 };
    MutableByteSpan s8(arr3);
    NL_TEST_ASSERT(inSuite, arr3[1] == 2);
    s8.data()[1] = 3;
    NL_TEST_ASSERT(inSuite, arr3[1] == 3);

    // Not mutable span on purpose, to test conversion.
    ByteSpan s9 = s8;
    NL_TEST_ASSERT(inSuite, s9.data_equal(s8));
    NL_TEST_ASSERT(inSuite, s8.data_equal(s9));
}

static void TestFixedByteSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };

    FixedByteSpan<3> s0 = FixedByteSpan<3>();
    NL_TEST_ASSERT(inSuite, s0.data() == nullptr);
    NL_TEST_ASSERT(inSuite, s0.size() == 3);
    NL_TEST_ASSERT(inSuite, s0.empty());
    NL_TEST_ASSERT(inSuite, s0.data_equal(s0));

    FixedByteSpan<2> s1(arr);
    NL_TEST_ASSERT(inSuite, s1.data() == arr);
    NL_TEST_ASSERT(inSuite, s1.size() == 2);
    NL_TEST_ASSERT(inSuite, !s1.empty());
    NL_TEST_ASSERT(inSuite, s1.data_equal(s1));

    FixedByteSpan<3> s2(arr);
    NL_TEST_ASSERT(inSuite, s2.data() == arr);
    NL_TEST_ASSERT(inSuite, s2.size() == 3);
    NL_TEST_ASSERT(inSuite, s2.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s2.empty());
    NL_TEST_ASSERT(inSuite, s2.data_equal(s2));

    FixedByteSpan<3> s3 = s2;
    NL_TEST_ASSERT(inSuite, s3.data() == arr);
    NL_TEST_ASSERT(inSuite, s3.size() == 3);
    NL_TEST_ASSERT(inSuite, s3.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, !s3.empty());
    NL_TEST_ASSERT(inSuite, s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    FixedByteSpan<3> s4(arr2);
    NL_TEST_ASSERT(inSuite, !s4.data_equal(s2));

    size_t idx = 0;
    for (auto & entry : s4)
    {
        NL_TEST_ASSERT(inSuite, entry == arr2[idx++]);
    }
    NL_TEST_ASSERT(inSuite, idx == 3);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestByteSpan), NL_TEST_DEF_FN(TestMutableByteSpan),
                                 NL_TEST_DEF_FN(TestFixedByteSpan), NL_TEST_SENTINEL() };

int TestSpan(void)
{
    nlTestSuite theSuite = { "CHIP Span tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSpan)
