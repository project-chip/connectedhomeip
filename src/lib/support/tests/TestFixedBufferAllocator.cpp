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

#include <lib/support/FixedBufferAllocator.h>

#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

namespace {

TEST(TestFixedBufferAllocator, TestClone)
{
    uint8_t buffer[128];
    FixedBufferAllocator alloc(buffer);

    static const char kTestString[] = "Test string";
    const char * allocatedString    = alloc.Clone(kTestString);

    ASSERT_NE(allocatedString, nullptr);
    EXPECT_NE(allocatedString, kTestString);

    // NOLINTNEXTLINE(clang-analyzer-unix.cstring.NullArg): null check for allocated string already done
    EXPECT_STREQ(allocatedString, kTestString);

    const uint8_t kTestData[]     = { 0xDE, 0xAD, 0xBE, 0xEF };
    const uint8_t * allocatedData = alloc.Clone(kTestData, sizeof(kTestData));

    ASSERT_NE(allocatedData, nullptr);
    EXPECT_NE(allocatedData, kTestData);

    // NOLINTNEXTLINE(clang-analyzer-unix.cstring.NullArg): null check for allocated data already done
    EXPECT_EQ(memcmp(allocatedData, kTestData, sizeof(kTestData)), 0);
}

TEST(TestFixedBufferAllocator, TestOutOfMemory)
{
    uint8_t buffer[16];
    FixedBufferAllocator alloc(buffer);

    static const char kTestData[] = "0123456789abcdef";

    // Allocating 16 bytes still works...
    EXPECT_NE(alloc.Clone(kTestData, 16), nullptr);
    EXPECT_FALSE(alloc.AnyAllocFailed());

    // ...but cannot allocate even one more byte...
    EXPECT_EQ(alloc.Clone(kTestData, 1), nullptr);
    EXPECT_TRUE(alloc.AnyAllocFailed());
}
} // namespace
