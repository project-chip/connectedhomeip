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
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

TEST(TestAttributeAccessInterfaceCache, TestBasicLifecycle)
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

    EXPECT_EQ(cache.Get(1, 1, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(entry, nullptr);
    cache.MarkUsed(1, 1, accessor1);

    EXPECT_EQ(cache.Get(1, 1, &entry), CacheResult::kDefinitelyUsed);
    EXPECT_EQ(entry, accessor1);

    entry = nullptr;
    EXPECT_EQ(cache.Get(1, 2, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(entry, nullptr);
    EXPECT_EQ(cache.Get(2, 1, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(entry, nullptr);

    cache.MarkUsed(1, 2, accessor1);

    entry = nullptr;
    EXPECT_EQ(cache.Get(1, 2, &entry), CacheResult::kDefinitelyUsed);
    EXPECT_EQ(entry, accessor1);
    EXPECT_EQ(cache.Get(2, 1, &entry), CacheResult::kCacheMiss);

    cache.MarkUsed(1, 2, accessor2);

    entry = nullptr;
    EXPECT_EQ(cache.Get(1, 2, &entry), CacheResult::kDefinitelyUsed);
    EXPECT_EQ(entry, accessor2);
    // The following should not crash (e.g. output not used if nullptr).
    EXPECT_EQ(cache.Get(1, 2, nullptr), CacheResult::kDefinitelyUsed);

    // Setting used to nullptr == does not mark used.
    cache.MarkUsed(1, 2, nullptr);
    entry = nullptr;
    EXPECT_EQ(cache.Get(1, 2, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(entry, nullptr);

    cache.Invalidate();
    EXPECT_EQ(cache.Get(1, 1, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(entry, nullptr);
    EXPECT_EQ(cache.Get(1, 2, &entry), CacheResult::kCacheMiss);
    EXPECT_EQ(cache.Get(2, 1, &entry), CacheResult::kCacheMiss);

    // Marking unused works, keeps single entry, and is invalidated when invalidated fully.
    EXPECT_NE(cache.Get(2, 2, nullptr), CacheResult::kDefinitelyUnused);
    EXPECT_NE(cache.Get(3, 3, nullptr), CacheResult::kDefinitelyUnused);
    cache.MarkUnused(2, 2);
    EXPECT_EQ(cache.Get(2, 2, nullptr), CacheResult::kDefinitelyUnused);
    EXPECT_NE(cache.Get(3, 3, nullptr), CacheResult::kDefinitelyUnused);

    cache.MarkUnused(3, 3);
    EXPECT_NE(cache.Get(2, 2, nullptr), CacheResult::kDefinitelyUnused);
    EXPECT_EQ(cache.Get(3, 3, nullptr), CacheResult::kDefinitelyUnused);

    cache.Invalidate();
    EXPECT_NE(cache.Get(3, 3, nullptr), CacheResult::kDefinitelyUnused);
}
} // namespace
