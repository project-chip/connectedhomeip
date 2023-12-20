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

#include <lib/support/BytesCircularBuffer.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestPushInvalid(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    BytesCircularBuffer buffer(storage, sizeof(storage));
    const uint8_t s[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, static_cast<size_t>(-1))) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 8)) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 9)) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s, 7)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
}

void RandomPushPop(nlTestSuite * inSuite, void * inContext, BytesCircularBuffer & buffer,
                   std::list<std::vector<uint8_t>> & alreadyInBuffer)
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

    auto verify = [&inSuite, &alreadyInBuffer, &buffer] {
        if (alreadyInBuffer.empty())
        {
            NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
        }
        else
        {
            NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
            auto length = alreadyInBuffer.front().size();
            NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == length);
            std::vector<uint8_t> str(length);
            MutableByteSpan readSpan(str.data(), length);
            NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, readSpan.size() == length);
            NL_TEST_ASSERT(inSuite, alreadyInBuffer.front() == str);
        }
    };

    verify();

    // Run 200 random operations
    for (int i = 0; i < 200; ++i)
    {
        if (!buffer.IsEmpty() && std::rand() % 3 == 0)
        {
            // pop
            NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
            alreadyInBuffer.pop_front();

            verify();
        }
        else
        {
            // push random string
            auto str = randomString();
            NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(str.data(), str.size())) == CHIP_NO_ERROR);
            alreadyInBuffer.push_back(str);

            while (bufferSize() > 9)
            {
                alreadyInBuffer.pop_front();
            }

            verify();
        }
    }
}

void TestPushPopRandom(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    std::list<std::vector<uint8_t>> alreadyInBuffer;
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
}

void TestPushToJustFull(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Used = 9, Free = 0
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s1[0], s1[1], s1[2], s1[3] });
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0] });
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
}

void TestPushToJustFullOverOne(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Used = 4, Free = 5
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 2)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 2);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 2) == 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0], s2[1] });
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
}

void TestPushWrap(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Used = 6, Free = 3, s1 was discarded due to lack of storage
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 4) == 0);

    // Used = 9, Free = 0
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 4) == 0);

    // Used = 3, Free = 6
    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 1);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 1) == 0);

    // All freed
    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());
}

void TestPushWrapStartAtEdge(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3, mDataStart = 0, mDataEnd = 6
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Used = 4, Free = 5, mDataStart = 6, mDataEnd = 0 wrap mDataEnd
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 2)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 2);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 2) == 0);

    // Used = 7, Free = 2, mDataStart = 6, mDataEnd = 3 wrap mDataEnd
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 1)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 2);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 2) == 0);

    // Used = 3, Free = 6, mDataStart = 0, mDataEnd = 3
    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 1);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 1) == 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s1[0] });
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
}

void TestPushWrapEndAtEdge(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 6, Free = 3, mDataStart = 0, mDataEnd = 6
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 4)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 4);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 4) == 0);

    // Used = 4, Free = 5, mDataStart = 6, mDataEnd = 0 wrap mDataEnd
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 2)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 2);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 2) == 0);

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    alreadyInBuffer.push_back(std::vector<uint8_t>{ s2[0], s2[1] });
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
}

void TestPushWhenFull(nlTestSuite * inSuite, void * inContext)
{
    uint8_t storage[10];
    const uint8_t s1[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const uint8_t s2[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    uint8_t o[sizeof(s1)];
    MutableByteSpan readSpan(o);

    // Total space = 9
    BytesCircularBuffer buffer(storage, sizeof(storage));
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Used = 9, Free = 0
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s1, 7)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 7);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s1, readSpan.data(), 7) == 0);

    // Used = 5, Free = 4
    NL_TEST_ASSERT(inSuite, buffer.Push(ByteSpan(s2, 3)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !buffer.IsEmpty());
    NL_TEST_ASSERT(inSuite, buffer.GetFrontSize() == 3);
    readSpan = MutableByteSpan(o);
    NL_TEST_ASSERT(inSuite, buffer.ReadFront(readSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(s2, readSpan.data(), 3) == 0);

    // All freed
    NL_TEST_ASSERT(inSuite, buffer.Pop() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, buffer.IsEmpty());

    // Try normal push/pop again
    std::list<std::vector<uint8_t>> alreadyInBuffer;
    RandomPushPop(inSuite, inContext, buffer, alreadyInBuffer);
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
static const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestPushPopRandom),         NL_TEST_DEF_FN(TestPushInvalid),  NL_TEST_DEF_FN(TestPushToJustFull),
    NL_TEST_DEF_FN(TestPushToJustFullOverOne), NL_TEST_DEF_FN(TestPushWrap),     NL_TEST_DEF_FN(TestPushWrapStartAtEdge),
    NL_TEST_DEF_FN(TestPushWrapEndAtEdge),     NL_TEST_DEF_FN(TestPushWhenFull), NL_TEST_SENTINEL()
};

int TestBytesCircularBuffer()
{
    nlTestSuite theSuite = { "CHIP BytesCircularBuffer tests", &sTests[0], Setup, Teardown };

    unsigned seed = static_cast<unsigned>(std::time(nullptr));
    printf("Running " __FILE__ " using seed %d", seed);
    std::srand(seed);
    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBytesCircularBuffer);
