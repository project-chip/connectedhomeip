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
};

int CtorDtorCounter::created   = 0;
int CtorDtorCounter::destroyed = 0;

} // namespace

static void TestBasic(nlTestSuite * inSuite, void * inContext)
{
    // Set up our test CtorDtorCounter objects, which will mess with counts, before we reset the
    // counts.
    CtorDtorCounter c100(100), c101(101), c102(102);

    CtorDtorCounter::ResetCounter();

    {
        auto testNullable = MakeNullable<CtorDtorCounter>(100);
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 100);
        NL_TEST_ASSERT(inSuite, testNullable == c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNull();
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 1);
        NL_TEST_ASSERT(inSuite, !!testNullable.IsNull());
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNonNull(CtorDtorCounter(101));
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 2);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 101);
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable == c101);
        NL_TEST_ASSERT(inSuite, testNullable != c102);

        testNullable.SetNonNull(102);
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 3);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 102);
        NL_TEST_ASSERT(inSuite, testNullable != c100);
        NL_TEST_ASSERT(inSuite, testNullable != c101);
        NL_TEST_ASSERT(inSuite, testNullable == c102);
    }

    // Our test CtorDtorCounter objects are still in scope here.
    NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 4);
}

static void TestMake(nlTestSuite * inSuite, void * inContext)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testNullable = MakeNullable<CtorDtorCounter>(200);
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testNullable.IsNull() && testNullable.Value().m == 200);
    }

    NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 1);
}

static void TestCopy(nlTestSuite * inSuite, void * inContext)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testSrc = MakeNullable<CtorDtorCounter>(300);
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 1 && CtorDtorCounter::destroyed == 0);
        NL_TEST_ASSERT(inSuite, !testSrc.IsNull() && testSrc.Value().m == 300);

        {
            Nullable<CtorDtorCounter> testDst(testSrc);
            NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 0);
            NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 1);

        {
            Nullable<CtorDtorCounter> testDst;
            NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 1);
            NL_TEST_ASSERT(inSuite, !!testDst.IsNull());

            testDst = testSrc;
            NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 1);
            NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 2);
    }
    NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 3 && CtorDtorCounter::destroyed == 3);
}

static void TestMove(nlTestSuite * inSuite, void * inContext)
{
    CtorDtorCounter::ResetCounter();

    {
        auto testSrc = MakeNullable<MovableCtorDtorCounter>(400);
        Nullable<MovableCtorDtorCounter> testDst(std::move(testSrc));
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 1);
        NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 400);
    }
    NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 2);

    {
        Nullable<MovableCtorDtorCounter> testDst;
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 2 && CtorDtorCounter::destroyed == 2);
        NL_TEST_ASSERT(inSuite, !!testDst.IsNull());

        auto testSrc = MakeNullable<MovableCtorDtorCounter>(401);
        testDst      = std::move(testSrc);
        NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 3);
        NL_TEST_ASSERT(inSuite, !testDst.IsNull() && testDst.Value().m == 401);
    }
    NL_TEST_ASSERT(inSuite, CtorDtorCounter::created == 4 && CtorDtorCounter::destroyed == 4);
}

static void TestUpdate(nlTestSuite * inSuite, void * inContext)
{
    using SmallArray = std::array<uint8_t, 3>;
    // Arrays
    {
        auto nullable1 = MakeNullable<SmallArray>({ 1, 2, 3 });
        auto nullable2 = MakeNullable<SmallArray>({ 1, 2, 3 });

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, !nullable2.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        // No-op on change to same.
        NL_TEST_ASSERT(inSuite, nullable1.Update(nullable2) == false);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        nullable1.Value()[0] = 100;

        NL_TEST_ASSERT(inSuite, nullable1 != nullable2);
        NL_TEST_ASSERT(inSuite, nullable2.Update(nullable1) == true);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);
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

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, !nullable2.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        // No-op on change to same.
        NL_TEST_ASSERT(inSuite, nullable1.Update(nullable2) == false);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);

        nullable1.Value().a = 100;

        NL_TEST_ASSERT(inSuite, nullable1 != nullable2);
        NL_TEST_ASSERT(inSuite, nullable2.Update(nullable1) == true);
        NL_TEST_ASSERT(inSuite, nullable1 == nullable2);
    }

    // Scalar cases
    {
        auto nullable1 = MakeNullable(static_cast<uint8_t>(1));

        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());

        // Non-null to non-null same value
        NL_TEST_ASSERT(inSuite, nullable1.Update(nullable1) == false);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());

        // Non-null to null
        NL_TEST_ASSERT(inSuite, nullable1.Update(NullNullable) == true);
        NL_TEST_ASSERT(inSuite, nullable1.IsNull());

        // Null to null
        NL_TEST_ASSERT(inSuite, nullable1.Update(NullNullable) == false);
        NL_TEST_ASSERT(inSuite, nullable1.IsNull());

        // Null to non-null
        NL_TEST_ASSERT(inSuite, nullable1.Update(MakeNullable(static_cast<uint8_t>(1))) == true);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1.Value() == 1);

        // Non-null to non-null different value
        NL_TEST_ASSERT(inSuite, nullable1.Update(MakeNullable(static_cast<uint8_t>(2))) == true);
        NL_TEST_ASSERT(inSuite, !nullable1.IsNull());
        NL_TEST_ASSERT(inSuite, nullable1.Value() == 2);

        // Non-null to extent of range --> changes to "invalid" value in range.
        NL_TEST_ASSERT(inSuite, nullable1.Update(MakeNullable(static_cast<uint8_t>(255))) == true);
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
    NL_TEST_DEF("Nullable Update operation", TestUpdate),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestNullable()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Test for Nullable abstraction",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestNullable)
