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

#include "app/util/attribute-storage.h"
#include "actions_service/actions_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Actions final : public pw_rpc::nanopb::Actions::Service<Actions>
{
public:
    enum class Type: uint8_t {
        Attribute = 0,
        Command = 1,
        Event = 2,
    };

    ::pw::Status Set( const chip_rpc_ActionsRequest & request, ::pw_protobuf_Empty & response)
    {
        DeviceLayer::StackLock lock;
        ChipLogError(NotSpecified," request.endpoint_id=%d, request.cluster_id=%d", request.endpoint_id, request.cluster_id);

        for (int i = 0; i < request.actions_count; i++) {
            chip_rpc_Action action = request.actions[i];
            std::vector<uint32_t> args;
            if (action.has_arg1) args.push_back(action.arg1);
            if (action.has_arg2) args.push_back(action.arg2);
            if (action.has_arg3) args.push_back(action.arg3);
 
            mActionsSubscribeCallback(request.endpoint_id, request.cluster_id, static_cast<uint8_t>(action.type), action.delayMs, action.actionId, args);
        } 

        return pw::OkStatus();
    }

    using RpcActionsSubscribeCallback = bool (*)(EndpointId endpointId, ClusterId clusterId, uint8_t type, uint32_t delayMs, uint32_t actionId, std::vector<uint32_t> args); 

    void SubscribeActions(RpcActionsSubscribeCallback subscriber) { mActionsSubscribeCallback = subscriber; };

private:
    RpcActionsSubscribeCallback mActionsSubscribeCallback;

};

} // namespace rpc
} // namespace chip
