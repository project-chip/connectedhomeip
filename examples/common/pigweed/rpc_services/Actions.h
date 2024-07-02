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

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "actions_service/actions_service.rpc.pb.h"
#include "app/util/attribute-storage.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Actions final : public pw_rpc::nanopb::Actions::Service<Actions>
{
    /*
     * RPC Actions Service is a debugging interface for writting Attributes, emitting Events, or
     * running Commands batchwisely. Each action will be execute one by one according to the
     * delayed time in ms.
     *
     * An Action is a abstract type of Attribute, Command, Event
     *  type                : this defines the action is either a Attribute, Command, or Event
     *  delayMs             : this define the relative delayed time in ms after last action
     *  actionId            : this defines the ID of Attribute, Command or Event
     *  arg1, arg2, arg3    : it has up to 3 optional arguments which could be mapped to Attribute/Command/Event aruments
     *
     * An Action Request is composed of a batch of Actions
     *
     * The usage of Actions Service:
     *      1. Create an Action Request by protos.chip.rpc.ActionsWrite
     *      2. Append Actions one by one to the action request
     *      3. After all action added to the request, call rpcs.chip.rpc.Actions.Set to execute that
     *
     * e.g. In rpc_console:
     *
     *      # Init a RPC Action request which is for endpoint 1 and Switch Cluster (0x003B)
     *      In [1]: request=protos.chip.rpc.ActionsWrite(endpoint_id=1, cluster_id=int("0x003B", 16))
     *
     *      # Add an action to write attribute id 0x1 (CurrentPosition) with data 2 after 1000 ms (meaning right away)
     *      In [2]: request.actions.append(protos.chip.rpc.Action(type=protos.chip.rpc.ActionType.WRITE_ATTRIBUTE, delayMs=1000,
     * actionId=1, arg1=2))
     *
     *      # Define the follow action to emit an event id 0x1 (InitialPress) with data 2 (NewPosition) after 1000 ms
     *      In [3]: message.actions.append(protos.chip.rpc.Action(type=protos.chip.rpc.ActionType.EMIT_EVENT, delayMs=1000,
     * actionId=1, arg1=2))
     *
     *      # Define the follow action to emit an event id 0x2 (LongPress) with data 2 (NewPosition) after 2000 ms
     *      In [4]: message.actions.append(protos.chip.rpc.Action(type=protos.chip.rpc.ActionType.EMIT_EVENT, delayMs=2000,
     * actionId=2, arg1=2))
     *
     *      # Define an action to write attribute id 0x1 (CurrentPosition) with data 0 after 2000 ms (meaning button bouncing back)
     *      In [5]: message.actions.append(protos.chip.rpc.Action(type=protos.chip.rpc.ActionType.WRITE_ATTRIBUTE, delayMs=2000,
     * actionId=1, arg1=0))
     *
     *      # Define the follow action to emit an event id 0x4 (LongRelease) with data 2 (PreviousPosition) after 1000 ms
     *      In [6]: message.actions.append(protos.chip.rpc.Action(type=protos.chip.rpc.ActionType.EMIT_EVENT, delayMs=1000,
     * actionId=4, arg1=2))
     *
     *      # Set the actions to device
     *      In [7]: rpcs.chip.rpc.Actions.Set(message, pw_rpc_timeout_s=10000)
     *
     */
public:
    enum class Type : uint8_t
    {
        Attribute = 0,
        Command   = 1,
        Event     = 2,
    };

    ::pw::Status Set(const chip_rpc_ActionsRequest & request, ::pw_protobuf_Empty & response)

    {
        DeviceLayer::StackLock lock;
        ChipLogProgress(NotSpecified, " request.endpoint_id=%d, request.cluster_id=%d", request.endpoint_id, request.cluster_id);

        for (int i = 0; i < request.actions_count; i++)
        {
            chip_rpc_Action action = request.actions[i];
            std::vector<uint32_t> args;
            if (action.has_arg1)
                args.push_back(action.arg1);
            if (action.has_arg2)
                args.push_back(action.arg2);
            if (action.has_arg3)
                args.push_back(action.arg3);

            mActionsSubscribeCallback(request.endpoint_id, request.cluster_id, static_cast<uint8_t>(action.type), action.delayMs,
                                      action.actionId, args);
        }

        return pw::OkStatus();
    }

    using RpcActionsSubscribeCallback = bool (*)(EndpointId endpointId, ClusterId clusterId, uint8_t type, uint32_t delayMs,
                                                 uint32_t actionId, std::vector<uint32_t> args);

    void SubscribeActions(RpcActionsSubscribeCallback subscriber) { mActionsSubscribeCallback = subscriber; };

private:
    RpcActionsSubscribeCallback mActionsSubscribeCallback;
};

} // namespace rpc
} // namespace chip
