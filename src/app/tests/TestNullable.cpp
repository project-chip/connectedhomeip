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

#include <app/data-model/Nullable.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app::DataModel;

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

static void TestBasic(nlTestSuite * inSuite, void * inContext)
{
    // Set up our test Count objects, which will mess with counts, before we reset the
    // counts.
    Count c100(100), c101(101), c102(102);

    Count::ResetCounter();

    {
        auto testNullable = MakeNullable<Count>(100);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 100);
        NL_TEST_ASSERT(inSuite, testNullable == c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNull();
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 1);
        NL_TEST_ASSERT(inSuite, !!testNullable.IsNull());
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNonNull(Count(101));
        NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 2);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 101);
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable == c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNonNull(102);
        NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 3);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 102);
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable == c102);
    }

    // Our test Count objects are still in scope here.
    NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 4);
}

static void TestMake(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testNullable = MakeNullable<Count>(200);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 200);
    }

    NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 1);
}

static void TestCopy(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testSrc = MakeNullable<Count>(300);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testSrc.IsNull() && testSrc.Value().m == 300);

        {
            Nullable<Count> testDst(testSrc);
            NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 0);
            NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);

        {
            Nullable<Count> testDst;
            NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);
            NL_TEST_ASSERT(inSuite, !!testDst.IsNull());

            testDst = testSrc;
            NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 1);
            NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 2);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 3);
}

static void TestMove(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testSrc = MakeNullable<CountMovable>(400);
        Nullable<CountMovable> testDst(std::move(testSrc));
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);
        NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 400);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 2);

    {
        Nullable<CountMovable> testDst;
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 2);
        NL_TEST_ASSERT(inSuite, !!testDst.IsNull());

        auto testSrc = MakeNullable<CountMovable>(401);
        testDst      = std::move(testSrc);
        NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 3);
        NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 401);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 4);
}

static void TestSetToMatch(nlTestSuite * inSuite, void * inContext)
{
    using SmallArray = std::array<uint8_t, 3>;
    // Arrays
    {
        auto nullable1 = MakeNullable<SmallArray>({1,2,3});
        auto nullable2 = MakeNullable<SmallArray>({1,2,3});

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, !nullable2.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        // No-op on change to same.
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(nullable2) == false);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        nullable1.Value()[0] = 100;

        NL_TEST_ASSERT(inSuite, nullable1 != nullable2);
        NL_TEST_ASSERT(inSuite, nullable2.SetToMatch(nullable1) == true);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);
    }

    // Structs
    {
        struct SomeObject
        {
            uint8_t a;
            uint8_t b;

            bool operator==(const SomeObject & other) const {
              return (a == other.a) && (b == other.b);
            }
        };

        auto nullable1 = MakeNullable<SomeObject>({1, 2});
        auto nullable2 = MakeNullable<SomeObject>({1, 2});

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, !nullable2.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        // No-op on change to same.
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(nullable2) == false);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        nullable1.Value().a = 100;

        NL_TEST_ASSERT(inSuite, nullable1 != nullable2);
        NL_TEST_ASSERT(inSuite, nullable2.SetToMatch(nullable1) == true);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);
    }

    // Scalar cases
    {
        auto nullable1 = MakeNullable(static_cast<uint8_t>(1));

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());

        // Non-null to non-null same value
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(nullable1) == false);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());

        // Non-null to null
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(NullNullable) == true);
        NL_TEST_ASSERT(inSuite, nullable1.IsNull());

        // Null to null
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(NullNullable) == false);
        NL_TEST_ASSERT(inSuite, nullable1.IsNull());

        // Null to non-null
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(MakeNullable(static_cast<uint8_t>(1))) == true);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1.Value() == 1);

        // Non-null to non-null different value
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(MakeNullable(static_cast<uint8_t>(2))) == true);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1.Value() == 2);

        // Non-null to extent of range --> changes to "invalid" value in range.
        NL_TEST_ASSERT(inSuite, nullable1.SetToMatch(MakeNullable(static_cast<uint8_t>(255))) == true);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1.Value() == 255);
    }
}

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("NullableBasic", TestBasic),
    NL_TEST_DEF("NullableMake", TestMake),
    NL_TEST_DEF("NullableCopy", TestCopy),
    NL_TEST_DEF("NullableMove", TestMove),
    NL_TEST_DEF("Nullable SetToMatch operation", TestSetToMatch),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestNullable_Setup(void * inContext)
{
    return SUCCESS;
}

int TestNullable_Teardown(void * inContext)
{
    return SUCCESS;
}

int TestNullable()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Test for Nullable abstraction",
        &sTests[0],
        TestNullable_Setup,
        TestNullable_Teardown
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestNullable)
