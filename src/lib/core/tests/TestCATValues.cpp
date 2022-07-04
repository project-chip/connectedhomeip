/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements unit tests for the CHIP TLV implementation.
 *
 */

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <lib/core/CASEAuthTag.h>

using namespace chip;

void TestEqualityOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a = CATValues{ { 1, 2, 3 } };
        auto b = CATValues{ { 1, 3, 2 } };
        auto c = CATValues{ { 2, 1, 3 } };
        auto d = CATValues{ { 2, 3, 1 } };
        auto e = CATValues{ { 3, 1, 2 } };
        auto f = CATValues{ { 3, 2, 1 } };
        NL_TEST_ASSERT(inSuite, a == a);
        NL_TEST_ASSERT(inSuite, a == b);
        NL_TEST_ASSERT(inSuite, a == c);
        NL_TEST_ASSERT(inSuite, a == d);
        NL_TEST_ASSERT(inSuite, a == e);
        NL_TEST_ASSERT(inSuite, a == f);
        NL_TEST_ASSERT(inSuite, b == a);
        NL_TEST_ASSERT(inSuite, b == b);
        NL_TEST_ASSERT(inSuite, b == c);
        NL_TEST_ASSERT(inSuite, b == d);
        NL_TEST_ASSERT(inSuite, b == e);
        NL_TEST_ASSERT(inSuite, b == f);
        NL_TEST_ASSERT(inSuite, c == a);
        NL_TEST_ASSERT(inSuite, c == b);
        NL_TEST_ASSERT(inSuite, c == c);
        NL_TEST_ASSERT(inSuite, c == d);
        NL_TEST_ASSERT(inSuite, c == e);
        NL_TEST_ASSERT(inSuite, c == f);
        NL_TEST_ASSERT(inSuite, d == a);
        NL_TEST_ASSERT(inSuite, d == b);
        NL_TEST_ASSERT(inSuite, d == c);
        NL_TEST_ASSERT(inSuite, d == d);
        NL_TEST_ASSERT(inSuite, d == e);
        NL_TEST_ASSERT(inSuite, d == f);
        NL_TEST_ASSERT(inSuite, e == a);
        NL_TEST_ASSERT(inSuite, e == b);
        NL_TEST_ASSERT(inSuite, e == c);
        NL_TEST_ASSERT(inSuite, e == d);
        NL_TEST_ASSERT(inSuite, e == e);
        NL_TEST_ASSERT(inSuite, e == f);
        NL_TEST_ASSERT(inSuite, f == a);
        NL_TEST_ASSERT(inSuite, f == b);
        NL_TEST_ASSERT(inSuite, f == c);
        NL_TEST_ASSERT(inSuite, f == d);
        NL_TEST_ASSERT(inSuite, f == e);
        NL_TEST_ASSERT(inSuite, f == f);
    }
    {
        auto a = CATValues{ { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } };
        auto b = CATValues{ { kUndefinedCAT, kUndefinedCAT, kUndefinedCAT } };
        NL_TEST_ASSERT(inSuite, a == b);
    }
}

void TestInequalityOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a = CATValues{ { 1, 2, 3 } };
        auto b = CATValues{ { 4, 5, 6 } };
        NL_TEST_ASSERT(inSuite, a != b);
    }
    {
        auto a = CATValues{ { 1, 2, 3 } };
        auto b = CATValues{ { 1, 1, 1 } };
        NL_TEST_ASSERT(inSuite, a != b);
    }
    {
        auto a = CATValues{ { 1, 2, 3 } };
        auto b = CATValues{ { 1, 2, kUndefinedCAT } };
        NL_TEST_ASSERT(inSuite, a != b);
    }
}

void TestLessThanOperator(nlTestSuite * inSuite, void * inContext)
{
    {
        auto a = CATValues{ { 3, 4, 5 } };
        auto b = CATValues{ { 4, 5, 6 } };
        auto c = CATValues{ { 3, 4, 6 } };
        NL_TEST_ASSERT(inSuite, a < b);
        NL_TEST_ASSERT(inSuite, a < c);
    }
    {
        auto a = CATValues{ { 1, 1, 1 } };
        auto b = CATValues{ { 1, 1, 2 } };
        auto c = CATValues{ { 2, 1, 1 } };
        NL_TEST_ASSERT(inSuite, a < b);
        NL_TEST_ASSERT(inSuite, a < c);
    }
    {
        auto a = CATValues{ { 3, 2, 6 } };
        auto b = CATValues{ { 2, 4, 7 } };
        auto c = CATValues{ { 3, 2, 7 } };
        NL_TEST_ASSERT(inSuite, a < b);
        NL_TEST_ASSERT(inSuite, a < c);
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
    NL_TEST_DEF("Less Than operator", TestLessThanOperator),
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
