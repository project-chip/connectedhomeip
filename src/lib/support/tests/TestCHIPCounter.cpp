/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPCounter)
