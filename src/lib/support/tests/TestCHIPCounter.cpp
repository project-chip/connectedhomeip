/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <nlunit-test.h>

#include <lib/support/CHIPCounter.h>
#include <lib/support/UnitTestRegistration.h>

static void CheckStartWithZero(nlTestSuite * inSuite, void * inContext)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;
    NL_TEST_ASSERT(inSuite, counter.GetValue() == 0);
}

static void CheckInitialize(nlTestSuite * inSuite, void * inContext)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;

    NL_TEST_ASSERT(inSuite, counter.Init(4321) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter.GetValue() == 4321);
}

static void CheckAdvance(nlTestSuite * inSuite, void * inContext)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;

    NL_TEST_ASSERT(inSuite, counter.Init(22) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter.GetValue() == 22);
    NL_TEST_ASSERT(inSuite, counter.Advance() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter.GetValue() == 23);
    NL_TEST_ASSERT(inSuite, counter.Advance() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter.GetValue() == 24);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Start with zero", CheckStartWithZero),
    NL_TEST_DEF("Can initialize",  CheckInitialize),
    NL_TEST_DEF("Can Advance",     CheckAdvance),
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    return (SUCCESS);
}

int TestCHIPCounter()
{
    // clang-format off
    nlTestSuite theSuite = {
        "chip-counter",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPCounter)
