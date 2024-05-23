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
    using CacheResult = AttributeAccessInterfaceCache::CacheResult;

    int data1 = 1;
    int data2 = 2;

    // We alias the pointers to given locations to avoid needing to implement anything
    // since the AttributeAccessInterfaceCache only deals in pointers, and never calls
    // the API itself.
    AttributeAccessInterface * accessor1 = reinterpret_cast<AttributeAccessInterface *>(&data1);
    AttributeAccessInterface * accessor2 = reinterpret_cast<AttributeAccessInterface *>(&data2);

    AttributeAccessInterfaceCache cache;

    // Cache can keep track of at least 1 entry,
    AttributeAccessInterface * entry = nullptr;

    NL_TEST_ASSERT(inSuite, cache.Get(1, 1, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, entry == nullptr);
    cache.MarkUsed(1, 1, accessor1);

    NL_TEST_ASSERT(inSuite, cache.Get(1, 1, &entry) == CacheResult::kDefinitelyUsed);
    NL_TEST_ASSERT(inSuite, entry == accessor1);

    entry = nullptr;
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, entry == nullptr);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, entry == nullptr);

    cache.MarkUsed(1, 2, accessor1);

    entry = nullptr;
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, &entry) == CacheResult::kDefinitelyUsed);
    NL_TEST_ASSERT(inSuite, entry == accessor1);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1, &entry) == CacheResult::kCacheMiss);

    cache.MarkUsed(1, 2, accessor2);

    entry = nullptr;
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, &entry) == CacheResult::kDefinitelyUsed);
    NL_TEST_ASSERT(inSuite, entry == accessor2);
    // The following should not crash (e.g. output not used if nullptr).
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, nullptr) == CacheResult::kDefinitelyUsed);

    // Setting used to nullptr == does not mark used.
    cache.MarkUsed(1, 2, nullptr);
    entry = nullptr;
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, entry == nullptr);

    cache.Invalidate();
    NL_TEST_ASSERT(inSuite, cache.Get(1, 1, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, entry == nullptr);
    NL_TEST_ASSERT(inSuite, cache.Get(1, 2, &entry) == CacheResult::kCacheMiss);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 1, &entry) == CacheResult::kCacheMiss);

    // Marking unused works, keeps single entry, and is invalidated when invalidated fully.
    NL_TEST_ASSERT(inSuite, cache.Get(2, 2, nullptr) != CacheResult::kDefinitelyUnused);
    NL_TEST_ASSERT(inSuite, cache.Get(3, 3, nullptr) != CacheResult::kDefinitelyUnused);
    cache.MarkUnused(2, 2);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 2, nullptr) == CacheResult::kDefinitelyUnused);
    NL_TEST_ASSERT(inSuite, cache.Get(3, 3, nullptr) != CacheResult::kDefinitelyUnused);

    cache.MarkUnused(3, 3);
    NL_TEST_ASSERT(inSuite, cache.Get(2, 2, nullptr) != CacheResult::kDefinitelyUnused);
    NL_TEST_ASSERT(inSuite, cache.Get(3, 3, nullptr) == CacheResult::kDefinitelyUnused);

    cache.Invalidate();
    NL_TEST_ASSERT(inSuite, cache.Get(3, 3, nullptr) != CacheResult::kDefinitelyUnused);
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
