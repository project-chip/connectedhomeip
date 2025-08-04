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

/**
 *    @file
 *      This file implements unit tests for AttributePathParams
 *
 */

#include <app/CommandPathParams.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace TestCommandPathParams {
TEST(TestCommandPathParams, TestSamePath)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    EXPECT_TRUE(commandPathParams1.IsSamePath(commandPathParams2));
}

TEST(TestCommandPathParams, TestDifferentEndpointId)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(6, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    EXPECT_FALSE(commandPathParams1.IsSamePath(commandPathParams2));
}

TEST(TestCommandPathParams, TestDifferentGroupId)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kGroupIdValid);
    CommandPathParams commandPathParams2(1, 6, 3, 4, CommandPathFlags::kGroupIdValid);
    EXPECT_FALSE(commandPathParams1.IsSamePath(commandPathParams2));
}

TEST(TestCommandPathParams, TestDifferentClusterId)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 6, 4, CommandPathFlags::kEndpointIdValid);
    EXPECT_FALSE(commandPathParams1.IsSamePath(commandPathParams2));
}

TEST(TestCommandPathParams, TestDifferentCommandId)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 6, CommandPathFlags::kEndpointIdValid);
    EXPECT_FALSE(commandPathParams1.IsSamePath(commandPathParams2));
}

TEST(TestCommandPathParams, TestDifferentPathFlag)
{
    CommandPathParams commandPathParams1(1, 2, 3, 4, CommandPathFlags::kEndpointIdValid);
    CommandPathParams commandPathParams2(1, 2, 3, 4, CommandPathFlags::kGroupIdValid);
    EXPECT_FALSE(commandPathParams1.IsSamePath(commandPathParams2));
}
} // namespace TestCommandPathParams
} // namespace app
} // namespace chip
