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

#include "LightSwitch.h"
#include "AppEvent.h"
#include "BindingHandler.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void LightSwitch::Init()
{
    BindingHandler::GetInstance().Init();
}

void LightSwitch::InitiateActionSwitch(chip::EndpointId endpointId, uint8_t action)
{
    BindingTable & bindingTable = BindingTable::GetInstance();
    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }

    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId = endpointId;
        data->ClusterId  = Clusters::OnOff::Id;
        data->IsGroup    = false;

        for (auto & entry : bindingTable)
        {
            if (endpointId == entry.local && MATTER_MULTICAST_BINDING == entry.type)
            {
                data->IsGroup = true;
                break;
            }
        }

        switch (action)
        {
        case Action::Toggle:
            data->CommandId = Clusters::OnOff::Commands::Toggle::Id;
            break;
        case Action::On:
            data->CommandId = Clusters::OnOff::Commands::On::Id;
            break;
        case Action::Off:
            data->CommandId = Clusters::OnOff::Commands::Off::Id;
            break;
        default:
            Platform::Delete(data);
            return;
        }
        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::GenericSwitchInitialPress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Press moves Position from 0 (idle) to 1 (press)
        uint8_t newPosition = 1;

        Clusters::Switch::Attributes::CurrentPosition::Set(kLightGenericSwitchEndpointId, newPosition);
        // InitialPress event takes newPosition as event data
        Clusters::SwitchServer::Instance().OnInitialPress(kLightGenericSwitchEndpointId, newPosition);
    });
}

void LightSwitch::GenericSwitchReleasePress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Release moves Position from 1 (press) to 0 (idle)
        uint8_t previousPosition = 1;
        uint8_t newPosition      = 0;

        Clusters::Switch::Attributes::CurrentPosition::Set(kLightGenericSwitchEndpointId, newPosition);
        // ShortRelease event takes previousPosition as event data
        Clusters::SwitchServer::Instance().OnShortRelease(kLightGenericSwitchEndpointId, previousPosition);
    });
}

#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
void LightSwitch::SubscribeRequestForOneNode(chip::EndpointId endpointId)
{
    BindingTable & bindingTable = BindingTable::GetInstance();

    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }
    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction2, endpointId);
}

void LightSwitch::ShutdownSubscribeRequestForOneNode(chip::EndpointId endpointId)
{
    BindingTable & bindingTable = BindingTable::GetInstance();
    for (auto & entry : bindingTable)
    {
        ChipLogError(DeviceLayer, "entry.local %d", entry.local);
        if (endpointId == entry.local)
        {
            BindingHandler::SubscribeCommandData * data = Platform::New<BindingHandler::SubscribeCommandData>();
            data->localEndpointId                       = endpointId;
            data->nodeId                                = entry.nodeId;
            data->fabricIndex                           = entry.fabricIndex;

            DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction3, reinterpret_cast<intptr_t>(data));
            break;
        }
    }
}
#endif
