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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ScopedBuffer.h>

namespace {

class TestScopedBuffer : public ::testing::Test
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
    static void * MemoryAlloc(size_t size)
    {
        mAllocCount++;
        return chip::Platform::MemoryAlloc(size);
    }
    static void * MemoryCalloc(size_t num, size_t size)
    {

        mAllocCount++;
        return chip::Platform::MemoryCalloc(num, size);
    }

private:
    static int mAllocCount;
};
int TestCounterMemoryManagement::mAllocCount = 0;

using TestCounterScopedBuffer = chip::Platform::ScopedMemoryBuffer<char, TestCounterMemoryManagement>;

TEST_F(TestScopedBuffer, TestAutoFree)
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

TEST_F(TestScopedBuffer, TestFreeDuringAllocs)
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

TEST_F(TestScopedBuffer, TestRelease)
{
    EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
    void * ptr = nullptr;

    {
        TestCounterScopedBuffer buffer;

        EXPECT_EQ(TestCounterMemoryManagement::Counter(), 0);
        EXPECT_TRUE(buffer.Alloc(128));
        EXPECT_NE(buffer.Get(), nullptr);

        ptr = buffer.Release();
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(buffer.Get(), nullptr);
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

} // namespace
