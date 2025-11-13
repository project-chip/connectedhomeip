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

#include <inttypes.h>
#include <stdint.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/data-model/List.h>

using namespace chip;
using namespace chip::app::DataModel;

namespace {
template <typename T>
void PassListArg(const List<T> &)
{}
} // namespace

TEST(TestList, TestConstructorTypeDeduction)
{
    const uint8_t nums[]      = { 1, 2 };
    const uint8_t otherNums[] = { 1, 2, 3 };

    // These will fail to compile if type deduction is not working correctly.
    PassListArg(List(nums));
    PassListArg(List(nums, 1));

    List a(nums);
    EXPECT_TRUE(a.data_equal(List<const uint8_t>(otherNums, 2)));

    List b(nums, 1);
    EXPECT_TRUE(b.data_equal(List<const uint8_t>(otherNums, 1)));
}
