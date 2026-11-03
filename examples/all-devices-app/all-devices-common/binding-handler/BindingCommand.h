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

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <messaging/ExchangeMgr.h>
#include <transport/Session.h>

#include <variant>

namespace chip {
namespace app {

/**
 * Destination of a unicast binding, resolved once the CASE session to the peer is established.
 */
struct UnicastBindingTarget
{
    /** Exchange manager of the peer's session. */
    Messaging::ExchangeManager & exchangeManager;
    /** Secure session to the bound peer node. */
    const SessionHandle & session;
    /** Endpoint on the bound peer node. */
    EndpointId endpoint;
};

/**
 * Destination of a multicast (group) binding.
 */
struct GroupBindingTarget
{
    /** Exchange manager used to send group messages. */
    Messaging::ExchangeManager & exchangeManager;
    FabricIndex fabricIndex;
    GroupId groupId;
};

/**
 * Where BindingCommand::Dispatch must send its command. Each alternative carries everything needed to
 * send to that kind of destination.
 *
 * The references in a target are only valid for the duration of the BindingCommand::Dispatch call.
 */
using BindingTarget = std::variant<UnicastBindingTarget, GroupBindingTarget>;

/**
 * A command that a device sends to the peers bound to one of its endpoints.
 *
 * Instances are handed to BindingHandler::Dispatch, which owns them until every matching binding has
 * been served. Unicast bindings are served once the session to the peer is established, which can be
 * long after Dispatch returned, so implementations must own all the data they send (no spans or lists
 * pointing to caller-owned buffers).
 */
class BindingCommand
{
public:
    virtual ~BindingCommand() = default;

    /**
     * Cluster of the command. Only bindings on this cluster (or without a cluster) are served.
     */
    virtual ClusterId GetClusterId() const = 0;

    /**
     * Sends the command to @p target. Called once per matching binding, on the Matter event loop.
     */
    virtual void Dispatch(const BindingTarget & target) = 0;
};

} // namespace app
} // namespace chip
