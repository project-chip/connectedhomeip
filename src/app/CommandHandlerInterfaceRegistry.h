/**
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <app/CommandHandlerInterface.h>

namespace chip {
namespace app {

/// Keeps track of a list of registered command handler interfaces
///
/// NOTE: command handler interface objects are IntrusiveList elements (i.e.
///       their pointers are contained within). As a result, a command handler
///       may only ever be part of a single registry.
class CommandHandlerInterfaceRegistry
{
public:
    /// Remove the entire linked list of handlers
    void UnregisterAllHandlers();

    /// Add a new handler to the list of registered command handlers
    ///
    /// At most one command handler can exist for a given endpoint/cluster combination. Trying
    /// to register conflicting handlers will result in a `CHIP_ERROR_INCORRECT_STATE` error.
    CHIP_ERROR RegisterCommandHandler(CommandHandlerInterface * handler);

    /// Unregister all commandHandlers that `MatchesEndpoint` for the given endpointId.
    void UnregisterAllCommandHandlersForEndpoint(EndpointId endpointId);

    /// Unregister a single handler.
    ///
    /// If the handler is not registered, a `CHIP_ERROR_KEY_NOT_FOUND` is returned.
    CHIP_ERROR UnregisterCommandHandler(CommandHandlerInterface * handler);

    /// Find the command handler for the given endpoint/cluster combination or return
    /// nullptr if no such command handler exists.
    CommandHandlerInterface * GetCommandHandler(EndpointId endpointId, ClusterId clusterId);

    /// A global instance of a command handler registry
    static CommandHandlerInterfaceRegistry & Instance();

private:
    CommandHandlerInterface * mCommandHandlerList = nullptr;
};

} // namespace app
} // namespace chip
