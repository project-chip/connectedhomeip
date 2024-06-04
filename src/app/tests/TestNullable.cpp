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

#include <array>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/data-model/Nullable.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::app::DataModel;

namespace {

// Counts calls to constructor and destructor, to determine if the right
// semantics applied in cases where destruction is expected.
struct CtorDtorCounter
{
    CtorDtorCounter(int i) : m(i) { ++created; }
    ~CtorDtorCounter() { ++destroyed; }

    CtorDtorCounter(const CtorDtorCounter & o) : m(o.m) { ++created; }
    CtorDtorCounter & operator=(const CtorDtorCounter &) = default;

    CtorDtorCounter(CtorDtorCounter && o) : m(o.m) { ++created; }
    CtorDtorCounter & operator=(CtorDtorCounter &&) = default;

    bool operator==(const CtorDtorCounter & o) const { return m == o.m; }
    bool operator!=(const CtorDtorCounter & o) const { return m != o.m; }

    int m;

    static void ResetCounter()
    {
        created   = 0;
        destroyed = 0;
    }

    static int created;
    static int destroyed;
};

struct MovableCtorDtorCounter : public CtorDtorCounter
{
public:
    MovableCtorDtorCounter(int i) : CtorDtorCounter(i) {}

    MovableCtorDtorCounter(const MovableCtorDtorCounter & o)           = delete;
    MovableCtorDtorCounter & operator=(const MovableCtorDtorCounter &) = delete;

    MovableCtorDtorCounter(MovableCtorDtorCounter && o)           = default;
    MovableCtorDtorCounter & operator=(MovableCtorDtorCounter &&) = default;

    using CtorDtorCounter::operator==;
    using CtorDtorCounter::operator!=;
};

int CtorDtorCounter::created   = 0;
int CtorDtorCounter::destroyed = 0;

} // namespace

TEST(TestNullable, TestBasic)
{
    // Set up our test CtorDtorCounter objects, which will mess with counts, before we reset the
    // counts.
    CtorDtorCounter c100(100), c101(101), c102(102);

    CtorDtorCounter::ResetCounter();

    {
        auto testNullable = MakeNullable<CtorDtorCounter>(100);
        EXPECT_TRUE(CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(!testNullable.IsNull() && testNullable.Value().m == 100);
        EXPECT_EQ(testNullable, c100);
        EXPECT_NE(testNullable, c101);
        EXPECT_NE(testNullable, c102);

        testNullable.SetNull();
        EXPECT_TRUE(CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 1);
        EXPECT_TRUE(testNullable.IsNull());
        EXPECT_NE(testNullable, c100);
        EXPECT_NE(testNullable, c101);
        EXPECT_NE(testNullable, c102);

        testNullable.SetNonNull(CtorDtorCounter(101));
        EXPECT_TRUE(CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 2);
        EXPECT_TRUE(!testNullable.IsNull() && testNullable.Value().m == 101);
        EXPECT_NE(testNullable, c100);
        EXPECT_EQ(testNullable, c101);
        EXPECT_NE(testNullable, c102);

        testNullable.SetNonNull(102);
        EXPECT_TRUE(CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 3);
        EXPECT_TRUE(!testNullable.IsNull() && testNullable.Value().m == 102);
        EXPECT_NE(testNullable, c100);
        EXPECT_NE(testNullable, c101);
        EXPECT_EQ(testNullable, c102);
    }

    // Our test CtorDtorCounter objects are still in scope here.
    EXPECT_TRUE(CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 4);
}

TEST(TestNullable, TestMake)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testNullable = MakeNullable<CtorDtorCounter>(200);
        EXPECT_TRUE(CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(!testNullable.IsNull() && testNullable.Value().m == 200);
    }

    EXPECT_TRUE(CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 1);
}

TEST(TestNullable, TestCopy)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testSrc = MakeNullable<CtorDtorCounter>(300);
        EXPECT_TRUE(CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(!testSrc.IsNull() && testSrc.Value().m == 300);

        {
            Nullable<CtorDtorCounter> testDst(testSrc);
            EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 0);
            EXPECT_TRUE(!testDst.IsNull() && testDst.Value().m == 300);
        }
        EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 1);

        {
            Nullable<CtorDtorCounter> testDst;
            EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 1);
            EXPECT_TRUE(testDst.IsNull());

            testDst = testSrc;
            EXPECT_TRUE(CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 1);
            EXPECT_TRUE(!testDst.IsNull() && testDst.Value().m == 300);
        }
        EXPECT_TRUE(CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 2);
    }
    EXPECT_TRUE(CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 3);
}

TEST(TestNullable, TestMove)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testSrc = MakeNullable<MovableCtorDtorCounter>(400);     // construct
        Nullable<MovableCtorDtorCounter> testDst(std::move(testSrc)); // move construct
        EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(!testDst.IsNull() && testDst.Value().m == 400);
        // destroy both testsSrc and testDst
    }
    EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 2);

    CtorDtorCounter::ResetCounter();
    {
        Nullable<MovableCtorDtorCounter> testDst; // no object construction
        EXPECT_TRUE(CtorDtorCounter::created == 0 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(testDst.IsNull());

        auto testSrc = MakeNullable<MovableCtorDtorCounter>(401); // construct object
        testDst      = std::move(testSrc);                        // construct a copy
        EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 0);
        EXPECT_TRUE(!testDst.IsNull() && testDst.Value().m == 401);
    }
    EXPECT_TRUE(CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 2);
}

TEST(TestNullable, TestUpdate)
{
    using SmallArray = std::array<uint8_t, 3>;
    // Arrays
    {
        auto nullable1 = MakeNullable<SmallArray>({ 1, 2, 3 });
        auto nullable2 = MakeNullable<SmallArray>({ 1, 2, 3 });

        EXPECT_FALSE(nullable1.IsNull());
        EXPECT_FALSE(nullable2.IsNull());
        EXPECT_EQ(nullable1, nullable2);

        // No-op on change to same.
        EXPECT_FALSE(nullable1.Update(nullable2));
        EXPECT_EQ(nullable1, nullable2);

        nullable1.Value()[0] = 100;

        EXPECT_NE(nullable1, nullable2);
        EXPECT_TRUE(nullable2.Update(nullable1));
        EXPECT_EQ(nullable1, nullable2);
    }

    // Structs
    {
        struct SomeObject
        {
            uint8_t a;
            uint8_t b;

            bool operator==(const SomeObject & other) const { return (a == other.a) && (b == other.b); }
        };

        auto nullable1 = MakeNullable<SomeObject>({ 1, 2 });
        auto nullable2 = MakeNullable<SomeObject>({ 1, 2 });

        EXPECT_FALSE(nullable1.IsNull());
        EXPECT_FALSE(nullable2.IsNull());
        EXPECT_EQ(nullable1, nullable2);

        // No-op on change to same.
        EXPECT_FALSE(nullable1.Update(nullable2));
        EXPECT_EQ(nullable1, nullable2);

        nullable1.Value().a = 100;

        EXPECT_NE(nullable1, nullable2);
        EXPECT_TRUE(nullable2.Update(nullable1));
        EXPECT_EQ(nullable1, nullable2);
    }

    // Scalar cases
    {
        auto nullable1 = MakeNullable(static_cast<uint8_t>(1));

        EXPECT_FALSE(nullable1.IsNull());

        // Non-null to non-null same value
        EXPECT_FALSE(nullable1.Update(nullable1));
        EXPECT_FALSE(nullable1.IsNull());

        // Non-null to null
        EXPECT_TRUE(nullable1.Update(NullNullable));
        EXPECT_TRUE(nullable1.IsNull());

        // Null to null
        EXPECT_FALSE(nullable1.Update(NullNullable));
        EXPECT_TRUE(nullable1.IsNull());

        // Null to non-null
        EXPECT_TRUE(nullable1.Update(MakeNullable(static_cast<uint8_t>(1))));
        EXPECT_FALSE(nullable1.IsNull());
        EXPECT_EQ(nullable1.Value(), 1);

        // Non-null to non-null different value
        EXPECT_TRUE(nullable1.Update(MakeNullable(static_cast<uint8_t>(2))));
        EXPECT_FALSE(nullable1.IsNull());
        EXPECT_EQ(nullable1.Value(), 2);

        // Non-null to extent of range --> changes to "invalid" value in range.
        EXPECT_TRUE(nullable1.Update(MakeNullable(static_cast<uint8_t>(255))));
        EXPECT_FALSE(nullable1.IsNull());
        EXPECT_EQ(nullable1.Value(), 255);
    }
}
