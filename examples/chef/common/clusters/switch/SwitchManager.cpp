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

#include "chef-rpc-actions-worker.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

using namespace chip::rpc;
using namespace chip::app;

class SwitchActionsDelegate: public chip::app::ActionsDelegate
{
public:
    SwitchActionsDelegate(EndpointId endpointId, ClusterId clusterId, SwitchEventHandler *eventHandler): ActionsDelegate(endpointId, clusterId), mEventHandler(eventHandler){};
    ~SwitchActionsDelegate() override {};

    void AttributeWriteHandler(chip::AttributeId attributeId, std::vector<uint32_t>args) override;
    void EventHandler(chip::EventId eventId, std::vector<uint32_t>args) override;


private:
    SwitchEventHandler *mEventHandler;
};

void SwitchActionsDelegate::AttributeWriteHandler(chip::AttributeId attributeId, std::vector<uint32_t>args)
{

}

void SwitchActionsDelegate::EventHandler(chip::EventId eventId, std::vector<uint32_t>args)
{
    switch (eventId) {
    case Events::SwitchLatched::Id:
        {
            uint8_t newPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnSwitchLatched(newPosition);
        }
        break;
    default:
        break;
    }
};

void emberAfSwitchClusterInitCallback(EndpointId endpointId)
{
    ChipLogProgress(Zcl, "Chef: emberAfSwitchClusterInitCallback");
printf("\033[44m %s, %d, Switch::ID=%u \033[0m \n", __func__, __LINE__, Switch::Id);
    ActionsDelegate::RegisterRpcActionsDelegate(Clusters::Switch::Id, new SwitchActionsDelegate(endpointId, Clusters::Switch::Id, new SwitchEventHandler(endpointId)));
}


