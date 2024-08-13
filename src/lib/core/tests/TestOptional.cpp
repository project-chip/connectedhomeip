/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a test for  CHIP core library reference counted object.
 *
 */

#include <array>
#include <cinttypes>
#include <cstdint>
#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

using namespace chip;

struct Count
{
    Count(int i) : m(i) { ++created; }
    ~Count() { ++destroyed; }

    Count(const Count & o) : m(o.m) { ++created; }
    Count & operator=(const Count &) = default;

    Count(Count && o) : m(o.m) { ++created; }
    Count & operator=(Count &&) = default;

    bool operator==(const Count & o) const { return m == o.m; }

    int m;

    static void ResetCounter()
    {
        created   = 0;
        destroyed = 0;
    }

    static int created;
    static int destroyed;
};

struct CountMovable : public Count
{
public:
    CountMovable(int i) : Count(i) {}

    CountMovable(const CountMovable & o)           = delete;
    CountMovable & operator=(const CountMovable &) = delete;

    CountMovable(CountMovable && o)           = default;
    CountMovable & operator=(CountMovable &&) = default;
};

int Count::created;
int Count::destroyed;

TEST(TestOptional, TestBasic)
{
    // Set up our test Count objects, which will mess with counts, before we reset the
    // counts.
    Count c100(100), c101(101), c102(102);

    Count::ResetCounter();

    {
        auto testOptional = Optional<Count>::Value(100);
        EXPECT_TRUE(Count::created == 1 && Count::destroyed == 0);
        EXPECT_TRUE(testOptional.HasValue() && testOptional.Value().m == 100);
        EXPECT_EQ(testOptional, c100);
        EXPECT_NE(testOptional, c101);
        EXPECT_NE(testOptional, c102);

        testOptional.ClearValue();
        EXPECT_TRUE(Count::created == 1 && Count::destroyed == 1);
        EXPECT_FALSE(testOptional.HasValue());
        EXPECT_NE(testOptional, c100);
        EXPECT_NE(testOptional, c101);
        EXPECT_NE(testOptional, c102);

        testOptional.SetValue(Count(101));
        EXPECT_TRUE(Count::created == 3 && Count::destroyed == 2);
        EXPECT_TRUE(testOptional.HasValue() && testOptional.Value().m == 101);
        EXPECT_NE(testOptional, c100);
        EXPECT_EQ(testOptional, c101);
        EXPECT_NE(testOptional, c102);

        testOptional.Emplace(102);
        EXPECT_TRUE(Count::created == 4 && Count::destroyed == 3);
        EXPECT_TRUE(testOptional.HasValue() && testOptional.Value().m == 102);
        EXPECT_NE(testOptional, c100);
        EXPECT_NE(testOptional, c101);
        EXPECT_EQ(testOptional, c102);
    }

    // Our test Count objects are still in scope here.
    EXPECT_TRUE(Count::created == 4 && Count::destroyed == 4);
}

TEST(TestOptional, TestMake)
{
    Count::ResetCounter();

    {
        auto testOptional = MakeOptional<Count>(200);
        EXPECT_TRUE(Count::created == 1 && Count::destroyed == 0);
        EXPECT_TRUE(testOptional.HasValue() && testOptional.Value().m == 200);
    }

    EXPECT_TRUE(Count::created == 1 && Count::destroyed == 1);
}

TEST(TestOptional, TestCopy)
{
    Count::ResetCounter();

    {
        auto testSrc = Optional<Count>::Value(300);
        EXPECT_TRUE(Count::created == 1 && Count::destroyed == 0);
        EXPECT_TRUE(testSrc.HasValue() && testSrc.Value().m == 300);

        {
            Optional<Count> testDst(testSrc);
            EXPECT_TRUE(Count::created == 2 && Count::destroyed == 0);
            EXPECT_TRUE(testDst.HasValue() && testDst.Value().m == 300);
        }
        EXPECT_TRUE(Count::created == 2 && Count::destroyed == 1);

        {
            Optional<Count> testDst;
            EXPECT_TRUE(Count::created == 2 && Count::destroyed == 1);
            EXPECT_FALSE(testDst.HasValue());

            testDst = testSrc;
            EXPECT_TRUE(Count::created == 3 && Count::destroyed == 1);
            EXPECT_TRUE(testDst.HasValue() && testDst.Value().m == 300);
        }
        EXPECT_TRUE(Count::created == 3 && Count::destroyed == 2);
    }
    EXPECT_TRUE(Count::created == 3 && Count::destroyed == 3);
}

TEST(TestOptional, TestMove)
{
    Count::ResetCounter();

    {
        auto testSrc = MakeOptional<CountMovable>(400);
        Optional<CountMovable> testDst(std::move(testSrc));
        EXPECT_TRUE(Count::created == 2 && Count::destroyed == 1);
        EXPECT_TRUE(testDst.HasValue() && testDst.Value().m == 400);
    }
    EXPECT_TRUE(Count::created == 2 && Count::destroyed == 2);

    {
        Optional<CountMovable> testDst;
        EXPECT_TRUE(Count::created == 2 && Count::destroyed == 2);
        EXPECT_FALSE(testDst.HasValue());

        auto testSrc = MakeOptional<CountMovable>(401);
        testDst      = std::move(testSrc);
        EXPECT_TRUE(Count::created == 4 && Count::destroyed == 3);
        EXPECT_TRUE(testDst.HasValue() && testDst.Value().m == 401);
    }
    EXPECT_TRUE(Count::created == 4 && Count::destroyed == 4);
}

TEST(TestOptional, TestConversion)
{
    // FixedSpan is implicitly convertible from std::array
    using WidgetView    = FixedSpan<const bool, 10>;
    using WidgetStorage = std::array<bool, 10>;

    auto optStorage                   = MakeOptional<WidgetStorage>();
    auto const & constOptStorage      = optStorage;
    auto optOtherStorage              = MakeOptional<WidgetStorage>();
    auto const & constOptOtherStorage = optOtherStorage;

    EXPECT_TRUE(optStorage.HasValue());
    EXPECT_TRUE(optOtherStorage.HasValue());

    Optional<WidgetView> optView(constOptStorage);
    EXPECT_TRUE(optView.HasValue());
    EXPECT_EQ(&optView.Value()[0], &optStorage.Value()[0]);

    optView = optOtherStorage;
    optView = constOptOtherStorage;
    EXPECT_TRUE(optView.HasValue());
    EXPECT_EQ(&optView.Value()[0], &optOtherStorage.Value()[0]);

    struct ExplicitBool
    {
        explicit ExplicitBool(bool) {}
    };
    Optional<ExplicitBool> e(Optional<bool>(true)); // OK, explicitly constructing the optional

    // The following should not compile
    // e = Optional<bool>(false); // relies on implicit conversion
}
