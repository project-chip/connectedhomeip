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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

#include "Rpc.h"

namespace chip {
namespace app {

class ActionsDelegate
{

public:
    ActionsDelegate(EndpointId endpoint, ClusterId cluster): mEndpointId(endpoint), mClusterId(cluster) { };

    virtual ~ActionsDelegate() = default;

    virtual void AttributeWriteHandler(chip::AttributeId attributeId, std::vector<uint32_t>args) {};
    virtual void CommandHandler(chip::CommandId commandId, std::vector<uint32_t>args) {};
    virtual void EventHandler(chip::EventId eventId, std::vector<uint32_t>args) {};

protected:
    EndpointId mEndpointId;
    ClusterId mClusterId;
};

struct ActionTask {
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::rpc::ActionType type;       // Aligned with Storage buf
    uint32_t delayMs;
    uint32_t actionId;
    std::vector<uint32_t> args;
    ActionTask(chip::EndpointId e, chip::ClusterId c,
           chip::rpc::ActionType t, uint32_t d, uint32_t i, std::vector<uint32_t> a): endpointId(e), clusterId(c), type(t), delayMs(d), actionId(i), args(a) {};
    ~ActionTask() {};
};

class ChefRpcActionsWorker
{
public:
    static ChefRpcActionsWorker& Instance(); 

    ChefRpcActionsWorker();

    bool EnqueueAction(ActionTask task);
    ActionTask PopActionQueue();
    void RegisterRpcActionsDelegate(ClusterId clusterId, ActionsDelegate * delegate);

 private:
    std::queue<ActionTask> queue;
};


} // namespace app
} // namespace chip

