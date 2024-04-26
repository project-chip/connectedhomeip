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

static void RpcActionsTaskCallback(System::Layer * systemLayer, void * data)
{
//printf("\033[41m %s , %d, endpointId=%d, clusterId=%d \033[0m \n", __func__, __LINE__, queue->endpointId, queue->clusterId);

//    struct ActionsDelegate * delegate = RpcFindActionsDelegate(queue->clusterId);
//    if ( nullptr == delegate ) {
        // TBD: Error cluster not registered
//        return;
//    }

    // TBD: insert the queue t ActionHandler's queue
//    delete queue;
}

bool ChefRpcActionsWorker::publishAction(chip::rpc::ActionTask task)
{
    bool kickTimer = queue.size() == 0;

    queue.push(task);

    if (kickTimer) {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(10), RpcActionsTaskCallback, this);
    }
    return true;
}

struct ActionsDelegate * RpcFindActionsDelegate(ClusterId clusterId)
{
    if (gActionsDelegateMap.find(clusterId) != gActionsDelegateMap.end()) {
        return gActionsDelegateMap[clusterId];
    }

    return nullptr;
}


void ActionsDelegate::RegisterRpcActionsDelegate(ClusterId clusterId, ActionsDelegate * delegate)
{
    if ( nullptr == RpcFindActionsDelegate(clusterId) ) {
        gActionsDelegateMap[clusterId] = delegate;
        return;
    }
        
    // TBD: print already registered
}
