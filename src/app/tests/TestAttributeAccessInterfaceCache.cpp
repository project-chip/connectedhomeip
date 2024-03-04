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

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceCache.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;

namespace {

void TestBasicLifecycle(nlTestSuite * inSuite, void * inContext)
{
    int data1 = 1;
    int data2 = 2;

    // We alias the pointers to given locations to avoid needing to implement anything
    // since the AttributeAccessInterfaceCache only deals in pointers, and never calls
    // the API itself.
    AttributeAccessInterface * accessor1 = reinterpret_cast<AttributeAccessInterface *>(&data1);
    AttributeAccessInterface * accessor2 = reinterpret_cast<AttributeAccessInterface *>(&data2);

    AttributeAccessInterfaceCache<1> cache;

    // Cache can keep track of at least 1 entry,
    NL_TEST_ASSERT(inSuite, cache.Get(1, 1) == nullptr);
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(1, 1));
    cache.MarkUsed(1, 1, accessor1);

    NL_TEST_ASSERT(inSuite, cache.Get(1, 1) == accessor1);
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2) == nullptr);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1) == nullptr);

    cache.MarkUsed(1, 2, accessor1);
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2) == accessor1);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1) == nullptr);

    cache.MarkUsed(1, 2, accessor2);
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2) == accessor2);

    cache.Invalidate();
    NL_TEST_ASSERT(inSuite, cache.Get(1, 1) == nullptr);
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2) == nullptr);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1) == nullptr);

    // Marking unused works, keeps single entry, and is invalidated when invalidated fully.
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(2, 2));
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(3, 3));
    cache.MarkUnused(2, 2);
    NL_TEST_ASSERT(inSuite, cache.IsUnused(2, 2));
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(3, 3));

    cache.MarkUnused(3, 3);
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(2, 2));
    NL_TEST_ASSERT(inSuite, cache.IsUnused(3, 3));

    cache.Invalidate();
    NL_TEST_ASSERT(inSuite, !cache.IsUnused(3, 3));
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Basic AttributeAccessInterfaceCache lifecycle works", TestBasicLifecycle),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestAttributeAccessInterfaceCache()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Test for AttributeAccessInterface cache utility",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributeAccessInterfaceCache)
