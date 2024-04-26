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

#include "chef-rpc-actions-worker.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceLayer.h>
#include <map>

//#include <Actions.h>

using chip::app::DataModel::Nullable;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::rpc;

static std::map<ClusterId, ActionsDelegate *> gActionsDelegateMap {};  

ActionsDelegate * RpcFindActionsDelegate(ClusterId clusterId)
{
    if (gActionsDelegateMap.find(clusterId) != gActionsDelegateMap.end()) {
        return gActionsDelegateMap[clusterId];
    }

    return nullptr;
}

static void RpcActionsTaskCallback(System::Layer * systemLayer, void * data)
{
    ChefRpcActionsWorker * worker = (ChefRpcActionsWorker *)data;
printf("\033[44m %s , %d \033[0m \n", __func__, __LINE__);

    worker->ProcessActionQueue();
}

bool ChefRpcActionsCallback(EndpointId endpointId, ClusterId clusterId, uint8_t type, uint32_t delayMs, uint32_t actionId, std::vector<uint32_t> args)
{
    ActionTask task(endpointId, clusterId, static_cast<ActionType>(type), delayMs, actionId, args);
    // TBD: Stack lock
    return ChefRpcActionsWorker::Instance().EnqueueAction(task);
}

bool ChefRpcActionsWorker::EnqueueAction(ActionTask task)
{
    bool kickTimer = false;

    if (queue.empty()) {
        queue.push(task);
        kickTimer = true;   // kick timer when the first task is adding to the queue
    }
    if (kickTimer) {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(task.delayMs), RpcActionsTaskCallback, this);
    }
    return true;
}

void ChefRpcActionsWorker::ProcessActionQueue()
{
//    delete queue;
    ActionTask task = queue.front();
    queue.pop();

printf("\033[41m %s , %d, endpointId=%d, clusterId=%d \033[0m \n", __func__, __LINE__, task.endpointId, task.clusterId);

    ActionsDelegate * delegate = RpcFindActionsDelegate(task.clusterId);
    if ( nullptr == delegate ) {
printf("\033[41m %s , %d, Cannot run action due to not finding delegate: endpointId=%d, clusterId=%d \033[0m \n", __func__, __LINE__, task.endpointId, task.clusterId);
    } else {
        ActionType type = static_cast<ActionType>(task.type);

        switch (type) {
        case ActionType::WRITE_ATTRIBUTE:
        {
printf("\033[41m %s , %d, Writing Attribute: %d, args size=%lu \033[0m \n", __func__, __LINE__, task.actionId, task.args.size());
            delegate->AttributeWriteHandler(task.endpointId, static_cast<chip::AttributeId>(task.actionId), task.args);
        }
        break; 
        case ActionType::RUN_COMMAND:
        {
printf("\033[41m %s , %d, Running Command: %d, args size=%lu \033[0m \n", __func__, __LINE__, task.actionId, task.args.size());
            delegate->CommandHandler(task.endpointId, static_cast<chip::CommandId>(task.actionId), task.args);
        }
        break; 
        case ActionType::EMIT_EVENT:
        {
printf("\033[41m %s , %d, Emitting Event: %d, args size=%lu \033[0m \n", __func__, __LINE__, task.actionId, task.args.size());
            delegate->EventHandler(task.endpointId, static_cast<chip::EventId>(task.actionId), task.args);
        }
        break; 
        default:
            break;
        }
    }

    if (queue.empty()) {
        // Return due to no extra queue item to run. 
        return;
    }

    // Run next task
    task = queue.front();
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(task.delayMs), RpcActionsTaskCallback, this);
}

void ChefRpcActionsWorker::RegisterRpcActionsDelegate(ClusterId clusterId, ActionsDelegate * delegate)
{
    if ( nullptr == RpcFindActionsDelegate(clusterId) ) {
        gActionsDelegateMap[clusterId] = delegate;
        return;
    }
    // TBD: print already registered
}

ChefRpcActionsWorker::ChefRpcActionsWorker()
{
    chip::rpc::SubscribeActions(ChefRpcActionsCallback);
}

static ChefRpcActionsWorker instance;

ChefRpcActionsWorker & ChefRpcActionsWorker::Instance()
{
    return instance;
}

