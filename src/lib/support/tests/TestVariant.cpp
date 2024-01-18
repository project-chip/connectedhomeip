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

#include <functional>

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/Variant.h>

#include <nlunit-test.h>

namespace {

struct Simple
{
    bool operator==(const Simple &) const { return true; }
};

struct Pod
{
    Pod(int v1, int v2) : m1(v1), m2(v2) {}
    bool operator==(const Pod & other) const { return m1 == other.m1 && m2 == other.m2; }

    int m1;
    int m2;
};

struct Movable
{
    Movable(int v1, int v2) : m1(v1), m2(v2) {}

    Movable(Movable &)             = delete;
    Movable & operator=(Movable &) = delete;

    Movable(Movable &&)             = default;
    Movable & operator=(Movable &&) = default;

    int m1;
    int m2;
};

struct Count
{
    Count() { ++created; }
    ~Count() { ++destroyed; }

    Count(const Count &) { ++created; }
    Count & operator=(Count &) = default;

    Count(Count &&) { ++created; }
    Count & operator=(Count &&) = default;

    static int created;
    static int destroyed;
};

int Count::created   = 0;
int Count::destroyed = 0;

using namespace chip;

void TestVariantSimple(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Pod> v;
    NL_TEST_ASSERT(inSuite, !v.Valid());
    v.Set<Pod>(5, 10);
    NL_TEST_ASSERT(inSuite, v.Valid());
    NL_TEST_ASSERT(inSuite, v.Is<Pod>());
    NL_TEST_ASSERT(inSuite, v.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v.Get<Pod>().m2 == 10);
}

void TestVariantMovable(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Movable> v;
    v.Set<Simple>();
    v.Set<Movable>(Movable{ 5, 10 });
    NL_TEST_ASSERT(inSuite, v.Get<Movable>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v.Get<Movable>().m2 == 10);
    auto & m = v.Get<Movable>();
    NL_TEST_ASSERT(inSuite, m.m1 == 5);
    NL_TEST_ASSERT(inSuite, m.m2 == 10);
    v.Set<Simple>();
}

void TestVariantCtorDtor(nlTestSuite * inSuite, void * inContext)
{
    {
        Variant<Simple, Count> v;
        NL_TEST_ASSERT(inSuite, Count::created == 0);
        v.Set<Simple>();
        NL_TEST_ASSERT(inSuite, Count::created == 0);
        v.Get<Simple>();
        NL_TEST_ASSERT(inSuite, Count::created == 0);
    }
    {
        Variant<Simple, Count> v;
        NL_TEST_ASSERT(inSuite, Count::created == 0);
        v.Set<Simple>();
        NL_TEST_ASSERT(inSuite, Count::created == 0);
        v.Set<Count>();
        NL_TEST_ASSERT(inSuite, Count::created == 1);
        NL_TEST_ASSERT(inSuite, Count::destroyed == 0);
        v.Get<Count>();
        NL_TEST_ASSERT(inSuite, Count::created == 1);
        NL_TEST_ASSERT(inSuite, Count::destroyed == 0);
        v.Set<Simple>();
        NL_TEST_ASSERT(inSuite, Count::created == 1);
        NL_TEST_ASSERT(inSuite, Count::destroyed == 1);
        v.Set<Count>();
        NL_TEST_ASSERT(inSuite, Count::created == 2);
        NL_TEST_ASSERT(inSuite, Count::destroyed == 1);
    }
    NL_TEST_ASSERT(inSuite, Count::destroyed == 2);

    {
        Variant<Simple, Count> v1;
        v1.Set<Count>();
        Variant<Simple, Count> v2(v1);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 4);
    NL_TEST_ASSERT(inSuite, Count::destroyed == 4);

    {
        Variant<Simple, Count> v1;
        v1.Set<Count>();
        Variant<Simple, Count> v2(std::move(v1));
    }
    NL_TEST_ASSERT(inSuite, Count::created == 6);
    NL_TEST_ASSERT(inSuite, Count::destroyed == 6);

    {
        Variant<Simple, Count> v1, v2;
        v1.Set<Count>();
        v2 = v1;
    }
    NL_TEST_ASSERT(inSuite, Count::created == 8);
    NL_TEST_ASSERT(inSuite, Count::destroyed == 8);

    {
        Variant<Simple, Count> v1, v2;
        v1.Set<Count>();
        v2 = std::move(v1);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 10);
    NL_TEST_ASSERT(inSuite, Count::destroyed == 10);
}

void TestVariantCopy(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Pod> v1;
    v1.Set<Pod>(5, 10);
    Variant<Simple, Pod> v2 = v1;
    NL_TEST_ASSERT(inSuite, v1.Valid());
    NL_TEST_ASSERT(inSuite, v1.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v1.Get<Pod>().m2 == 10);
    NL_TEST_ASSERT(inSuite, v2.Valid());
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m2 == 10);
}

void TestVariantMove(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Movable> v1;
    v1.Set<Movable>(5, 10);
    Variant<Simple, Movable> v2 = std::move(v1);
    NL_TEST_ASSERT(inSuite, !v1.Valid()); // NOLINT(bugprone-use-after-move)
    NL_TEST_ASSERT(inSuite, v2.Valid());
    NL_TEST_ASSERT(inSuite, v2.Get<Movable>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v2.Get<Movable>().m2 == 10);
}

void TestVariantCopyAssign(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    v1.Set<Pod>(5, 10);
    v2 = v1;
    NL_TEST_ASSERT(inSuite, v1.Valid());
    NL_TEST_ASSERT(inSuite, v1.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v1.Get<Pod>().m2 == 10);
    NL_TEST_ASSERT(inSuite, v2.Valid());
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m2 == 10);
}

void TestVariantMoveAssign(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    v1.Set<Pod>(5, 10);
    v2 = std::move(v1);
    NL_TEST_ASSERT(inSuite, !v1.Valid()); // NOLINT(bugprone-use-after-move)
    NL_TEST_ASSERT(inSuite, v2.Valid());
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m1 == 5);
    NL_TEST_ASSERT(inSuite, v2.Get<Pod>().m2 == 10);
}

void TestVariantInPlace(nlTestSuite * inSuite, void * inContext)
{
    int i = 0;

    Variant<std::reference_wrapper<int>> v1 = Variant<std::reference_wrapper<int>>(InPlaceTemplate<std::reference_wrapper<int>>, i);
    NL_TEST_ASSERT(inSuite, v1.Valid());
    NL_TEST_ASSERT(inSuite, v1.Is<std::reference_wrapper<int>>());
    NL_TEST_ASSERT(inSuite, &v1.Get<std::reference_wrapper<int>>().get() == &i);

    Variant<std::reference_wrapper<int>> v2 = Variant<std::reference_wrapper<int>>::Create<std::reference_wrapper<int>>(i);
    NL_TEST_ASSERT(inSuite, v2.Valid());
    NL_TEST_ASSERT(inSuite, v2.Is<std::reference_wrapper<int>>());
    NL_TEST_ASSERT(inSuite, &v2.Get<std::reference_wrapper<int>>().get() == &i);
}

void TestVariantCompare(nlTestSuite * inSuite, void * inContext)
{
    Variant<Simple, Pod> v0;
    Variant<Simple, Pod> v1;
    Variant<Simple, Pod> v2;
    Variant<Simple, Pod> v3;
    Variant<Simple, Pod> v4;

    v1.Set<Simple>();
    v2.Set<Pod>(5, 10);
    v3.Set<Pod>(5, 10);
    v4.Set<Pod>(5, 11);

    NL_TEST_ASSERT(inSuite, (v0 == v0));
    NL_TEST_ASSERT(inSuite, !(v0 == v1));
    NL_TEST_ASSERT(inSuite, !(v0 == v2));
    NL_TEST_ASSERT(inSuite, !(v0 == v3));
    NL_TEST_ASSERT(inSuite, !(v0 == v4));

    NL_TEST_ASSERT(inSuite, !(v1 == v0));
    NL_TEST_ASSERT(inSuite, (v1 == v1));
    NL_TEST_ASSERT(inSuite, !(v1 == v2));
    NL_TEST_ASSERT(inSuite, !(v1 == v3));
    NL_TEST_ASSERT(inSuite, !(v1 == v4));

    NL_TEST_ASSERT(inSuite, !(v2 == v0));
    NL_TEST_ASSERT(inSuite, !(v2 == v1));
    NL_TEST_ASSERT(inSuite, (v2 == v2));
    NL_TEST_ASSERT(inSuite, (v2 == v3));
    NL_TEST_ASSERT(inSuite, !(v2 == v4));

    NL_TEST_ASSERT(inSuite, !(v3 == v0));
    NL_TEST_ASSERT(inSuite, !(v3 == v1));
    NL_TEST_ASSERT(inSuite, (v3 == v2));
    NL_TEST_ASSERT(inSuite, (v3 == v3));
    NL_TEST_ASSERT(inSuite, !(v3 == v4));

    NL_TEST_ASSERT(inSuite, !(v4 == v0));
    NL_TEST_ASSERT(inSuite, !(v4 == v1));
    NL_TEST_ASSERT(inSuite, !(v4 == v2));
    NL_TEST_ASSERT(inSuite, !(v4 == v3));
    NL_TEST_ASSERT(inSuite, (v4 == v4));
}

int Setup(void * inContext)
{
    return SUCCESS;
}

int Teardown(void * inContext)
{
    return SUCCESS;
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestVariantSimple),     NL_TEST_DEF_FN(TestVariantMovable),
                                 NL_TEST_DEF_FN(TestVariantCtorDtor),   NL_TEST_DEF_FN(TestVariantCopy),
                                 NL_TEST_DEF_FN(TestVariantMove),       NL_TEST_DEF_FN(TestVariantCopyAssign),
                                 NL_TEST_DEF_FN(TestVariantMoveAssign), NL_TEST_DEF_FN(TestVariantInPlace),
                                 NL_TEST_DEF_FN(TestVariantCompare),    NL_TEST_SENTINEL() };

int TestVariant()
{
    nlTestSuite theSuite = { "CHIP Variant tests", &sTests[0], Setup, Teardown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestVariant);
