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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/CommandHandlerInterfaceRegistry.h>

#include <type_traits>

namespace chip {
namespace app {

namespace {

class TestCommandHandlerInterfaceB : public CommandHandlerInterfaceB
{
public:
    TestCommandHandlerInterfaceB(Optional<EndpointId> endpointId, ClusterId clusterId) :
        CommandHandlerInterfaceB(endpointId, clusterId)
    {}

    // Just need this to compile
    void InvokeCommand(HandlerContext & handlerContext) override {}
};

} // anonymous namespace

TEST(TestCommandHandlerInterfaceRegistry, TestRegisterUnregister)
{
    TestCommandHandlerInterfaceB a(Optional<EndpointId>(1), 1);
    TestCommandHandlerInterfaceB b(Optional<EndpointId>(1), 2);
    TestCommandHandlerInterfaceB c(Optional<EndpointId>(2), 1);
    TestCommandHandlerInterfaceB d(NullOptional, 3);

    CommandHandlerInterfaceRegistry registry;
    EXPECT_EQ(registry.RegisterCommandHandler(&a), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&b), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&c), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&d), CHIP_NO_ERROR);

    EXPECT_EQ(registry.GetCommandHandler(1, 1), &a);
    EXPECT_EQ(registry.GetCommandHandler(1, 2), &b);
    EXPECT_EQ(registry.GetCommandHandler(2, 1), &c);
    EXPECT_EQ(registry.GetCommandHandler(1, 3), &d);
    EXPECT_EQ(registry.GetCommandHandler(5, 3), &d);

    EXPECT_EQ(registry.UnregisterCommandHandler(&b), CHIP_NO_ERROR);

    EXPECT_EQ(registry.GetCommandHandler(1, 1), &a);
    EXPECT_EQ(registry.GetCommandHandler(1, 2), nullptr);
    EXPECT_EQ(registry.GetCommandHandler(2, 1), &c);
    EXPECT_EQ(registry.GetCommandHandler(1, 3), &d);
    EXPECT_EQ(registry.GetCommandHandler(5, 3), &d);

    EXPECT_EQ(registry.UnregisterCommandHandler(&b), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST(TestCommandHandlerInterfaceRegistry, TestUnregisterAll)
{
    TestCommandHandlerInterfaceB a(Optional<EndpointId>(1), 1);
    TestCommandHandlerInterfaceB b(Optional<EndpointId>(1), 2);
    TestCommandHandlerInterfaceB c(Optional<EndpointId>(2), 1);
    TestCommandHandlerInterfaceB d(NullOptional, 3);

    CommandHandlerInterfaceRegistry registry;
    EXPECT_EQ(registry.RegisterCommandHandler(&a), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&b), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&c), CHIP_NO_ERROR);
    EXPECT_EQ(registry.RegisterCommandHandler(&d), CHIP_NO_ERROR);

    EXPECT_EQ(registry.GetCommandHandler(1, 1), &a);
    EXPECT_EQ(registry.GetCommandHandler(1, 2), &b);
    EXPECT_EQ(registry.GetCommandHandler(2, 1), &c);
    EXPECT_EQ(registry.GetCommandHandler(1, 3), &d);
    EXPECT_EQ(registry.GetCommandHandler(5, 3), &d);

    registry.UnregisterAllCommandHandlersForEndpoint(1);

    EXPECT_EQ(registry.GetCommandHandler(1, 1), nullptr);
    EXPECT_EQ(registry.GetCommandHandler(1, 2), nullptr);
    EXPECT_EQ(registry.GetCommandHandler(2, 1), &c);
    EXPECT_EQ(registry.GetCommandHandler(1, 3), &d);
    EXPECT_EQ(registry.GetCommandHandler(5, 3), &d);
}

} // namespace app
} // namespace chip
