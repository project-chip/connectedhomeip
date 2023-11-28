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
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace chip {
namespace app {
namespace TestBasicCommandPathRegistry {
namespace {

size_t constexpr kQuickTestSize = 10;

} // namespace

void TestAddingSameConcretePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    ConcreteCommandPath concretePath(0, 0, 0);
    Optional<uint16_t> commandRef;
    uint16_t commandRefValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize && err == CHIP_NO_ERROR; idx++)
    {
        commandRef.SetValue(commandRefValue);
        commandRefValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_DUPLICATE_KEY_ID);
    NL_TEST_ASSERT(apSuite, basicCommandPathRegistry.Count() == 1);
}

void TestAddingSameCommandRef(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    Optional<uint16_t> commandRef;
    commandRef.SetValue(0);

    uint16_t endpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize && err == CHIP_NO_ERROR; idx++)
    {
        ConcreteCommandPath concretePath(endpointValue, 0, 0);
        endpointValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_DUPLICATE_KEY_ID);
    NL_TEST_ASSERT(apSuite, basicCommandPathRegistry.Count() == 1);
}

void TestAddingMaxNumberOfEntries(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;

    Optional<uint16_t> commandRef;
    uint16_t commandRefAndEndpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < kQuickTestSize && err == CHIP_NO_ERROR; idx++)
    {
        ConcreteCommandPath concretePath(commandRefAndEndpointValue, 0, 0);
        commandRef.SetValue(commandRefAndEndpointValue);
        commandRefAndEndpointValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, basicCommandPathRegistry.Count() == kQuickTestSize);
}

void TestAddingTooManyEntries(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BasicCommandPathRegistry<kQuickTestSize> basicCommandPathRegistry;
    size_t maxPlusOne = kQuickTestSize + 1;

    Optional<uint16_t> commandRef;
    uint16_t commandRefAndEndpointValue = 0;

    size_t idx = 0;
    for (idx = 0; idx < maxPlusOne && err == CHIP_NO_ERROR; idx++)
    {
        ConcreteCommandPath concretePath(commandRefAndEndpointValue, 0, 0);
        commandRef.SetValue(commandRefAndEndpointValue);
        commandRefAndEndpointValue++;
        err = basicCommandPathRegistry.Add(concretePath, commandRef);
    }

    NL_TEST_ASSERT(apSuite, err == CHIP_ERROR_NO_MEMORY);
    NL_TEST_ASSERT(apSuite, basicCommandPathRegistry.Count() == kQuickTestSize);
}

} // namespace TestBasicCommandPathRegistry
} // namespace app
} // namespace chip

namespace {
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestAddingSameConcretePath", chip::app::TestBasicCommandPathRegistry::TestAddingSameConcretePath),
    NL_TEST_DEF("TestAddingSameCommandRef", chip::app::TestBasicCommandPathRegistry::TestAddingSameCommandRef),
    NL_TEST_DEF("TestAddingMaxNumberOfEntries", chip::app::TestBasicCommandPathRegistry::TestAddingMaxNumberOfEntries),
    NL_TEST_DEF("TestAddingTooManyEntries", chip::app::TestBasicCommandPathRegistry::TestAddingTooManyEntries),

    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestBasicCommandPathRegistry()
{
    nlTestSuite theSuite = { "CommandPathRegistry", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBasicCommandPathRegistry)
