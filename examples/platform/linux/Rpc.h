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

#pragma once

#include <queue>
#include <vector>

namespace chip {
namespace rpc {

enum class ActionType : uint8_t
{
    WRITE_ATTRIBUTE = 0x00, // Write an cluster Attribute
    RUN_COMMAND     = 0x01, // Run a cluster Command
    EMIT_EVENT      = 0x02, // Emit a cluster Events
};

using RpcActionsSubscribeCallback = bool (*)(EndpointId endpointId, ClusterId clusterId, uint8_t type, uint32_t delayMs,
                                             uint32_t actionId, std::vector<uint32_t> args);

void SubscribeActions(RpcActionsSubscribeCallback subscriber);

int Init(uint16_t rpcServerPort);

} // namespace rpc
} // namespace chip
