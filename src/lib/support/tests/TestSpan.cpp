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

#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#include <array>

using namespace chip;

static void TestByteSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };

    ByteSpan s0 = ByteSpan();
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
    NL_TEST_ASSERT(inSuite, s2.front() == 1);
    NL_TEST_ASSERT(inSuite, s2.back() == 3);
    NL_TEST_ASSERT(inSuite, s2[0] == 1);
    NL_TEST_ASSERT(inSuite, s2[1] == 2);
    NL_TEST_ASSERT(inSuite, s2[2] == 3);

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

    // Not mutable span on purpose.
    ByteSpan s10(s8);
    NL_TEST_ASSERT(inSuite, s10.data_equal(s8));
    NL_TEST_ASSERT(inSuite, s8.data_equal(s10));
}

static void TestFixedByteSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };

    FixedByteSpan<3> s0 = FixedByteSpan<3>();
    NL_TEST_ASSERT(inSuite, s0.data() != nullptr);
    NL_TEST_ASSERT(inSuite, s0.size() == 3);
    NL_TEST_ASSERT(inSuite, s0.data_equal(s0));
    NL_TEST_ASSERT(inSuite, s0[0] == 0);
    NL_TEST_ASSERT(inSuite, s0[1] == 0);
    NL_TEST_ASSERT(inSuite, s0[2] == 0);

    FixedByteSpan<2> s1(arr);
    NL_TEST_ASSERT(inSuite, s1.data() == arr);
    NL_TEST_ASSERT(inSuite, s1.size() == 2);
    NL_TEST_ASSERT(inSuite, s1.data_equal(s1));

    FixedByteSpan<3> s2(arr);
    NL_TEST_ASSERT(inSuite, s2.data() == arr);
    NL_TEST_ASSERT(inSuite, s2.size() == 3);
    NL_TEST_ASSERT(inSuite, s2.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, s2.data_equal(s2));
    NL_TEST_ASSERT(inSuite, s2.front() == 1);
    NL_TEST_ASSERT(inSuite, s2.back() == 3);
    NL_TEST_ASSERT(inSuite, s2[0] == 1);
    NL_TEST_ASSERT(inSuite, s2[1] == 2);
    NL_TEST_ASSERT(inSuite, s2[2] == 3);

    FixedByteSpan<3> s3 = s2;
    NL_TEST_ASSERT(inSuite, s3.data() == arr);
    NL_TEST_ASSERT(inSuite, s3.size() == 3);
    NL_TEST_ASSERT(inSuite, s3.data()[2] == 3);
    NL_TEST_ASSERT(inSuite, s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    FixedSpan<uint8_t, 3> s4(arr2);
    NL_TEST_ASSERT(inSuite, !s4.data_equal(s2));

    size_t idx = 0;
    for (auto & entry : s4)
    {
        NL_TEST_ASSERT(inSuite, entry == arr2[idx++]);
    }
    NL_TEST_ASSERT(inSuite, idx == 3);

    FixedByteSpan<3> s5(arr2);
    NL_TEST_ASSERT(inSuite, s5.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s5));

    FixedByteSpan<2> s6(s4);
    idx = 0;
    for (auto & entry : s6)
    {
        NL_TEST_ASSERT(inSuite, entry == arr2[idx++]);
    }
    NL_TEST_ASSERT(inSuite, idx == 2);

    // Not fixed, to test conversion.
    ByteSpan s7(s4);
    NL_TEST_ASSERT(inSuite, s7.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s7));

    MutableByteSpan s8(s4);
    NL_TEST_ASSERT(inSuite, s8.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s8));
}

static void TestSpanOfPointers(nlTestSuite * inSuite, void * inContext)
{
    uint8_t x        = 5;
    uint8_t * ptrs[] = { &x, &x };
    Span<uint8_t *> s1(ptrs);
    Span<uint8_t * const> s2(s1);
    NL_TEST_ASSERT(inSuite, s1.data_equal(s2));
    NL_TEST_ASSERT(inSuite, s2.data_equal(s1));

    FixedSpan<uint8_t *, 2> s3(ptrs);
    FixedSpan<uint8_t * const, 2> s4(s3);
    NL_TEST_ASSERT(inSuite, s1.data_equal(s3));
    NL_TEST_ASSERT(inSuite, s3.data_equal(s1));

    NL_TEST_ASSERT(inSuite, s2.data_equal(s3));
    NL_TEST_ASSERT(inSuite, s3.data_equal(s2));

    NL_TEST_ASSERT(inSuite, s1.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s1));

    NL_TEST_ASSERT(inSuite, s2.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s2));

    NL_TEST_ASSERT(inSuite, s3.data_equal(s4));
    NL_TEST_ASSERT(inSuite, s4.data_equal(s3));

    Span<uint8_t *> s5(s3);
    NL_TEST_ASSERT(inSuite, s5.data_equal(s3));
    NL_TEST_ASSERT(inSuite, s3.data_equal(s5));
}

static void TestSubSpan(nlTestSuite * inSuite, void * inContext)
{
    uint8_t array[16];
    ByteSpan span(array);

    NL_TEST_ASSERT(inSuite, span.data() == &array[0]);
    NL_TEST_ASSERT(inSuite, span.size() == 16);

    ByteSpan subspan = span.SubSpan(1, 14);
    NL_TEST_ASSERT(inSuite, subspan.data() == &array[1]);
    NL_TEST_ASSERT(inSuite, subspan.size() == 14);

    subspan = span.SubSpan(1, 0);
    NL_TEST_ASSERT(inSuite, subspan.size() == 0);

    subspan = span.SubSpan(10);
    NL_TEST_ASSERT(inSuite, subspan.data() == &array[10]);
    NL_TEST_ASSERT(inSuite, subspan.size() == 6);

    subspan = span.SubSpan(16);
    NL_TEST_ASSERT(inSuite, subspan.size() == 0);
}

static void TestFromZclString(nlTestSuite * inSuite, void * inContext)
{
    // Purposefully larger size than data.
    constexpr uint8_t array[16] = { 3, 0x41, 0x63, 0x45 };

    static constexpr char str[] = "AcE";

    ByteSpan s1 = ByteSpan::fromZclString(array);
    NL_TEST_ASSERT(inSuite, s1.data_equal(ByteSpan(&array[1], 3)));

    CharSpan s2 = CharSpan::fromZclString(array);
    NL_TEST_ASSERT(inSuite, s2.data_equal(CharSpan(str, 3)));
}

static void TestFromCharString(nlTestSuite * inSuite, void * inContext)
{
    static constexpr char str[] = "AcE";

    CharSpan s1 = CharSpan::fromCharString(str);
    NL_TEST_ASSERT(inSuite, s1.data_equal(CharSpan(str, 3)));
}

static void TestLiteral(nlTestSuite * inSuite, void * inContext)
{
    constexpr CharSpan literal = "HI!"_span;
    NL_TEST_ASSERT(inSuite, literal.size() == 3);
    NL_TEST_ASSERT(inSuite, literal.data_equal(CharSpan::fromCharString("HI!")));
    NL_TEST_ASSERT(inSuite, ""_span.size() == 0);

    // These should be compile errors -- if they were allowed they would produce
    // a CharSpan that includes the trailing '\0' byte in the value.
    // constexpr CharSpan disallowed1("abcd");
    // constexpr CharSpan disallowed2{ "abcd" };
}

static void TestConversionConstructors(nlTestSuite * inSuite, void * inContext)
{
    struct Foo
    {
        int member = 0;
    };
    struct Bar : public Foo
    {
    };

    Bar objects[2];

    // Check that various things here compile.
    Span<Foo> span1(objects);
    Span<Foo> span2(&objects[0], 1);
    FixedSpan<Foo, 2> span3(objects);
    FixedSpan<Foo, 1> span4(objects);

    Span<Bar> testSpan1(objects);
    FixedSpan<Bar, 2> testSpan2(objects);

    Span<Foo> span5(testSpan1);
    Span<Foo> span6(testSpan2);

    FixedSpan<Foo, 2> span7(testSpan2);

    std::array<Bar, 3> array;
    const auto & constArray = array;
    FixedSpan<Foo, 3> span9(array);
    FixedSpan<const Foo, 3> span10(constArray);
    Span<Foo> span11(array);
    Span<const Foo> span12(constArray);

    // Various places around the code base expect these conversions to be implicit
    ([](FixedSpan<Foo, 3> f) {})(array);
    ([](Span<Foo> f) {})(array);
    ([](FixedSpan<const Foo, 3> f) {})(constArray);
    ([](Span<const Foo> f) {})(constArray);

    NL_TEST_ASSERT(inSuite, span10.data_equal(span10));
    NL_TEST_ASSERT(inSuite, span10.data_equal(span9));
    NL_TEST_ASSERT(inSuite, span10.data_equal(array));
    NL_TEST_ASSERT(inSuite, span10.data_equal(constArray));
    NL_TEST_ASSERT(inSuite, span9.data_equal(span9));
    NL_TEST_ASSERT(inSuite, span9.data_equal(span10));
    NL_TEST_ASSERT(inSuite, span9.data_equal(array));
    NL_TEST_ASSERT(inSuite, span9.data_equal(constArray));

    // The following should not compile
    // Span<const Foo> error1 = std::array<Foo, 3>(); // Span would point into a temporary value
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestByteSpan),
    NL_TEST_DEF_FN(TestMutableByteSpan),
    NL_TEST_DEF_FN(TestFixedByteSpan),
    NL_TEST_DEF_FN(TestSpanOfPointers),
    NL_TEST_DEF_FN(TestSubSpan),
    NL_TEST_DEF_FN(TestFromZclString),
    NL_TEST_DEF_FN(TestFromCharString),
    NL_TEST_DEF_FN(TestLiteral),
    NL_TEST_DEF_FN(TestConversionConstructors),
    NL_TEST_SENTINEL(),
};

int TestSpan()
{
    nlTestSuite theSuite = { "CHIP Span tests", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSpan)
