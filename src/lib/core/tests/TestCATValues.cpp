/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <lib/core/CASEAuthTag.h>

using namespace chip;

void TestEqualityOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a                 = CATValues{ { 1, 2, 3 } };
        auto b                 = CATValues{ { 1, 3, 2 } };
        auto c                 = CATValues{ { 2, 1, 3 } };
        auto d                 = CATValues{ { 2, 3, 1 } };
        auto e                 = CATValues{ { 3, 1, 2 } };
        auto f                 = CATValues{ { 3, 2, 1 } };
        CATValues candidates[] = { a, b, c, d, e, f };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
    {
        auto a                 = CATValues{ { 1, 2 } };
        auto b                 = CATValues{ { 1, 1, 2 } };
        auto c                 = CATValues{ { 1, 2, 2 } };
        auto d                 = CATValues{ { 1, 2, 1 } };
        auto e                 = CATValues{ { 2, 1 } };
        auto f                 = CATValues{ { 2, 1, 1 } };
        auto g                 = CATValues{ { 2, 2, 1 } };
        auto h                 = CATValues{ { 2, 1, 2 } };
        CATValues candidates[] = { a, b, c, d, e, f, g, h };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
    {
        auto a                 = CATValues{ { 1 } };
        auto b                 = CATValues{ { 1, 1 } };
        auto c                 = CATValues{ { 1, 1, 1 } };
        CATValues candidates[] = { a, b, c };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
    {
        auto a                 = CATValues{ {} };
        auto b                 = CATValues{ {} };
        CATValues candidates[] = { a, b };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                NL_TEST_ASSERT(inSuite, inner == outer);
            }
        }
    }
}

void TestInequalityOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a                 = CATValues{ { 1 } };
        auto b                 = CATValues{ { 1, 2 } };
        auto c                 = CATValues{ { 1, 2, 3 } };
        auto d                 = CATValues{ { 2 } };
        auto e                 = CATValues{ { 2, 3 } };
        auto f                 = CATValues{ { 2, 3, 4 } };
        auto g                 = CATValues{ { 3 } };
        auto h                 = CATValues{ { 3, 4 } };
        auto i                 = CATValues{ { 3, 4, 5 } };
        auto j                 = CATValues{ { 4 } };
        auto k                 = CATValues{ { 4, 5 } };
        auto l                 = CATValues{ { 4, 5, 6 } };
        auto m                 = CATValues{ {
            5,
        } };
        auto n                 = CATValues{ { 5, 6 } };
        auto o                 = CATValues{ { 5, 6, 7 } };
        auto p                 = CATValues{ { 6 } };
        auto q                 = CATValues{ { 6, 7 } };
        auto r                 = CATValues{ { 6, 7, 8 } };
        auto s                 = CATValues{ { 7 } };
        auto t                 = CATValues{ { 7, 8 } };
        auto u                 = CATValues{ { 7, 8, 9 } };
        CATValues candidates[] = { a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u };
        for (auto & outer : candidates)
        {
            for (auto & inner : candidates)
            {
                if (&inner == &outer)
                {
                    continue;
                }
                NL_TEST_ASSERT(inSuite, inner != outer);
            }
        }
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Equality operator", TestEqualityOperator),
    NL_TEST_DEF("Inequality operator", TestInequalityOperator),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestCATValues_Setup(void * inContext)
{
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestCATValues_Teardown(void * inContext)
{
    return SUCCESS;
}

int TestCATValues(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "CATValues",
        &sTests[0],
        TestCATValues_Setup,
        TestCATValues_Teardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCATValues)
