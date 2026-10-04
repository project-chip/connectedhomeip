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

#include <pw_unit_test/framework.h>

#include <cstdint>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ScopedMemoryBuffer.h>

namespace {

class TestScopedMemoryBuffer : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

class TestCounterMemoryManagement
{
public:
    static int Counter() { return mAllocCount; }

    static void MemoryFree(void * p)
    {
        mAllocCount--;
        chip::Platform::MemoryFree(p);
    }
    template <typename T>
    static void * MemoryAlloc(size_t elementCount)
    {
        mAllocCount++;
        return chip::Platform::MemoryAllocTyped<T>(elementCount);
    }
    template <typename T>
    static void * MemoryCalloc(size_t elementCount)
    {
        mAllocCount++;
        return chip::Platform::MemoryCallocTyped<T>(elementCount);
    }

private:
    static int mAllocCount;
};
int TestCounterMemoryManagement::mAllocCount = 0;

class ByteCountMemoryManagement
{
public:
    static void MemoryFree(void * p);
    static void * MemoryAlloc(size_t size);
    static void * MemoryCalloc(size_t num, size_t size);
};

static_assert(chip::Platform::Impl::HasElementCountAllocators<chip::Platform::Impl::PlatformMemoryManagement, uint32_t>::value);
static_assert(chip::Platform::Impl::HasElementCountAllocators<TestCounterMemoryManagement, uint32_t>::value);
static_assert(!chip::Platform::Impl::HasElementCountAllocators<ByteCountMemoryManagement, uint32_t>::value);

using TestCounterScopedBuffer = chip::Platform::ScopedMemoryBuffer<char, TestCounterMemoryManagement>;

TEST_F(TestScopedMemoryBuffer, TestAutoFree)
{
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);

    {
        TestCounterScopedBuffer buffer;

        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
        EXPECT_TRUE(buffer.Alloc(128));
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);
    }
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
}

TEST_F(TestScopedMemoryBuffer, TestFreeDuringAllocs)
{
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);

    {
        TestCounterScopedBuffer buffer;

        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
        EXPECT_TRUE(buffer.Alloc(128));
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);
        EXPECT_TRUE(buffer.Alloc(64));
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);
        EXPECT_TRUE(buffer.Calloc(10));
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);
    }
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
}

TEST_F(TestScopedMemoryBuffer, TestRelease)
{
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
    void * ptr = nullptr;

    {
        TestCounterScopedBuffer buffer;

        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
        EXPECT_TRUE(buffer.Alloc(128));
        EXPECT_NE(buffer.Get(), nullptr);
        EXPECT_FALSE(buffer.IsNull());

        ptr = buffer.Release();
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(buffer.Get(), nullptr);
        EXPECT_TRUE(buffer.IsNull());
    }

    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);

    {
        TestCounterScopedBuffer buffer;
        EXPECT_TRUE(buffer.Alloc(128));
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 2);
        TestCounterMemoryManagement::MemoryFree(ptr);
        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 1);
    }

    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
}

TEST_F(TestScopedMemoryBuffer, TestCopyFromSpanMemcpyByteCountUsesSizeof)
{
    const uint32_t source[] = { 0x1234, 0x5678, 0x9ABC, 0xDEF0 };
    chip::Span<const uint32_t> sourceSpan(source, 4);

    chip::Platform::ScopedMemoryBufferWithSize<uint32_t> buffer;
    buffer.CopyFromSpan(sourceSpan);

    EXPECT_EQ(buffer.AllocatedSize(), static_cast<size_t>(4));
    EXPECT_NE(buffer.Get(), nullptr);

    // Verify each element
    for (size_t i = 0; i < 4; i++)
    {
        EXPECT_EQ(buffer.Get()[i], source[i]);
    }
}

TEST_F(TestScopedMemoryBuffer, TestAllocSizeOverflow)
{
    constexpr size_t kMaxCount = SIZE_MAX / sizeof(uint32_t);

    chip::Platform::ScopedMemoryBuffer<uint32_t> buffer;
    EXPECT_FALSE(buffer.Alloc(kMaxCount + 1));
    EXPECT_FALSE(buffer.Alloc(kMaxCount + 2));
    EXPECT_TRUE(buffer.Alloc(4));

    chip::Platform::ScopedMemoryBufferWithSize<uint32_t> sizedBuffer;
    EXPECT_FALSE(sizedBuffer.Alloc(kMaxCount + 2));
    EXPECT_EQ(sizedBuffer.AllocatedSize(), static_cast<size_t>(0));
    EXPECT_TRUE(sizedBuffer.Alloc(4));
    EXPECT_EQ(sizedBuffer.AllocatedSize(), static_cast<size_t>(4));
}

} // namespace
