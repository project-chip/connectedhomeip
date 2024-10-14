/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2019 Google Inc. All Rights Reserved.
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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/PrivateHeap.h>

namespace {

constexpr size_t kBlockHeaderSize = sizeof(internal::PrivateHeapBlockHeader);

// Splitting block tests assume we know the size
static_assert(kBlockHeaderSize == 16, "Test assumes block size of 16");

// helper class for allocating things
template <size_t kSize>
class PrivateHeapAllocator
{
public:
    PrivateHeapAllocator() { PrivateHeapInit(mHeap.buffer, kSize); }
    void * HeapAlloc(size_t size) { return PrivateHeapAlloc(mHeap.buffer, size); }
    void HeapFree(void * buffer) { PrivateHeapFree(buffer); }
    void * HeapRealloc(void * buffer, size_t size) { return PrivateHeapRealloc(mHeap.buffer, buffer, size); }

private:
    struct alignas(kPrivateHeapAllocationAlignment)
    {
        uint8_t buffer[kSize];
    } mHeap;
};

TEST(TestPrivateHeap, TestSingleHeapAllocAndFree)
{
    PrivateHeapAllocator<16 + 2 * kBlockHeaderSize> allocator;

    EXPECT_EQ(nullptr, allocator.HeapAlloc(17)); // insufficient size
    void * ptr = allocator.HeapAlloc(16);
    ASSERT_NE(nullptr, ptr);
    EXPECT_EQ(nullptr, allocator.HeapAlloc(1)); // insufficient size
    memset(ptr, 0xab, 16);
    allocator.HeapFree(ptr);

    // allocate different sizes on this heap, see how that goes
    for (size_t i = 1; i < 17; ++i)
    {
        ptr = allocator.HeapAlloc(i);
        ASSERT_NE(nullptr, ptr);
        EXPECT_EQ(nullptr, allocator.HeapAlloc(17 - i)); // insufficient size
        allocator.HeapFree(ptr);
    }
}

TEST(TestPrivateHeap, TestSplitHeapAllocAndFree)
{
    PrivateHeapAllocator<128> allocator;
    // allocator state:
    // <HDR-FREE> 96 <HDR-END>

    void * p1 = allocator.HeapAlloc(30);
    ASSERT_NE(nullptr, p1);
    // allocator state:
    // <HDR-IN_USE> 32 <HRD-FREE> 48 <HDR-END>

    void * p2 = allocator.HeapAlloc(4);
    ASSERT_NE(nullptr, p2);
    // allocator state:
    // <HDR-IN_USE> 32 <HRD-IN_USE> 8 <HDR-FREE> 24 <HDR-END>

    allocator.HeapFree(p1);
    // allocator state:
    // <HDR-FREE> 32 <HRD-IN_USE> 8 <HDR-FREE> 24 <HDR-END>

    allocator.HeapFree(p2);
    // allocator state:
    // <HDR-FREE> 96 <HDR-END>

    p1 = allocator.HeapAlloc(90);
    ASSERT_NE(nullptr, p1);
    allocator.HeapFree(p1);
}

TEST(TestPrivateHeap, TestFreeMergeNext)
{
    PrivateHeapAllocator<5 * 16> allocator;

    void * p1 = allocator.HeapAlloc(16);
    void * p2 = allocator.HeapAlloc(16);

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);
    EXPECT_EQ(nullptr, allocator.HeapAlloc(1));

    memset(p1, 0xab, 16);
    memset(p2, 0xcd, 16);

    // freeing 1,2 should clear space
    allocator.HeapFree(p1);
    allocator.HeapFree(p2);

    p1 = allocator.HeapAlloc(3 * 16);
    ASSERT_NE(nullptr, p1);
    allocator.HeapFree(p1);
}

TEST(TestPrivateHeap, TestFreeMergePrevious)
{
    PrivateHeapAllocator<5 * 16> allocator;

    void * p1 = allocator.HeapAlloc(16);
    void * p2 = allocator.HeapAlloc(16);

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);
    EXPECT_EQ(nullptr, allocator.HeapAlloc(1));

    memset(p1, 0xab, 16);
    memset(p2, 0xcd, 16);

    // freeing 2,1 should clear space
    allocator.HeapFree(p2);
    allocator.HeapFree(p1);
    p1 = allocator.HeapAlloc(3 * 16);
    ASSERT_NE(nullptr, p1);
    allocator.HeapFree(p1);
}

TEST(TestPrivateHeap, TestFreeMergePreviousAndNext)
{

    PrivateHeapAllocator<7 * 16> allocator;

    void * p1 = allocator.HeapAlloc(16);
    void * p2 = allocator.HeapAlloc(16);
    void * p3 = allocator.HeapAlloc(16);

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);
    ASSERT_NE(nullptr, p3);
    EXPECT_EQ(nullptr, allocator.HeapAlloc(1));

    memset(p1, 0xab, 16);
    memset(p2, 0xcd, 16);
    memset(p3, 0xef, 16);

    allocator.HeapFree(p1);
    allocator.HeapFree(p3);
    // we have 2 slots of size 16 available now
    EXPECT_EQ(nullptr, allocator.HeapAlloc(17));

    // Freeing p2 makes enoug space
    allocator.HeapFree(p2);
    p1 = allocator.HeapAlloc(5 * 16);
    ASSERT_NE(nullptr, p1);
    allocator.HeapFree(p1);
}

TEST(TestPrivateHeap, TestMultipleMerge)
{
    PrivateHeapAllocator<32 * kBlockHeaderSize> allocator;

    // 31 blocks available for alloc
    void * p1 = allocator.HeapAlloc(2 * kBlockHeaderSize); // uses up 3 blocks
    void * p2 = allocator.HeapAlloc(5 * kBlockHeaderSize); // uses up 6 blocks
    void * p3 = allocator.HeapAlloc(8 * kBlockHeaderSize); // uses up 9 blocks
    void * p4 = allocator.HeapAlloc(1 * kBlockHeaderSize); // uses up 2 blocks
    void * p5 = allocator.HeapAlloc(7 * kBlockHeaderSize); // uses up 8 blocks
    void * p6 = allocator.HeapAlloc(2 * kBlockHeaderSize); // uses up 2 (last given)

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);
    ASSERT_NE(nullptr, p3);
    ASSERT_NE(nullptr, p4);
    ASSERT_NE(nullptr, p5);
    ASSERT_NE(nullptr, p6);

    allocator.HeapFree(p3);
    allocator.HeapFree(p4);
    // 10 blocks available (9 from p3 without HDR and 2 from p4 + HDR)
    p3 = allocator.HeapAlloc(10 * kBlockHeaderSize);
    ASSERT_NE(nullptr, p3);
    EXPECT_EQ(nullptr, allocator.HeapAlloc(1)); // full

    allocator.HeapFree(p6);
    allocator.HeapFree(p5);
    allocator.HeapFree(p3);
    allocator.HeapFree(p2);
    allocator.HeapFree(p1);

    p1 = allocator.HeapAlloc(30 * kBlockHeaderSize);
    ASSERT_NE(nullptr, p1);
    allocator.HeapFree(p1);
}

TEST(TestPrivateHeap, TestForwardFreeAndRealloc)
{
    constexpr int kNumBlocks = 16;
    PrivateHeapAllocator<(2 * kNumBlocks + 1) * kBlockHeaderSize> allocator;
    void * ptrs[kNumBlocks];

    for (auto & ptr : ptrs)
    {
        ptr = allocator.HeapAlloc(kBlockHeaderSize);
        ASSERT_NE(nullptr, ptr);
        memset(ptr, 0xab, kBlockHeaderSize);
    }

    // heap looks like:
    ///  |HDR| 16 |HDR| 16 |HDR| ..... |HDR| 16 |HDR|

    // free each block from the start and re-allocate into a bigger block
    for (size_t i = 1; i < kNumBlocks; ++i)
    {
        allocator.HeapFree(ptrs[0]);
        allocator.HeapFree(ptrs[i]);

        ptrs[0] = allocator.HeapAlloc((1 + 2 * i) * kBlockHeaderSize);
        ASSERT_NE(nullptr, ptrs[0]);
    }
    allocator.HeapFree(ptrs[0]);
}

TEST(TestPrivateHeap, TestBackwardFreeAndRealloc)
{
    constexpr int kNumBlocks = 16;
    PrivateHeapAllocator<(2 * kNumBlocks + 1) * kBlockHeaderSize> allocator;
    void * ptrs[kNumBlocks];

    for (auto & ptr : ptrs)
    {
        ptr = allocator.HeapAlloc(kBlockHeaderSize);
        ASSERT_NE(nullptr, ptr);
        memset(ptr, 0xab, kBlockHeaderSize);
    }

    // heap looks like:
    ///  |HDR| 16 |HDR| 16 |HDR| ..... |HDR| 16 |HDR|

    // free each block from the send and re-allocate into a bigger block
    for (size_t i = 1; i < kNumBlocks; ++i)
    {
        allocator.HeapFree(ptrs[kNumBlocks - 1]);
        allocator.HeapFree(ptrs[kNumBlocks - i - 1]);

        ptrs[kNumBlocks - 1] = allocator.HeapAlloc((1 + 2 * i) * kBlockHeaderSize);
        ASSERT_NE(nullptr, ptrs[kNumBlocks - 1]);
    }
    allocator.HeapFree(ptrs[kNumBlocks - 1]);
}

// Fills the data with a known pattern
void FillKnownPattern(void * buffer, size_t size, uint8_t start)
{
    uint8_t * p = static_cast<uint8_t *>(buffer);
    size_t cnt  = start;
    while (cnt++ < size)
    {
        uint8_t value = static_cast<uint8_t>(cnt * 31 + 7);
        *p            = value;
    }
}

// checks if the specified buffer has the given pattern in it
bool IsKnownPattern(void * buffer, size_t size, uint8_t start)
{
    uint8_t * p = static_cast<uint8_t *>(buffer);
    size_t cnt  = start;
    while (cnt++ < size)
    {
        uint8_t value = static_cast<uint8_t>(cnt * 31 + 7);
        if (*p != value)
        {
            return false;
        }
    }
    return true;
}

TEST(TestPrivateHeap, TestRealloc)
{
    PrivateHeapAllocator<6 * 16> allocator;

    void * p1 = allocator.HeapRealloc(nullptr, 16); // malloc basically
    ASSERT_NE(p1, nullptr);

    FillKnownPattern(p1, 16, 11);

    void * p2 = allocator.HeapRealloc(p1, 8); // resize, should fit
    EXPECT_EQ(p1, p2);
    EXPECT_TRUE(IsKnownPattern(p1, 8, 11));

    p2 = allocator.HeapRealloc(p1, 16); // resize, should fit
    EXPECT_EQ(p1, p2);
    EXPECT_TRUE(IsKnownPattern(p2, 8, 11)); // only 8 bytes are guaranteed

    FillKnownPattern(p1, 16, 33);
    p2 = allocator.HeapRealloc(p1, 32); // resize, does not fit. This frees p1
    ASSERT_NE(p2, nullptr);
    EXPECT_NE(p2, p1); // new reallocation occurred
    EXPECT_TRUE(IsKnownPattern(p2, 16, 33));

    void * p3 = allocator.HeapAlloc(48); // insufficient heap for this
    EXPECT_EQ(p3, nullptr);

    p1 = allocator.HeapRealloc(p2, 16); // reallocation does not change block size
    EXPECT_EQ(p1, p2);

    p3 = allocator.HeapAlloc(48); // still insufficient heap for this
    EXPECT_EQ(p3, nullptr);

    p2 = allocator.HeapRealloc(p1, 48); // insufficient heap, p1 is NOT freed
    EXPECT_EQ(p2, nullptr);

    p2 = allocator.HeapRealloc(p1, 48); // Repeat the test to ensure p1 is not freed
    EXPECT_EQ(p2, nullptr);

    allocator.HeapFree(p1);

    p3 = allocator.HeapAlloc(48); // above free should have made sufficient space
    ASSERT_NE(p3, nullptr);
    allocator.HeapFree(p3);
}

} // namespace
