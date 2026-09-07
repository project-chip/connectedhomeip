/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/CHIPMem.h>
#include <lib/support/ReferenceCountedPtr.h>

namespace {

using namespace chip;

class TestReferenceCountedPtr : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

class TestCounterRelease
{
public:
    int Counter() { return mCount; }

    TestCounterRelease * Ref()
    {
        mCount++;
        return this;
    }
    void Unref() { mCount--; }

private:
    int mCount = 0;
};

TEST_F(TestReferenceCountedPtr, TestDestruction)
{
    TestCounterRelease refCounted;

    {
        ReferenceCountedPtr<TestCounterRelease> ptr(&refCounted);

        EXPECT_EQ(refCounted.Counter(), 1);
    }
    EXPECT_EQ(refCounted.Counter(), 0);
}

TEST_F(TestReferenceCountedPtr, TestOperators)
{
    TestCounterRelease refCounted;
    TestCounterRelease refCounted2;

    ReferenceCountedPtr<TestCounterRelease> ptr(&refCounted);
    EXPECT_EQ(refCounted.Counter(), 1);
    EXPECT_EQ(ptr->Counter(), 1);
    EXPECT_EQ(&*ptr, &refCounted);
    EXPECT_FALSE(ptr.IsNull());

    ReferenceCountedPtr<TestCounterRelease> other(ptr);
    EXPECT_EQ(ptr, other);
    EXPECT_EQ(refCounted.Counter(), 2);
    other = ptr;
    EXPECT_EQ(refCounted.Counter(), 2);
    other = &refCounted2;
    EXPECT_NE(other, &refCounted);
    EXPECT_EQ(refCounted.Counter(), 1);
    EXPECT_EQ(refCounted2.Counter(), 1);

    ptr.Release();
    EXPECT_EQ(refCounted.Counter(), 0);
    EXPECT_TRUE(ptr.IsNull());

    // Make sure additional release is no-op
    ptr.Release();
    EXPECT_EQ(refCounted.Counter(), 0);

    other = nullptr;
    EXPECT_EQ(refCounted2.Counter(), 0);
    EXPECT_TRUE(other.IsNull());
    EXPECT_NE(other, &refCounted);
}

TEST_F(TestReferenceCountedPtr, TestMove)
{
    TestCounterRelease refCounted;

    ReferenceCountedPtr<TestCounterRelease> source(&refCounted);
    EXPECT_EQ(refCounted.Counter(), 1);
    auto moved = std::move(source);
    EXPECT_EQ(refCounted.Counter(), 1);
    EXPECT_EQ(moved, &refCounted);
}

} // namespace
