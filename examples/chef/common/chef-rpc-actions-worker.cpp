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
#include <map>
#include <platform/CHIPDeviceLayer.h>

using chip::app::DataModel::Nullable;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::rpc;

static std::map<ClusterId, ActionsDelegate *> gActionsDelegateMap{};

ActionsDelegate * RpcFindActionsDelegate(ClusterId clusterId)
{
    if (gActionsDelegateMap.find(clusterId) != gActionsDelegateMap.end())
    {
        return gActionsDelegateMap[clusterId];
    }

    return nullptr;
}

static void RpcActionsTaskCallback(System::Layer * systemLayer, void * data)
{
    ChefRpcActionsWorker * worker = (ChefRpcActionsWorker *) data;

    worker->ProcessActionQueue();
}

bool ChefRpcActionsCallback(EndpointId endpointId, ClusterId clusterId, uint8_t type, uint32_t delayMs, uint32_t actionId,
                            std::vector<uint32_t> args)
{
    ActionTask task(endpointId, clusterId, static_cast<ActionType>(type), delayMs, actionId, args);

    return ChefRpcActionsWorker::Instance().EnqueueAction(task);
}

bool ChefRpcActionsWorker::EnqueueAction(ActionTask task)
{
    bool kickTimer = false;

    if (queue.empty())
    {
        kickTimer = true; // kick timer when the first task is adding to the queue
    }

    queue.push(task);

    if (kickTimer)
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(task.delayMs), RpcActionsTaskCallback, this);
    }
    return true;
}

void ChefRpcActionsWorker::ProcessActionQueue()
{
    // Dequeue the first item
    ActionTask task = queue.front();
    queue.pop();

    ActionsDelegate * delegate = RpcFindActionsDelegate(task.clusterId);
    if (nullptr == delegate)
    {
        ChipLogError(NotSpecified,
                     "Cannot run action due to not finding delegate: endpointId=%d, clusterId=%04lx, attributeId=%04lx",
                     task.endpointId, static_cast<unsigned long>(task.clusterId), static_cast<unsigned long>(task.actionId));
        return;
    }

    ActionType type = static_cast<ActionType>(task.type);

    switch (type)
    {
    case ActionType::WRITE_ATTRIBUTE: {
        ChipLogProgress(NotSpecified, "Writing Attribute: endpointId=%d, clusterId=%04lx, attributeId=%04lx, args.size=%lu",
                        task.endpointId, static_cast<unsigned long>(task.clusterId), static_cast<unsigned long>(task.actionId),
                        static_cast<unsigned long>(task.args.size()));
        delegate->AttributeWriteHandler(task.endpointId, static_cast<chip::AttributeId>(task.actionId), task.args);
    }
    break;
    case ActionType::RUN_COMMAND: {
        ChipLogProgress(NotSpecified, "Running Command: endpointId=%d, clusterId=%04lx, commandId=%04lx, args.size=%lu",
                        task.endpointId, static_cast<unsigned long>(task.clusterId), static_cast<unsigned long>(task.actionId),
                        static_cast<unsigned long>(task.args.size()));
        delegate->CommandHandler(task.endpointId, static_cast<chip::CommandId>(task.actionId), task.args);
    }
    break;
    case ActionType::EMIT_EVENT: {
        ChipLogProgress(NotSpecified, "Emitting Event: endpointId=%d, clusterId=%04lx, eventIdId=%04lx, args.size=%lu",
                        task.endpointId, static_cast<unsigned long>(task.clusterId), static_cast<unsigned long>(task.actionId),
                        static_cast<unsigned long>(task.args.size()));
        delegate->EventHandler(task.endpointId, static_cast<chip::EventId>(task.actionId), task.args);
    }
    break;
    default:
        break;
    }

    if (queue.empty())
    {
        // Return due to no more actions in queue
        return;
    }

    // Run next action
    task = queue.front();
    ChipLogProgress(NotSpecified, "StartTimer: endpointId=%d, clusterId=%04lx, eventIdId=%04lx, task.delyMs=%lu", task.endpointId,
                    static_cast<unsigned long>(task.clusterId), static_cast<unsigned long>(task.actionId),
                    static_cast<unsigned long>(task.delayMs));
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(task.delayMs), RpcActionsTaskCallback, this);
}

void ChefRpcActionsWorker::RegisterRpcActionsDelegate(ClusterId clusterId, ActionsDelegate * delegate)
{
    // Register by cluster
    if (nullptr == RpcFindActionsDelegate(clusterId))
    {
        gActionsDelegateMap[clusterId] = delegate;
        return;
    }
}

ChefRpcActionsWorker::ChefRpcActionsWorker()
{
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::SubscribeActions(ChefRpcActionsCallback);
#endif
}

static ChefRpcActionsWorker instance;

ChefRpcActionsWorker & ChefRpcActionsWorker::Instance()
{
    return instance;
}
