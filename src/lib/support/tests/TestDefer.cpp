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

#include <lib/support/Defer.h>
#include <lib/support/UnitTestRegistration.h>

#include <memory>

#include <nlunit-test.h>

using namespace chip;

namespace {

static void TestDeferUsage(nlTestSuite * inSuite, void * inContext)
{
    bool deferred = false;
    {
        auto deferredFunction = MakeDefer([&]() { deferred = true; });
        NL_TEST_ASSERT(inSuite, !deferred);
    }
    NL_TEST_ASSERT(inSuite, deferred);

    deferred = false;
    {
        std::unique_ptr<int> movable;
        auto deferredFunction = MakeDefer([movable = std::move(movable), &deferred]() { deferred = true; });
    }
    NL_TEST_ASSERT(inSuite, deferred);
}

} // namespace

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestDeferUsage), NL_TEST_SENTINEL() };

int TestDefer(void)
{
    nlTestSuite theSuite = { "CHIP Defer tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDefer)
