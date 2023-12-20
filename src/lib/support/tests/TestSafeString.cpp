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

/**
 *    @file
 *      This file implements a unit test suite for CHIP SafeString functions
 *
 */

#include <lib/support/SafeString.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

static void TestMaxStringLength(nlTestSuite * inSuite, void * inContext)
{
    constexpr size_t len = MaxStringLength("a", "bc", "def");
    NL_TEST_ASSERT(inSuite, len == 3);

    NL_TEST_ASSERT(inSuite, MaxStringLength("bc") == 2);

    NL_TEST_ASSERT(inSuite, MaxStringLength("def", "bc", "a") == 3);

    NL_TEST_ASSERT(inSuite, MaxStringLength("") == 0);
}

static void TestTotalStringLength(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, TotalStringLength("") == 0);
    NL_TEST_ASSERT(inSuite, TotalStringLength("a") == 1);
    NL_TEST_ASSERT(inSuite, TotalStringLength("def", "bc", "a") == 6);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestMaxStringLength), NL_TEST_DEF_FN(TestTotalStringLength), NL_TEST_SENTINEL() };

int TestSafeString()
{
    nlTestSuite theSuite = { "CHIP SafeString tests", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSafeString)
