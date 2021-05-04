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

/**
 *    @file
 *      This file implements a unit test suite for CHIP SafeInt functions
 *
 */

#include <support/Span.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

static void SpanConstructors(nlTestSuite * inSuite, void * inContext)
{
    uint8_t arr[] = { 1, 2, 3 };
    Span<uint8_t> s1(arr, 2);

    NL_TEST_ASSERT(inSuite, s1.data() == arr);
    NL_TEST_ASSERT(inSuite, s1.size() == 2);

    Span<uint8_t> s2(arr);
    NL_TEST_ASSERT(inSuite, s2.data() == arr);
    NL_TEST_ASSERT(inSuite, s2.size() == 3);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(SpanConstructors), NL_TEST_SENTINEL() };

int TestSpan(void)
{
    nlTestSuite theSuite = { "CHIP Span tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSpan)
