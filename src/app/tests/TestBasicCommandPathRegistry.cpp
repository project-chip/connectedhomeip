/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/CommandPathRegistry.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace TestBasicCommandPathRegistry {
namespace {

size_t constexpr kQuickTestSize = 10;

} // namespace

TEST(TestBasicCommandPathRegistry, TestAddingSameConcretePath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    ConcreteCommandPath concretePath(0, 0, 0);
    std::optional<uint16_t> commandRef;
    uint16_t commandRefValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize && err == CHIP_NO_ERROR; idx++)
    {
        commandRef.emplace(commandRefValue);
        commandRefValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    EXPECT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);
    EXPECT_EQ(basicCommandPathRegistry.Count(), 1u);
}

TEST(TestBasicCommandPathRegistry, TestAddingSameCommandRef)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    std::optional<uint16_t> commandRef;
    commandRef.emplace(0);

    uint16_t endpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize && err == CHIP_NO_ERROR; idx++)
    {
        ConcreteCommandPath concretePath(endpointValue, 0, 0);
        endpointValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    EXPECT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);
    EXPECT_EQ(basicCommandPathRegistry.Count(), 1u);
}

TEST(TestBasicCommandPathRegistry, TestAddingMaxNumberOfEntries)
{
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    std::optional<uint16_t> commandRef;
    uint16_t commandRefAndEndpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize; idx++)
    {
        ConcreteCommandPath concretePath(commandRefAndEndpointValue, 0, 0);
        commandRef.emplace(commandRefAndEndpointValue);
        commandRefAndEndpointValue++;
        ASSERT_EQ(basicCommandPathRegistry.Add(concretePath, commandRef), CHIP_NO_ERROR);
    }

    EXPECT_EQ(basicCommandPathRegistry.Count(), kQuickTestSize);
}

TEST(TestBasicCommandPathRegistry, TestAddingTooManyEntries)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;
    size_t maxPlusOne = kQuickTestSize + 1;

    std::optional<uint16_t> commandRef;
    uint16_t commandRefAndEndpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < maxPlusOne && err == CHIP_NO_ERROR; idx++)
    {
        ConcreteCommandPath concretePath(commandRefAndEndpointValue, 0, 0);
        commandRef.emplace(commandRefAndEndpointValue);
        commandRefAndEndpointValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    EXPECT_EQ(err, CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(basicCommandPathRegistry.Count(), kQuickTestSize);
}

} // namespace TestBasicCommandPathRegistry
} // namespace app
} // namespace chip
