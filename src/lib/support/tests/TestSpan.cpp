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

#include <array>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

using namespace chip;

TEST(TestSpan, TestByteSpan)
{
    uint8_t arr[] = { 1, 2, 3 };

    ByteSpan s0 = ByteSpan();
    EXPECT_EQ(s0.size(), 0u);
    EXPECT_TRUE(s0.empty());
    EXPECT_TRUE(s0.data_equal(s0));

    ByteSpan s1(arr, 2);
    EXPECT_EQ(s1.data(), arr);
    EXPECT_EQ(s1.size(), 2u);
    EXPECT_FALSE(s1.empty());
    EXPECT_TRUE(s1.data_equal(s1));
    EXPECT_FALSE(s1.data_equal(s0));

    ByteSpan s2(arr);
    EXPECT_EQ(s2.data(), arr);
    EXPECT_EQ(s2.size(), 3u);
    EXPECT_EQ(s2.data()[2], 3u);
    EXPECT_FALSE(s2.empty());
    EXPECT_TRUE(s2.data_equal(s2));
    EXPECT_FALSE(s2.data_equal(s1));
    EXPECT_EQ(s2.front(), 1u);
    EXPECT_EQ(s2.back(), 3u);
    EXPECT_EQ(s2[0], 1u);
    EXPECT_EQ(s2[1], 2u);
    EXPECT_EQ(s2[2], 3u);

    ByteSpan s3 = s2;
    EXPECT_EQ(s3.data(), arr);
    EXPECT_EQ(s3.size(), 3u);
    EXPECT_EQ(s3.data()[2], 3u);
    EXPECT_FALSE(s3.empty());
    EXPECT_TRUE(s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    ByteSpan s4(arr2);
    EXPECT_FALSE(s4.data_equal(s2));

    ByteSpan s5(arr2, 0);
    EXPECT_NE(s5.data(), nullptr);
    EXPECT_FALSE(s5.data_equal(s4));
    EXPECT_TRUE(s5.data_equal(s0));
    EXPECT_TRUE(s0.data_equal(s5));

    ByteSpan s6(arr2);
    s6.reduce_size(2);
    EXPECT_EQ(s6.size(), 2u);
    ByteSpan s7(arr2, 2);
    EXPECT_TRUE(s6.data_equal(s7));
    EXPECT_TRUE(s7.data_equal(s6));
}

TEST(TestSpan, TestMutableByteSpan)
{
    uint8_t arr[] = { 1, 2, 3 };

    MutableByteSpan s0 = MutableByteSpan();
    EXPECT_EQ(s0.size(), 0u);
    EXPECT_TRUE(s0.empty());
    EXPECT_TRUE(s0.data_equal(s0));

    MutableByteSpan s1(arr, 2);
    EXPECT_EQ(s1.data(), arr);
    EXPECT_EQ(s1.size(), 2u);
    EXPECT_FALSE(s1.empty());
    EXPECT_TRUE(s1.data_equal(s1));
    EXPECT_FALSE(s1.data_equal(s0));

    MutableByteSpan s2(arr);
    EXPECT_EQ(s2.data(), arr);
    EXPECT_EQ(s2.size(), 3u);
    EXPECT_EQ(s2.data()[2], 3u);
    EXPECT_FALSE(s2.empty());
    EXPECT_TRUE(s2.data_equal(s2));
    EXPECT_FALSE(s2.data_equal(s1));

    MutableByteSpan s3 = s2;
    EXPECT_EQ(s3.data(), arr);
    EXPECT_EQ(s3.size(), 3u);
    EXPECT_EQ(s3.data()[2], 3u);
    EXPECT_FALSE(s3.empty());
    EXPECT_TRUE(s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    MutableByteSpan s4(arr2);
    EXPECT_FALSE(s4.data_equal(s2));

    MutableByteSpan s5(arr2, 0);
    EXPECT_NE(s5.data(), nullptr);
    EXPECT_FALSE(s5.data_equal(s4));
    EXPECT_TRUE(s5.data_equal(s0));
    EXPECT_TRUE(s0.data_equal(s5));

    MutableByteSpan s6(arr2);
    s6.reduce_size(2);
    EXPECT_EQ(s6.size(), 2u);
    MutableByteSpan s7(arr2, 2);
    EXPECT_TRUE(s6.data_equal(s7));
    EXPECT_TRUE(s7.data_equal(s6));

    uint8_t arr3[] = { 1, 2, 3 };
    MutableByteSpan s8(arr3);
    EXPECT_EQ(arr3[1], 2u);
    s8.data()[1] = 3;
    EXPECT_EQ(arr3[1], 3u);

    // Not mutable span on purpose, to test conversion.
    ByteSpan s9 = s8;
    EXPECT_TRUE(s9.data_equal(s8));
    EXPECT_TRUE(s8.data_equal(s9));

    // Not mutable span on purpose.
    ByteSpan s10(s8);
    EXPECT_TRUE(s10.data_equal(s8));
    EXPECT_TRUE(s8.data_equal(s10));
}

TEST(TestSpan, TestFixedByteSpan)
{
    uint8_t arr[] = { 1, 2, 3 };

    FixedByteSpan<3> s0 = FixedByteSpan<3>();
    EXPECT_NE(s0.data(), nullptr);
    EXPECT_EQ(s0.size(), 3u);
    EXPECT_TRUE(s0.data_equal(s0));
    EXPECT_EQ(s0[0], 0u);
    EXPECT_EQ(s0[1], 0u);
    EXPECT_EQ(s0[2], 0u);

    FixedByteSpan<2> s1(arr);
    EXPECT_EQ(s1.data(), arr);
    EXPECT_EQ(s1.size(), 2u);
    EXPECT_TRUE(s1.data_equal(s1));

    FixedByteSpan<3> s2(arr);
    EXPECT_EQ(s2.data(), arr);
    EXPECT_EQ(s2.size(), 3u);
    EXPECT_EQ(s2.data()[2], 3u);
    EXPECT_TRUE(s2.data_equal(s2));
    EXPECT_EQ(s2.front(), 1u);
    EXPECT_EQ(s2.back(), 3u);
    EXPECT_EQ(s2[0], 1u);
    EXPECT_EQ(s2[1], 2u);
    EXPECT_EQ(s2[2], 3u);

    FixedByteSpan<3> s3 = s2;
    EXPECT_EQ(s3.data(), arr);
    EXPECT_EQ(s3.size(), 3u);
    EXPECT_EQ(s3.data()[2], 3u);
    EXPECT_TRUE(s3.data_equal(s2));

    uint8_t arr2[] = { 3, 2, 1 };
    FixedSpan<uint8_t, 3> s4(arr2);
    EXPECT_FALSE(s4.data_equal(s2));

    size_t idx = 0;
    for (auto & entry : s4)
    {
        EXPECT_EQ(entry, arr2[idx++]);
    }
    EXPECT_EQ(idx, 3u);

    FixedByteSpan<3> s5(arr2);
    EXPECT_TRUE(s5.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s5));

    FixedByteSpan<2> s6(s4);
    idx = 0;
    for (auto & entry : s6)
    {
        EXPECT_EQ(entry, arr2[idx++]);
    }
    EXPECT_EQ(idx, 2u);

    // Not fixed, to test conversion.
    ByteSpan s7(s4);
    EXPECT_TRUE(s7.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s7));

    MutableByteSpan s8(s4);
    EXPECT_TRUE(s8.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s8));
}

TEST(TestSpan, TestSpanOfPointers)
{
    uint8_t x        = 5;
    uint8_t * ptrs[] = { &x, &x };
    Span<uint8_t *> s1(ptrs);
    Span<uint8_t * const> s2(s1);
    EXPECT_TRUE(s1.data_equal(s2));
    EXPECT_TRUE(s2.data_equal(s1));

    FixedSpan<uint8_t *, 2> s3(ptrs);
    FixedSpan<uint8_t * const, 2> s4(s3);
    EXPECT_TRUE(s1.data_equal(s3));
    EXPECT_TRUE(s3.data_equal(s1));

    EXPECT_TRUE(s2.data_equal(s3));
    EXPECT_TRUE(s3.data_equal(s2));

    EXPECT_TRUE(s1.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s1));

    EXPECT_TRUE(s2.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s2));

    EXPECT_TRUE(s3.data_equal(s4));
    EXPECT_TRUE(s4.data_equal(s3));

    Span<uint8_t *> s5(s3);
    EXPECT_TRUE(s5.data_equal(s3));
    EXPECT_TRUE(s3.data_equal(s5));
}

TEST(TestSpan, TestSubSpan)
{
    uint8_t array[16];
    ByteSpan span(array);

    EXPECT_EQ(span.data(), &array[0]);
    EXPECT_EQ(span.size(), 16u);

    ByteSpan subspan = span.SubSpan(1, 14);
    EXPECT_EQ(subspan.data(), &array[1]);
    EXPECT_EQ(subspan.size(), 14u);

    subspan = span.SubSpan(1, 0);
    EXPECT_EQ(subspan.size(), 0u);

    subspan = span.SubSpan(10);
    EXPECT_EQ(subspan.data(), &array[10]);
    EXPECT_EQ(subspan.size(), 6u);

    subspan = span.SubSpan(16);
    EXPECT_EQ(subspan.size(), 0u);
}

TEST(TestSpan, TestFromZclString)
{
    // Purposefully larger size than data.
    constexpr uint8_t array[16] = { 3, 0x41, 0x63, 0x45 };

    static constexpr char str[] = "AcE";

    ByteSpan s1 = ByteSpan::fromZclString(array);
    EXPECT_TRUE(s1.data_equal(ByteSpan(&array[1], 3u)));

    CharSpan s2 = CharSpan::fromZclString(array);
    EXPECT_TRUE(s2.data_equal(CharSpan(str, 3)));
}

TEST(TestSpan, TestFromCharString)
{
    static constexpr char str[] = "AcE";

    CharSpan s1 = CharSpan::fromCharString(str);
    EXPECT_TRUE(s1.data_equal(CharSpan(str, 3)));
}

TEST(TestSpan, TestLiteral)
{
    constexpr CharSpan literal = "HI!"_span;
    EXPECT_EQ(literal.size(), 3u);
    EXPECT_TRUE(literal.data_equal(CharSpan::fromCharString("HI!")));
    EXPECT_EQ(""_span.size(), 0u);

    // These should be compile errors -- if they were allowed they would produce
    // a CharSpan that includes the trailing '\0' byte in the value.
    // constexpr CharSpan disallowed1("abcd");
    // constexpr CharSpan disallowed2{ "abcd" };
}

TEST(TestSpan, TestConversionConstructors)
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

    EXPECT_TRUE(span10.data_equal(span10));
    EXPECT_TRUE(span10.data_equal(span9));
    EXPECT_TRUE(span10.data_equal(array));
    EXPECT_TRUE(span10.data_equal(constArray));
    EXPECT_TRUE(span9.data_equal(span9));
    EXPECT_TRUE(span9.data_equal(span10));
    EXPECT_TRUE(span9.data_equal(array));
    EXPECT_TRUE(span9.data_equal(constArray));

    // The following should not compile
    // Span<const Foo> error1 = std::array<Foo, 3>(); // Span would point into a temporary value
}
