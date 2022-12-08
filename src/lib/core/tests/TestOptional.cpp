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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <lib/core/Optional.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

struct Count
{
    Count(int i) : m(i) { ++created; }
    ~Count() { ++destroyed; }

    Count(const Count & o) : m(o.m) { ++created; }
    Count & operator=(const Count &) = default;

    Count(Count && o) : m(o.m) { ++created; }
    Count & operator=(Count &&) = default;

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

    CountMovable(const CountMovable & o) = delete;
    CountMovable & operator=(const CountMovable &) = delete;

    CountMovable(CountMovable && o) = default;
    CountMovable & operator=(CountMovable &&) = default;
};

int Count::created;
int Count::destroyed;

static void TestBasic(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testOptional = Optional<Count>::Value(100);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, testOptional.HasValue() && testOptional.Value().m == 100);

        testOptional.ClearValue();
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 1);
        NL_TEST_ASSERT(inSuite, !testOptional.HasValue());

        testOptional.SetValue(Count(101));
        NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 2);
        NL_TEST_ASSERT(inSuite, testOptional.HasValue() && testOptional.Value().m == 101);

        testOptional.Emplace(102);
        NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 3);
        NL_TEST_ASSERT(inSuite, testOptional.HasValue() && testOptional.Value().m == 102);
    }

    NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 4);
}

static void TestMake(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testOptional = MakeOptional<Count>(200);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, testOptional.HasValue() && testOptional.Value().m == 200);
    }

    NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 1);
}

static void TestCopy(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testSrc = Optional<Count>::Value(300);
        NL_TEST_ASSERT(inSuite, Count::created == 1 && Count::destroyed == 0);
        NL_TEST_ASSERT(inSuite, testSrc.HasValue() && testSrc.Value().m == 300);

        {
            Optional<Count> testDst(testSrc);
            NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 0);
            NL_TEST_ASSERT(inSuite, testDst.HasValue() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);

        {
            Optional<Count> testDst;
            NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);
            NL_TEST_ASSERT(inSuite, !testDst.HasValue());

            testDst = testSrc;
            NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 1);
            NL_TEST_ASSERT(inSuite, testDst.HasValue() && testDst.Value().m == 300);
        }
        NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 2);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 3 && Count::destroyed == 3);
}

static void TestMove(nlTestSuite * inSuite, void * inContext)
{
    Count::ResetCounter();

    {
        auto testSrc = MakeOptional<CountMovable>(400);
        Optional<CountMovable> testDst(std::move(testSrc));
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 1);
        NL_TEST_ASSERT(inSuite, testDst.HasValue() && testDst.Value().m == 400);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 2);

    {
        Optional<CountMovable> testDst;
        NL_TEST_ASSERT(inSuite, Count::created == 2 && Count::destroyed == 2);
        NL_TEST_ASSERT(inSuite, !testDst.HasValue());

        auto testSrc = MakeOptional<CountMovable>(401);
        testDst      = std::move(testSrc);
        NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 3);
        NL_TEST_ASSERT(inSuite, testDst.HasValue() && testDst.Value().m == 401);
    }
    NL_TEST_ASSERT(inSuite, Count::created == 4 && Count::destroyed == 4);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("OptionalBasic", TestBasic),
    NL_TEST_DEF("OptionalMake", TestMake),
    NL_TEST_DEF("OptionalCopy", TestCopy),
    NL_TEST_DEF("OptionalMove", TestMove),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestOptional_Setup(void * inContext)
{
    return SUCCESS;
}

int TestOptional_Teardown(void * inContext)
{
    return SUCCESS;
}

int TestOptional()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Optional",
        &sTests[0],
        TestOptional_Setup,
        TestOptional_Teardown
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestOptional)
