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

#include <algorithm>
#include <ctime>
#include <list>
#include <numeric>
#include <string.h>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BytesCircularBuffer.h>

namespace {

using namespace chip;

class TestBytesCircularBuffer : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        unsigned seed = static_cast<unsigned>(std::time(nullptr));
        printf("Running " __FILE__ " using seed %d \n", seed);
        std::srand(seed);
    }
};

TEST_F(TestBytesCircularBuffer, TestPushInvalid)
{
    uint8_t storage[10];
    BytesCircularBuffer buffer(storage, sizeof(storage));
    const uint8_t s[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    EXPECT_TRUE(buffer.IsEmpty());

    EXPECT_EQ(buffer.Push(ByteSpan(s, static_cast<size_t>(-1))), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(buffer.IsEmpty());

    EXPECT_EQ(buffer.Push(ByteSpan(s, 8)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_EQ(buffer.Push(ByteSpan(s, 9)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_EQ(buffer.Push(ByteSpan(s, 7)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
}

void RandomPushPop(BytesCircularBuffer & buffer, std::list<std::vector<uint8_t>> & alreadyInBuffer)
{
    auto randomString = [] {
        std::vector<uint8_t> str(static_cast<size_t>(std::rand()) % 8);
        std::generate_n(str.begin(), str.size(), [] { return std::rand(); });
        return str;
    };

    auto bufferSize = [&alreadyInBuffer] {
        return std::accumulate(alreadyInBuffer.begin(), alreadyInBuffer.end(), 0u,
                               [](auto s, auto str) { return s + str.size() + 2; });
    };

    auto verify = [&alreadyInBuffer, &buffer] {
        if (alreadyInBuffer.empty())
        {
            EXPECT_TRUE(buffer.IsEmpty());
        }
        else
        {
            EXPECT_FALSE(buffer.IsEmpty());
            auto length = alreadyInBuffer.front().size();
            EXPECT_EQ(buffer.GetFrontSize(), length);
            std::vector<uint8_t> str(length);
            MutableByteSpan readSpan(str.data(), length);
            EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
            EXPECT_EQ(readSpan.size(), length);
            EXPECT_EQ(alreadyInBuffer.front(), str);
        }
    };

    verify();

    // Run 200 random operations
    for (int i = 0; i < 200; ++i)
    {
        if (!buffer.IsEmpty() && std::rand() % 3 == 0)
        {
            // pop
            EXPECT_EQ(buffer.Pop(), CHIP_NO_ERROR);
            alreadyInBuffer.pop_front();

            verify();
        }
        else
        {
            // push random string
            auto str = randomString();
            EXPECT_EQ(buffer.Push(ByteSpan(str.data(), str.size())), CHIP_NO_ERROR);
            alreadyInBuffer.push_back(str);

            while (bufferSize() > 9)
            {
                alreadyInBuffer.pop_front();
            }

            verify();
        }
    }
}

TEST_F(TestBytesCircularBuffer, TestPushPopRandom)
{
    uint8_t storage[10];
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    std::list<std::vector<uint8_t>> alreadyInBuffer;
    RandomPushPop(buffer, alreadyInBuffer);
}

TEST_F(TestBytesCircularBuffer, TestPushToJustFull)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 6, Free = 3
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(memcmp(s1, readSpan.data(), 4));

    // Used = 9, Free = 0
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 1)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(memcmp(s1, readSpan.data(), 4));

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s1[0], s1[1], s1[2], s1[3] });
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0] });
    RandomPushPop(buffer, alreadyInBuffer);
}

TEST_F(TestBytesCircularBuffer, TestPushToJustFullOverOne)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 6, Free = 3
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 4), 0);

    // Used = 4, Free = 5
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 2)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 2u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 2), 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0], s2[1] });
    RandomPushPop(buffer, alreadyInBuffer);
}

TEST_F(TestBytesCircularBuffer, TestPushWrap)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 6, Free = 3
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 4), 0);

    // Used = 6, Free = 3, s1 was discarded due to lack of storage
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 4), 0);

    // Used = 9, Free = 0
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 1)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 4), 0);

    // Used = 3, Free = 6
    EXPECT_EQ(buffer.Pop(), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_TRUE(buffer.GetFrontSize());
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 1), 0);

    // All freed
    EXPECT_EQ(buffer.Pop(), CHIP_NO_ERROR);
    EXPECT_TRUE(buffer.IsEmpty());
}

TEST_F(TestBytesCircularBuffer, TestPushWrapStartAtEdge)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 6, Free = 3, mDataStart = 0, mDataEnd = 6
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 4), 0);

    // Used = 4, Free = 5, mDataStart = 6, mDataEnd = 0 wrap mDataEnd
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 2)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 2u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 2), 0);

    // Used = 7, Free = 2, mDataStart = 6, mDataEnd = 3 wrap mDataEnd
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 1)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 2u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 2), 0);

    // Used = 3, Free = 6, mDataStart = 0, mDataEnd = 3
    EXPECT_EQ(buffer.Pop(), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 1u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 1), 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s1[0] });
    RandomPushPop(buffer, alreadyInBuffer);
}

TEST_F(TestBytesCircularBuffer, TestPushWrapEndAtEdge)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 6, Free = 3, mDataStart = 0, mDataEnd = 6
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 4)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 4u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 4), 0);

    // Used = 4, Free = 5, mDataStart = 6, mDataEnd = 0 wrap mDataEnd
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 2)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 2u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 2), 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0], s2[1] });
    RandomPushPop(buffer, alreadyInBuffer);
}

TEST_F(TestBytesCircularBuffer, TestPushWhenFull)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    EXPECT_TRUE(buffer.IsEmpty());

    // Used = 9, Free = 0
    EXPECT_EQ(buffer.Push(ByteSpan(s1, 7)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 7u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s1, readSpan.data(), 7), 0);

    // Used = 5, Free = 4
    EXPECT_EQ(buffer.Push(ByteSpan(s2, 3)), CHIP_NO_ERROR);
    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetFrontSize(), 3u);
    readSpan = MutableByteSpan(o);
    EXPECT_EQ(buffer.ReadFront(readSpan), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(s2, readSpan.data(), 3), 0);

    // All freed
    EXPECT_EQ(buffer.Pop(), CHIP_NO_ERROR);
    EXPECT_TRUE(buffer.IsEmpty());

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    RandomPushPop(buffer, alreadyInBuffer);
}

} // namespace
