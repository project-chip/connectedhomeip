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

#include <lib/support/Fold.h>
#include <lib/support/UnitTestRegistration.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <nlunit-test.h>

using namespace chip;

namespace {

void TestFoldMax(nlTestSuite * inSuite, void * inContext)
{
    using List     = std::initializer_list<int>;
    using Limits   = std::numeric_limits<int>;
    const auto max = [](int left, int right) { return std::max(left, right); };

    // Test empty list
    NL_TEST_ASSERT(inSuite, Fold(List{}, -1000, max) == -1000);

    // Test one-element (less than the initial value)
    NL_TEST_ASSERT(inSuite, Fold(List{ -1001 }, -1000, max) == -1000);

    // Test one-element (greater than the initial value)
    NL_TEST_ASSERT(inSuite, Fold(List{ -999 }, -1000, max) == -999);

    // Test limits
    NL_TEST_ASSERT(inSuite, Fold(List{ 1000, Limits::max(), 0 }, 0, max) == Limits::max());
    NL_TEST_ASSERT(inSuite, Fold(List{ Limits::max(), 1000, Limits::min() }, Limits::min(), max) == Limits::max());
}

void TestSum(nlTestSuite * inSuite, void * inContext)
{
    using List   = std::initializer_list<int>;
    using Limits = std::numeric_limits<int>;

    // Test empty list
    NL_TEST_ASSERT(inSuite, Sum(List{}) == 0);

    // Test one-element (min)
    NL_TEST_ASSERT(inSuite, Sum(List{ Limits::min() }) == Limits::min());

    // Test one-element (max)
    NL_TEST_ASSERT(inSuite, Sum(List{ Limits::max() }) == Limits::max());

    // Test multiple elements
    NL_TEST_ASSERT(inSuite, Sum(List{ 0, 5, 1, 4, 2, 3 }) == 15);
}

const nlTest sTests[] = { NL_TEST_DEF("Test fold (max)", TestFoldMax), NL_TEST_DEF("Test sum", TestSum), NL_TEST_SENTINEL() };

} // namespace

int TestFold()
{
    nlTestSuite theSuite = { "Fold tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFold)
