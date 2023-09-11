/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSafeString)
