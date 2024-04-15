/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/att-storage.h>
#include <platform/PlatformManager.h>
#include "SwitchEventHandler.h"

#include "Rpc.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

using namespace chip::rpc;

static std::map<int, SwitchEventHandler *> gSwitchEventHandlers{};

SwitchEventHandler * GetSwitchEventHandler(EndpointId endpointId)
{
    if (gSwitchEventHandlers.find(endpointId) == gSwitchEventHandlers.end()) {
        return nullptr;
    }

    return gSwitchEventHandlers[endpointId];
}

void SwitchManagerEventHandler(intptr_t ctx, struct EventsRequest * data)
{
    SwitchEventHandler * handler = GetSwitchEventHandler(data->endpointId);

    if (nullptr == handler) {
        // TODO: Error
        return;
    }
    // Parse data->events
    // According to data->events, to dispatch events
}

void emberAfSwitchClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Chef: emberAfSwitchClusterInitCallback");
    gSwitchEventHandlers[endpoint] = new SwitchEventHandler(endpoint);
printf("\033[44m %s, %d, Switch::ID=%u \033[0m \n", __func__, __LINE__, Switch::Id);
    RpcRegisterAppEventsHandler( Switch::Id, SwitchManagerEventHandler, reinterpret_cast<intptr_t>(&gSwitchEventHandlers));
}


