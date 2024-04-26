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

// #if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
// #include "../../common/pigweed/rpc_services/Actions.h"
// #endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

#include <vector>
#include <queue>
 
namespace chip {
namespace rpc {

enum class ActionType : uint8_t
{
  WRITE_ATTRIBUTE         = 0x00, // Write an cluster Attribute
  RUN_COMMAND             = 0x01, // Run a cluster Command
  EMIT_EVENT              = 0x02, // Emit a cluster Events
};

struct ActionTask {
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    enum ActionType type;       // Aligned with Storage buf
    uint32_t delayMs;
    uint32_t actionId;
    std::vector<uint32_t> args;
    ActionTask(chip::EndpointId e, chip::ClusterId c,
           ActionType t, uint32_t d, uint32_t i, std::vector<uint32_t> a): endpointId(e), clusterId(c), type(t), delayMs(d), actionId(i), args(a) {}; 
    ~ActionTask() {};
};

class ActionsSubscriber {
public:
    ActionsSubscriber() = default;
    virtual ~ActionsSubscriber() = default;
    virtual bool publishAction(ActionTask task) = 0;
private:
};

void RegisterActionsSubscriber(ActionsSubscriber * subscriber);

int Init(uint16_t rpcServerPort);

} // namespace rpc
} // namespace chip
