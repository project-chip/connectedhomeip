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
    ::pw::Status Set( const chip_rpc_ActionsWrite & request, ::pw_protobuf_Empty & response)
    {
printf("\033[41m %s, %d, request.endpoint_id=%d, request.cluster_id=%d \033[0m \n", __func__, __LINE__, request.endpoint_id, request.cluster_id);

        std::queue<Action> actionQueue;
        mActionsSubscriber(request.endpoint_id, request.cluster_id, actionQueue);

        return pw::OkStatus();
    }

    enum class Type: uint8_t {
        Attribute = 0,
        Command = 1,
        Event = 2,
    };

    struct Action {
        Type type;
        uint32_t delayMs;
        uint32_t actionId;
        std::vector<uint32_t> args;
    };

    using RpcActionsSubscriber = void (*)(EndpointId endpointId, ClusterId clusterId, std::queue<Action>); 

    void RegisterActionsSubscriber(RpcActionsSubscriber subscriber) { mActionsSubscriber = subscriber; };

private:
    RpcActionsSubscriber mActionsSubscriber;

};

} // namespace rpc
} // namespace chip
