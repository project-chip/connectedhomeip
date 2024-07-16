/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Scoped.h>

namespace {

using namespace chip;

TEST(TestScoped, TestScopedVariableChange)
{
    int x = 123;

    {
        ScopedChange change1(x, 10);
        EXPECT_EQ(x, 10);

        x = 15;
        {
            ScopedChange change2(x, 20);
            EXPECT_EQ(x, 20);
        }
        EXPECT_EQ(x, 15);
    }
    EXPECT_EQ(x, 123);
}

TEST(TestScoped, TestScopedChangeOnly)
{
    ScopedChangeOnly intValue(123);
    ScopedChangeOnly strValue("abc");

    EXPECT_EQ(intValue, 123);
    EXPECT_STREQ(strValue, "abc");

    {
        ScopedChange change1(intValue, 234);

        EXPECT_EQ(intValue, 234);
        EXPECT_STREQ(strValue, "abc");

        ScopedChange change2(strValue, "xyz");
        EXPECT_EQ(intValue, 234);
        EXPECT_STREQ(strValue, "xyz");

        {
            ScopedChange change3(intValue, 10);
            ScopedChange change4(strValue, "test");

            EXPECT_EQ(intValue, 10);
            EXPECT_STREQ(strValue, "test");
        }

        EXPECT_EQ(intValue, 234);
        EXPECT_STREQ(strValue, "xyz");
    }

    EXPECT_EQ(intValue, 123);
    EXPECT_STREQ(strValue, "abc");
}

} // namespace
