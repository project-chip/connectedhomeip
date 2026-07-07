/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/api/allocator/ConsecutiveIdAllocator.h>
#include <device/api/allocator/DynamicIdAllocator.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

TEST(TestIdAllocator, ConsecutiveAllocator)
{
    ConsecutiveIdAllocator allocator(10);

    EXPECT_EQ(allocator.Allocate(), 10);
    EXPECT_EQ(allocator.Allocate(), 11);
    EXPECT_EQ(allocator.Allocate(), 12);
}

TEST(TestIdAllocator, DynamicAllocator_SimpleAllocation)
{
    DynamicIdAllocator allocator;

    EXPECT_EQ(allocator.Allocate(), 1);
    EXPECT_EQ(allocator.Allocate(), 2);
    EXPECT_EQ(allocator.Allocate(), 3);
}

TEST(TestIdAllocator, DynamicAllocator_SkipReserved)
{
    DynamicIdAllocator allocator({ 2, 4 });

    EXPECT_EQ(allocator.Allocate(), 1);
    EXPECT_EQ(allocator.Allocate(), 3);
    EXPECT_EQ(allocator.Allocate(), 5);
}

TEST(TestIdAllocator, DynamicAllocator_ForceNext)
{
    DynamicIdAllocator allocator({ 10 });

    allocator.ForceNext(10);
    EXPECT_EQ(allocator.Allocate(), 10);
    EXPECT_EQ(allocator.Allocate(), 11);
}
