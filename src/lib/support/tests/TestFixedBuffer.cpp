/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <array>
#include <type_traits>

#include <pw_unit_test/framework.h>

#include <lib/support/FixedBuffer.h>
#include <lib/support/Span.h>

using namespace chip;

namespace {

template <typename FixedBufferType>
static void ExpectEqBytes(const FixedBufferType & inFixedBufferType,
                          const std::initializer_list<typename FixedBufferType::value_type> & inExpected)
{
    ASSERT_EQ(static_cast<std::size_t>(inFixedBufferType.size()), inExpected.size());
    std::size_t i = 0;
    for (auto v : inExpected)
    {
        EXPECT_EQ(inFixedBufferType[i], v);
        i++;
    }
}

} // namespace

TEST(TestFixedBuffer, TestFixedBufferConstruction)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;

    // Default construction
    {
        FixedBufferType b;
        EXPECT_EQ(b.size(), 0u);
        EXPECT_TRUE(b.empty());
        EXPECT_FALSE(b.full());
        EXPECT_EQ(FixedBufferType::capacity(), 8u);
        EXPECT_NE(b.data(), nullptr);
        EXPECT_EQ(b.begin(), b.end());
    }

    // initializer_list construction (within capacity)
    {
        FixedBufferType b{ 1, 2, 3 };
        EXPECT_EQ(b.size(), 3u);
        EXPECT_FALSE(b.empty());
        EXPECT_FALSE(b.full());
        ExpectEqBytes(b, { 1, 2, 3 });
    }

    // pointer+size construction
    {
        const uint8_t src[] = { 9, 8, 7, 6 };
        FixedBufferType b(src, sizeof(src));
        EXPECT_EQ(b.size(), 4u);
        ExpectEqBytes(b, { 9, 8, 7, 6 });
    }

    // pointer+size with nullptr + 0 is OK
    {
        FixedBufferType b(nullptr, 0);
        EXPECT_EQ(b.size(), 0u);
        EXPECT_TRUE(b.empty());
    }

    // Copy construction
    {
        FixedBufferType a{ 4, 5 };
        FixedBufferType b(a);
        EXPECT_EQ(b.size(), 2u);
        ExpectEqBytes(b, { 4, 5 });

        // Ensure deep copy (mutating b does not change a)
        b[0] = 99;
        EXPECT_EQ(a[0], 4u);
        EXPECT_EQ(b[0], 99u);
    }
}

TEST(TestFixedBuffer, TestFixedByteBufferEqualityAgainstByteSpan)
{
    using FixedByteBufferType = FixedByteBuffer<8, uint8_t>;

    // Empty buffer equals empty span
    {
        FixedByteBufferType b;
        ByteSpan s;
        EXPECT_TRUE(b == s);
        EXPECT_FALSE(b != s);
    }

    // Equal content
    {
        const uint8_t src[] = { 1, 2, 3 };
        FixedByteBufferType b(ByteSpan(src, sizeof(src)));
        EXPECT_TRUE(b == ByteSpan(src, sizeof(src)));
        EXPECT_FALSE(b != ByteSpan(src, sizeof(src)));
    }

    // Size mismatch
    {
        const uint8_t a[] = { 1, 2, 3 };
        const uint8_t c[] = { 1, 2, 3, 4 };
        FixedByteBufferType b(ByteSpan(a, sizeof(a)));
        EXPECT_FALSE(b == ByteSpan(c, sizeof(c)));
        EXPECT_TRUE(b != ByteSpan(c, sizeof(c)));
    }

    // Content mismatch same size
    {
        const uint8_t a[] = { 1, 2, 3 };
        const uint8_t d[] = { 1, 2, 9 };
        FixedByteBufferType b(ByteSpan(a, sizeof(a)));
        EXPECT_FALSE(b == ByteSpan(d, sizeof(d)));
        EXPECT_TRUE(b != ByteSpan(d, sizeof(d)));
    }

    // Capacity bytes beyond size do not affect equality
    {
        const uint8_t used[] = { 9, 8, 7 };
        FixedByteBufferType b;
        ASSERT_TRUE(b.assign(ByteSpan(used, sizeof(used))));

        b.fill(0xAA);
        ASSERT_TRUE(b.assign(ByteSpan(used, sizeof(used))));

        EXPECT_TRUE(b == ByteSpan(used, sizeof(used)));
        EXPECT_FALSE(b != ByteSpan(used, sizeof(used)));
    }
}

TEST(TestFixedBuffer, TestFixedCharBufferEqualityAgainstCharSpanAndSpanViews)
{
    using FixedCharBufferType = FixedCharBuffer<8, uint8_t>;

    FixedCharBufferType b;
    const char src[] = { 'a', 'b', 'c' };
    ASSERT_TRUE(b.assign(src, sizeof(src)));

    // span type behavior falls out naturally
    static_assert(std::is_same<decltype(b.span()), MutableCharSpan>::value, "non-const span() should be MutableCharSpan");
    const FixedCharBufferType & cb = b;
    static_assert(std::is_same<decltype(cb.span()), CharSpan>::value, "const span() should be CharSpan");

    // Equality against CharSpan via FixedBuffer::operator==(Span<const T>)
    const CharSpan cs = cb.span();
    EXPECT_TRUE(cb == cs);
    EXPECT_FALSE(cb != cs);

    // Mismatch
    const char other[] = { 'a', 'b', 'x' };
    EXPECT_FALSE(cb == CharSpan(other, sizeof(other)));
    EXPECT_TRUE(cb != CharSpan(other, sizeof(other)));
}

TEST(TestFixedBuffer, TestFixedBufferAssignment)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;

    // initializer_list assignment
    {
        FixedBufferType b;
        b = { 10, 11, 12 };
        EXPECT_EQ(b.size(), 3u);
        ExpectEqBytes(b, { 10, 11, 12 });
    }

    // copy assignment
    {
        FixedBufferType a{ 1, 2, 3, 4 };
        FixedBufferType b{ 9 };
        b = a;
        EXPECT_EQ(b.size(), 4u);
        ExpectEqBytes(b, { 1, 2, 3, 4 });
    }

    // Assignment from a subrange of itself should be overlap-safe (memmove hardening)
    {
        FixedBufferType b{ 0, 1, 2, 3, 4, 5 };
        // assign first 4 bytes from offset 2 -> {2,3,4,5}
        ASSERT_TRUE(b.assign(b.data() + 2, 4));
        EXPECT_EQ(b.size(), 4u);
        ExpectEqBytes(b, { 2, 3, 4, 5 });
    }
}

TEST(TestFixedBuffer, TestFixedByteBufferSpanTypesAndConstruction)
{
    using FixedByteBufferType = FixedByteBuffer<8, uint8_t>;

    const uint8_t src[] = { 9, 8, 7 };
    const ByteSpan in(src, sizeof(src));

    // Construct from ByteSpan (falls out of FixedBuffer<const_span_type> constructor)
    FixedByteBufferType b(in);
    EXPECT_EQ(b.size(), 3u);
    ExpectEqBytes(b, { 9, 8, 7 });

    // Non-const span() is MutableByteSpan, const span() is ByteSpan.
    static_assert(std::is_same<decltype(b.span()), MutableByteSpan>::value, "non-const span() should be MutableByteSpan");
    const FixedByteBufferType & cb = b;
    static_assert(std::is_same<decltype(cb.span()), ByteSpan>::value, "const span() should be ByteSpan");

    // Verify span views reflect logical size and alias the same backing storage.
    {
        ByteSpan s = cb.span();
        EXPECT_EQ(s.size(), 3u);
        EXPECT_EQ(s.data(), b.data());
        EXPECT_EQ(s.data()[0], 9u);
        EXPECT_EQ(s.data()[1], 8u);
        EXPECT_EQ(s.data()[2], 7u);
    }
    {
        MutableByteSpan ms = b.span();
        EXPECT_EQ(ms.size(), 3u);
        ms.data()[1] = 99;
        EXPECT_EQ(b[1], 99u);
    }
}

TEST(TestFixedBuffer, TestFixedBufferEquality)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;

    // Empty buffers compare equal
    {
        FixedBufferType a;
        FixedBufferType b;
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a != b);
    }

    // Same content and size => equal
    {
        FixedBufferType a{ 1, 2, 3 };
        FixedBufferType b{ 1, 2, 3 };
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a != b);
    }

    // Different size (even if prefix matches) => not equal
    {
        FixedBufferType a{ 1, 2, 3 };
        FixedBufferType b{ 1, 2, 3, 4 };
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);
    }

    // Same size, different content => not equal
    {
        FixedBufferType a{ 1, 2, 3 };
        FixedBufferType b{ 1, 2, 9 };
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);
    }

    // Self equality
    {
        FixedBufferType a{ 7, 8 };
        EXPECT_TRUE(a == a);
        EXPECT_FALSE(a != a);
    }

    // Bytes beyond size must not affect equality:
    // Make two buffers with same used bytes, then perturb capacity bytes via fill().
    {
        const uint8_t src[] = { 9, 9, 9 };
        FixedBufferType a;
        FixedBufferType b;
        ASSERT_TRUE(a.assign(src, sizeof(src)));
        ASSERT_TRUE(b.assign(src, sizeof(src)));

        // Change entire capacity backing store in 'a' but keep logical used bytes same by re-assign.
        a.fill(0xAA);
        ASSERT_TRUE(a.assign(src, sizeof(src))); // restores used portion to match

        // Change entire capacity backing store in 'b' differently but keep used bytes same.
        b.fill(0x55);
        ASSERT_TRUE(b.assign(src, sizeof(src))); // restores used portion to match

        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a != b);
    }
}

TEST(TestFixedBuffer, TestFixedBufferIntrospection)
{
    using FixedBufferType = FixedBuffer<uint8_t, 4, uint8_t>;

    FixedBufferType b;
    EXPECT_TRUE(b.empty());
    EXPECT_FALSE(b.full());

    // Default constructed: all capacity available.

    FixedBufferType c;
    EXPECT_EQ(c.size(), 0u);
    EXPECT_EQ(c.available(), FixedBufferType::capacity());
    EXPECT_EQ(c.available(), 4u);

    // After assign: available decreases; at full, available is 0.

    FixedBufferType d{ 1, 2, 3 };
    EXPECT_EQ(d.size(), 3u);
    EXPECT_EQ(d.available(), 1u);

    const uint8_t one[] = { 9 };
    ASSERT_TRUE(d.append(one, sizeof(one)));
    EXPECT_TRUE(d.full());
    EXPECT_EQ(d.available(), 0u);

    // Fill to capacity
    const uint8_t src[] = { 1, 2, 3, 4 };
    ASSERT_TRUE(b.assign(src, sizeof(src)));
    EXPECT_FALSE(b.empty());
    EXPECT_TRUE(b.full());
    EXPECT_EQ(b.size(), 4u);
    EXPECT_EQ(FixedBufferType::capacity(), 4u);

    // clear drops size only
    b.clear();
    EXPECT_TRUE(b.empty());
    EXPECT_FALSE(b.full());
    EXPECT_EQ(b.size(), 0u);
}

TEST(TestFixedBuffer, TestFixedBufferObservation)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;

    FixedBufferType b{ 7, 8, 9 };

    // operator[] and data() consistency
    EXPECT_EQ(b[0], 7u);
    EXPECT_EQ(b[1], 8u);
    EXPECT_EQ(b[2], 9u);
    EXPECT_EQ(b.data()[0], 7u);
    EXPECT_EQ(b.data()[1], 8u);
    EXPECT_EQ(b.data()[2], 9u);

    // const view
    const FixedBufferType & cb = b;
    EXPECT_EQ(cb[0], 7u);
    EXPECT_NE(cb.data(), nullptr);

    // at_ptr checks against used size
    EXPECT_NE(b.at_ptr(0), nullptr);
    EXPECT_NE(b.at_ptr(2), nullptr);
    EXPECT_EQ(b.at_ptr(3), nullptr);
    EXPECT_EQ(cb.at_ptr(3), nullptr);
}

TEST(TestFixedBuffer, TestFixedBufferMutation)
{
    using FixedBufferType = FixedBuffer<uint8_t, 6, uint8_t>;

    FixedBufferType b;

    // assign negative: nullptr + nonzero
    {
        EXPECT_FALSE(b.assign(nullptr, 1));
        EXPECT_EQ(b.size(), 0u);
    }

    // assign negative: too large
    {
        const uint8_t big[] = { 1, 2, 3, 4, 5, 6, 7 };
        EXPECT_FALSE(b.assign(big, sizeof(big)));
        EXPECT_EQ(b.size(), 0u);
    }

    // assign positive
    {
        const uint8_t src[] = { 1, 2, 3 };
        EXPECT_TRUE(b.assign(src, sizeof(src)));
        EXPECT_EQ(b.size(), 3u);
        ExpectEqBytes(b, { 1, 2, 3 });
    }

    // append negative: nullptr + nonzero
    {
        EXPECT_FALSE(b.append(nullptr, 1));
        EXPECT_EQ(b.size(), 3u);
    }

    // append negative: overflow capacity
    {
        const uint8_t src[] = { 4, 5, 6, 7 };
        // current size=3, capacity=6, appending 4 would overflow
        EXPECT_FALSE(b.append(src, sizeof(src)));
        EXPECT_EQ(b.size(), 3u);
        ExpectEqBytes(b, { 1, 2, 3 });
    }

    // append positive: fits exactly
    {
        const uint8_t src[] = { 4, 5, 6 };
        EXPECT_TRUE(b.append(src, sizeof(src)));
        EXPECT_EQ(b.size(), 6u);
        ExpectEqBytes(b, { 1, 2, 3, 4, 5, 6 });
        EXPECT_TRUE(b.full());
    }

    // Overlap-safe append (self-append from within used range)
    {
        // Start fresh with {10,11,12,13}
        FixedBufferType c;
        const uint8_t src[] = { 10, 11, 12, 13 };
        ASSERT_TRUE(c.assign(src, sizeof(src)));
        // Append last 2 bytes (12,13) -> {10,11,12,13,12,13}
        ASSERT_TRUE(c.append(c.data() + 2, 2));
        EXPECT_EQ(c.size(), 6u);
        ExpectEqBytes(c, { 10, 11, 12, 13, 12, 13 });
    }

    // fill fills capacity and does change size
    {
        FixedBufferType d;
        const uint8_t src[] = { 1, 2 };
        ASSERT_TRUE(d.assign(src, sizeof(src)));
        EXPECT_EQ(d.size(), 2u);

        d.fill(0xAA);
        EXPECT_EQ(d.size(), d.capacity());

        // Used portion now also 0xAA (since capacity fill overwrote all)
        EXPECT_EQ(d[0], 0xAA);
        EXPECT_EQ(d[1], 0xAA);
    }

    // reset scrubs capacity and clears size
    {
        FixedBufferType e{ 1, 2, 3 };
        e.reset(0x00);
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(e.size(), 0u);
        // Can't directly observe scrubbed bytes via public API without peeking at data(),
        // but data() is public; at least check the first few bytes.
        EXPECT_EQ(e.data()[0], 0x00);
        EXPECT_EQ(e.data()[1], 0x00);
        EXPECT_EQ(e.data()[2], 0x00);
    }

    // resize negative/positive
    {
        FixedBufferType f{ 1, 2, 3 };
        EXPECT_FALSE(f.resize(7)); // > capacity
        EXPECT_EQ(f.size(), 3u);

        EXPECT_TRUE(f.resize(6)); // == capacity ok
        EXPECT_EQ(f.size(), 6u);
        EXPECT_TRUE(f.full());

        EXPECT_TRUE(f.resize(0));
        EXPECT_TRUE(f.empty());
    }
}

TEST(TestFixedBuffer, TestFixedBufferIteration)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;
    FixedBufferType b{ 1, 2, 3, 4 };

    // forward iteration
    {
        uint8_t sum = 0;
        for (auto it = b.begin(); it != b.end(); ++it)
        {
            sum = static_cast<uint8_t>(sum + *it);
        }
        EXPECT_EQ(sum, static_cast<uint8_t>(1 + 2 + 3 + 4));
    }

    // range-for works due to begin/end
    {
        uint8_t x = 0;
        for (auto v : b)
        {
            x ^= v;
        }
        EXPECT_EQ(x, static_cast<uint8_t>(1 ^ 2 ^ 3 ^ 4));
    }

    // const iteration
    {
        const FixedBufferType & cb = b;
        std::size_t count          = 0;
        for (auto it = cb.cbegin(); it != cb.cend(); ++it)
        {
            count++;
        }
        EXPECT_EQ(count, 4u);
    }

    // reverse iteration
    {
        std::array<uint8_t, 4> got{};
        std::size_t i = 0;
        for (auto it = b.rbegin(); it != b.rend(); ++it)
        {
            got[i++] = *it;
        }
        EXPECT_EQ(i, 4u);
        EXPECT_EQ(got[0], 4u);
        EXPECT_EQ(got[1], 3u);
        EXPECT_EQ(got[2], 2u);
        EXPECT_EQ(got[3], 1u);
    }

    // const reverse iteration
    {
        const FixedBufferType & cb = b;
        std::array<uint8_t, 4> got{};
        std::size_t i = 0;
        for (auto it = cb.rbegin(); it != cb.rend(); ++it)
        {
            got[i++] = *it;
        }
        EXPECT_EQ(i, 4u);
        EXPECT_EQ(got[0], 4u);
        EXPECT_EQ(got[1], 3u);
        EXPECT_EQ(got[2], 2u);
        EXPECT_EQ(got[3], 1u);
    }

    // empty iteration: begin == end
    {
        FixedBufferType e;
        EXPECT_EQ(e.begin(), e.end());
        EXPECT_EQ(e.rbegin(), e.rend());
    }
}

TEST(FixedByteBuffer, FixedByteBufferConstruction)
{
    using FixedBufferType = FixedByteBuffer<8, uint8_t>;

    // Construct from ByteSpan
    {
        const uint8_t src[] = { 9, 8, 7 };
        ByteSpan s(src, sizeof(src));
        FixedBufferType e(s);
        EXPECT_EQ(e.size(), 3u);
        ExpectEqBytes(e, { 9, 8, 7 });
    }
}

TEST(FixedByteBuffer, FixedByteBufferEquality)
{
    using FixedBufferType = FixedByteBuffer<8, uint8_t>;

    // Empty vs empty span
    {
        FixedBufferType e;
        ByteSpan s;
        EXPECT_TRUE(e == s);
        EXPECT_FALSE(e != s);
    }

    // Equal content
    {
        const uint8_t src[] = { 1, 2, 3 };
        FixedBufferType e(ByteSpan(src, sizeof(src)));
        EXPECT_TRUE(e == ByteSpan(src, sizeof(src)));
        EXPECT_FALSE(e != ByteSpan(src, sizeof(src)));
    }

    // Size mismatch => not equal
    {
        const uint8_t a[] = { 1, 2, 3 };
        const uint8_t b[] = { 1, 2, 3, 4 };
        FixedBufferType e(ByteSpan(a, sizeof(a)));
        EXPECT_FALSE(e == ByteSpan(b, sizeof(b)));
        EXPECT_TRUE(e != ByteSpan(b, sizeof(b)));
    }

    // Content mismatch same size => not equal
    {
        const uint8_t a[] = { 1, 2, 3 };
        const uint8_t b[] = { 1, 2, 9 };
        FixedBufferType e(ByteSpan(a, sizeof(a)));
        EXPECT_FALSE(e == ByteSpan(b, sizeof(b)));
        EXPECT_TRUE(e != ByteSpan(b, sizeof(b)));
    }

    // Equality should ignore capacity bytes beyond size.
    {
        const uint8_t used[] = { 9, 8, 7 };
        FixedBufferType e;
        ASSERT_TRUE(e.assign(ByteSpan(used, sizeof(used))));

        // Smash capacity bytes; then restore used bytes.
        e.fill(0xAA);
        ASSERT_TRUE(e.assign(ByteSpan(used, sizeof(used))));

        EXPECT_TRUE(e == ByteSpan(used, sizeof(used)));
        EXPECT_FALSE(e != ByteSpan(used, sizeof(used)));
    }
}

TEST(FixedByteBuffer, FixedByteBufferAssignment)
{
    using FixedBufferType = FixedByteBuffer<8, uint8_t>;

    FixedBufferType e;
    const uint8_t src[] = { 1, 2, 3, 4 };
    ByteSpan s(src, sizeof(src));

    e = s;
    EXPECT_EQ(e.size(), 4u);
    ExpectEqBytes(e, { 1, 2, 3, 4 });

    // Overlap-safe assign via span into same backing buffer
    {
        // e currently has {1,2,3,4}; assign from subrange {3,4}
        ByteSpan sub(e.data() + 2, 2);
        ASSERT_TRUE(e.assign(sub));
        EXPECT_EQ(e.size(), 2u);
        ExpectEqBytes(e, { 3, 4 });
    }
}

TEST(FixedByteBuffer, TestFixedBufferExtensionObservation)
{
    using FixedBufferType = FixedByteBuffer<8, uint8_t>;

    const uint8_t src[] = { 5, 6, 7 };
    FixedBufferType e(ByteSpan(src, sizeof(src)));

    // const span view
    {
        ByteSpan s = e.span();
        EXPECT_EQ(s.size(), 3u);
        EXPECT_EQ(s.data()[0], 5u);
        EXPECT_EQ(s.data()[1], 6u);
        EXPECT_EQ(s.data()[2], 7u);
    }

    // mutable span view
    {
        MutableByteSpan ms = e.span();
        EXPECT_EQ(ms.size(), 3u);
        ms.data()[1] = 99;
        EXPECT_EQ(e[1], 99u);
    }
}

TEST(TestFixedBufferExtension, TestFixedBufferExtensionMutation)
{
    using FixedBufferType = FixedByteBuffer<6, uint8_t>;

    FixedBufferType e;

    // assign negative: too large
    {
        const uint8_t big[] = { 1, 2, 3, 4, 5, 6, 7 };
        EXPECT_FALSE(e.assign(ByteSpan(big, sizeof(big))));
        EXPECT_EQ(e.size(), 0u);
    }

    // assign positive then append positive
    {
        const uint8_t a[] = { 1, 2, 3 };
        ASSERT_TRUE(e.assign(ByteSpan(a, sizeof(a))));
        EXPECT_EQ(e.size(), 3u);
        ExpectEqBytes(e, { 1, 2, 3 });

        const uint8_t b[] = { 4, 5, 6 };
        ASSERT_TRUE(e.append(ByteSpan(b, sizeof(b))));
        EXPECT_EQ(e.size(), 6u);
        ExpectEqBytes(e, { 1, 2, 3, 4, 5, 6 });
    }

    // append negative: overflow
    {
        const uint8_t c[] = { 7 };
        EXPECT_FALSE(e.append(ByteSpan(c, sizeof(c))));
        EXPECT_EQ(e.size(), 6u);
    }

    // overlap-safe append from self via span
    {
        FixedBufferType x;
        const uint8_t d[] = { 10, 11, 12, 13 };
        ASSERT_TRUE(x.assign(ByteSpan(d, sizeof(d))));
        // Append last 2 bytes: {12,13} -> {10,11,12,13,12,13}
        ByteSpan tail(x.data() + 2, 2);
        ASSERT_TRUE(x.append(tail));
        EXPECT_EQ(x.size(), 6u);
        ExpectEqBytes(x, { 10, 11, 12, 13, 12, 13 });
    }
}

TEST(TestFixedBuffer, TestFixedBufferSpanAssignAndAppend)
{
    using FixedBufferType = FixedBuffer<uint8_t, 8, uint8_t>;

    FixedBufferType b;
    const uint8_t a[] = { 1, 2, 3 };
    const uint8_t c[] = { 4, 5 };

    ASSERT_TRUE(b.assign(ByteSpan(a, sizeof(a))));
    EXPECT_EQ(b.size(), 3u);
    ExpectEqBytes(b, { 1, 2, 3 });

    ASSERT_TRUE(b.append(ByteSpan(c, sizeof(c))));
    EXPECT_EQ(b.size(), 5u);
    ExpectEqBytes(b, { 1, 2, 3, 4, 5 });
}
