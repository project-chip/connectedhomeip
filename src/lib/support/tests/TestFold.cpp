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
#include <cstring>
#include <initializer_list>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Fold.h>

using namespace chip;

namespace {

TEST(TestFold, TestFoldMax)
{
    using List     = std::initializer_list<int>;
    using Limits   = std::numeric_limits<int>;
    const auto max = [](int left, int right) { return std::max(left, right); };

    // Test empty list
    EXPECT_EQ(Fold(List{}, -1000, max), -1000);

    // Test one-element (less than the initial value)
    EXPECT_EQ(Fold(List{ -1001 }, -1000, max), -1000);

    // Test one-element (greater than the initial value)
    EXPECT_EQ(Fold(List{ -999 }, -1000, max), -999);

    // Test limits
    EXPECT_EQ(Fold(List{ 1000, Limits::max(), 0 }, 0, max), Limits::max());
    EXPECT_EQ(Fold(List{ Limits::max(), 1000, Limits::min() }, Limits::min(), max), Limits::max());
}

TEST(TestFold, TestSum)
{
    using List   = std::initializer_list<int>;
    using Limits = std::numeric_limits<int>;

    // Test empty list
    EXPECT_FALSE(Sum(List{}));

    // Test one-element (min)
    EXPECT_EQ(Sum(List{ Limits::min() }), Limits::min());

    // Test one-element (max)
    EXPECT_EQ(Sum(List{ Limits::max() }), Limits::max());

    // Test multiple elements
    EXPECT_EQ(Sum(List{ 0, 5, 1, 4, 2, 3 }), 15);
}

} // namespace
