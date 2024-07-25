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

/**
 *    @file
 *      This file implements a unit test suite for CHIP SafeString functions
 *
 */

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/SafeString.h>

using namespace chip;

TEST(TestSafeString, TestMaxStringLength)
{
    constexpr size_t len = MaxStringLength("a", "bc", "def");
    EXPECT_EQ(len, 3u);

    EXPECT_EQ(MaxStringLength("bc"), 2u);

    EXPECT_EQ(MaxStringLength("def", "bc", "a"), 3u);

    EXPECT_EQ(MaxStringLength(""), 0u);
}

TEST(TestSafeString, TestTotalStringLength)
{
    EXPECT_EQ(TotalStringLength(""), 0u);
    EXPECT_EQ(TotalStringLength("a"), 1u);
    EXPECT_EQ(TotalStringLength("def", "bc", "a"), 6u);
}
