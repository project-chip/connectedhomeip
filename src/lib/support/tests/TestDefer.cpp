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

#include <memory>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

namespace {

TEST(TestDefer, TestDeferUsage)
{
    bool deferred = false;
    {
        auto deferredFunction = MakeDefer([&]() { deferred = true; });
        EXPECT_FALSE(deferred);
    }
    EXPECT_TRUE(deferred);

    deferred = false;
    {
        std::unique_ptr<int> movable;
        auto deferredFunction = MakeDefer([movable = std::move(movable), &deferred]() { deferred = true; });
    }
    EXPECT_TRUE(deferred);
}

} // namespace
