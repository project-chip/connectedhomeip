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

#include <lib/support/AutoRelease.h>
#include <lib/support/CHIPMem.h>

namespace {

using namespace chip;

class TestAutoRelease : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

class TestCounterRelease
{
public:
    int Counter() { return mCount; }

    void Release() { mCount--; }

private:
    int mCount = 1;
};

TEST_F(TestAutoRelease, TestDestruction)
{
    TestCounterRelease releasable;

    {
        AutoRelease<TestCounterRelease> releaser(&releasable);

        EXPECT_EQ(releasable.Counter(), 1);
    }
    EXPECT_EQ(releasable.Counter(), 0);
}

TEST_F(TestAutoRelease, TestOperators)
{
    TestCounterRelease releasable;
    TestCounterRelease releasable2;

    AutoRelease<TestCounterRelease> releaser(&releasable);
    EXPECT_EQ(releasable.Counter(), 1);
    EXPECT_EQ(releaser->Counter(), 1);
    EXPECT_EQ(&*releaser, &releasable);
    EXPECT_FALSE(releaser.IsNull());

    AutoRelease<TestCounterRelease> other(&releasable);
    EXPECT_EQ(releasable.Counter(), 1);
    other.Set(&releasable);
    EXPECT_EQ(releasable.Counter(), 1);
    other.Set(&releasable2);
    EXPECT_NE(&*other, &releasable);
    EXPECT_EQ(&*other, &releasable2);
    EXPECT_EQ(releasable.Counter(), 0);

    releaser.Release();
    EXPECT_EQ(releasable.Counter(), -1);
    EXPECT_TRUE(releaser.IsNull());

    // Make sure additional release is no-op
    releaser.Release();
    EXPECT_EQ(releasable.Counter(), -1);

    other.Set(nullptr);
    EXPECT_EQ(releasable2.Counter(), 0);
    EXPECT_TRUE(other.IsNull());
}

} // namespace
