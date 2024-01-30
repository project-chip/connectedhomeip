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

#include <lib/support/Scoped.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>
#include <string.h>

namespace {

using namespace chip;

void TestScopedVariableChange(nlTestSuite * inSuite, void * inContext)
{
    int x = 123;

    {
        ScopedChange change1(x, 10);
        NL_TEST_ASSERT(inSuite, x == 10);

        x = 15;
        {
            ScopedChange change2(x, 20);
            NL_TEST_ASSERT(inSuite, x == 20);
        }
        NL_TEST_ASSERT(inSuite, x == 15);
    }
    NL_TEST_ASSERT(inSuite, x == 123);
}

void TestScopedChangeOnly(nlTestSuite * inSuite, void * inContext)
{
    ScopedChangeOnly intValue(123);
    ScopedChangeOnly strValue("abc");

    NL_TEST_ASSERT(inSuite, intValue == 123);
    NL_TEST_ASSERT(inSuite, strcmp(strValue, "abc") == 0);

    {
        ScopedChange change1(intValue, 234);

        NL_TEST_ASSERT(inSuite, intValue == 234);
        NL_TEST_ASSERT(inSuite, strcmp(strValue, "abc") == 0);

        ScopedChange change2(strValue, "xyz");
        NL_TEST_ASSERT(inSuite, intValue == 234);
        NL_TEST_ASSERT(inSuite, strcmp(strValue, "xyz") == 0);

        {
            ScopedChange change3(intValue, 10);
            ScopedChange change4(strValue, "test");

            NL_TEST_ASSERT(inSuite, intValue == 10);
            NL_TEST_ASSERT(inSuite, strcmp(strValue, "test") == 0);
        }

        NL_TEST_ASSERT(inSuite, intValue == 234);
        NL_TEST_ASSERT(inSuite, strcmp(strValue, "xyz") == 0);
    }

    NL_TEST_ASSERT(inSuite, intValue == 123);
    NL_TEST_ASSERT(inSuite, strcmp(strValue, "abc") == 0);
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF_FN(TestScopedVariableChange), //
    NL_TEST_DEF_FN(TestScopedChangeOnly),     //
    NL_TEST_SENTINEL()                        //
};

int TestScoped()
{
    nlTestSuite theSuite = { "CHIP Scoped tests", &sTests[0], nullptr, nullptr };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestScoped)
