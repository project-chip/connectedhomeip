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
#include <queue>
#include <utility>
#include <vector>

#include "Rpc.h"

namespace chip {
namespace app {

class ActionsDelegate
{
public:
    ActionsDelegate(ClusterId clusterId) : mClusterId(clusterId){};

    virtual ~ActionsDelegate() = default;

    virtual void AttributeWriteHandler(chip::EndpointId endpointId, chip::AttributeId attributeId, std::vector<uint32_t> args) = 0;
    virtual void CommandHandler(chip::EndpointId endpointId, chip::CommandId commandId, std::vector<uint32_t> args)            = 0;
    virtual void EventHandler(chip::EndpointId endpointId, chip::EventId eventId, std::vector<uint32_t> args)                  = 0;

protected:
    ClusterId mClusterId;
};

struct ActionTask
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::rpc::ActionType type; // Aligned with Storage buf
    uint32_t delayMs;
    uint32_t actionId;
    std::vector<uint32_t> args;
    ActionTask(chip::EndpointId endpoint, chip::ClusterId cluster, chip::rpc::ActionType actionType, uint32_t delay, uint32_t id,
               std::vector<uint32_t> arg) :
        endpointId(endpoint),
        clusterId(cluster), type(actionType), delayMs(delay), actionId(id), args(arg){};
    ~ActionTask(){};
};

class ChefRpcActionsWorker
{
public:
    static ChefRpcActionsWorker & Instance();

    ChefRpcActionsWorker();

    bool EnqueueAction(ActionTask task);
    void ProcessActionQueue();
    void RegisterRpcActionsDelegate(ClusterId clusterId, ActionsDelegate * delegate);

private:
    std::queue<ActionTask> queue;
};

} // namespace app
} // namespace chip
