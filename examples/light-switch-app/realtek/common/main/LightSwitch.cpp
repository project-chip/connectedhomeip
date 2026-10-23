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
#include "BindingHandler.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>

#define CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM 254

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void LightSwitch::Init()
{
    BindingHandler::GetInstance().Init();
}

void LightSwitch::InitiateActionSwitch(chip::EndpointId endpointId, uint8_t action)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();
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
            if (endpointId == entry.local && Binding::MATTER_MULTICAST_BINDING == entry.type)
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
        // ScheduleWork is asynchronous and returns immediately.
        // Failure means the work couldn't be queued, but there's nothing
        // we can do about it here, so the return value can be safely ignored.
        RETURN_SAFELY_IGNORED(
            DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction, reinterpret_cast<intptr_t>(data)));
    }
}

void LightSwitch::DimmerChangeBrightness(chip::EndpointId endpointId, uint8_t brightness)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();
    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }

    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();

    if (data)
    {
        data->EndpointId = endpointId;
        data->CommandId  = Clusters::LevelControl::Commands::MoveToLevel::Id;
        data->ClusterId  = Clusters::LevelControl::Id;
        data->Value   = (brightness > CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM ? CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM : brightness);
        data->IsGroup = false;

        for (auto & entry : bindingTable)
        {
            if (endpointId == entry.local && Binding::MATTER_MULTICAST_BINDING == entry.type)
            {
                data->IsGroup = true;
                break;
            }
        }

        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::ColorChange(chip::EndpointId endpointId, uint16_t colorX, uint16_t colorY)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();
    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }

    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId = endpointId;
        data->CommandId  = Clusters::ColorControl::Commands::MoveToColor::Id;
        data->ClusterId  = Clusters::ColorControl::Id;
        data->ColorXY.x  = colorX;
        data->ColorXY.y  = colorY;
        data->IsGroup    = false;

        for (auto & entry : bindingTable)
        {
            if (endpointId == entry.local && Binding::MATTER_MULTICAST_BINDING == entry.type)
            {
                data->IsGroup = true;
                break;
            }
        }

        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::ColorTemperatureChange(chip::EndpointId endpointId, uint16_t colorTemperatureMireds)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();
    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }

    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId             = endpointId;
        data->CommandId              = Clusters::ColorControl::Commands::MoveToColorTemperature::Id;
        data->ClusterId              = Clusters::ColorControl::Id;
        data->ColorTemperatureMireds = colorTemperatureMireds;
        data->IsGroup                = false;

        for (auto & entry : bindingTable)
        {
            if (endpointId == entry.local && Binding::MATTER_MULTICAST_BINDING == entry.type)
            {
                data->IsGroup = true;
                break;
            }
        }

        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::GenericSwitchInitialPress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Press moves Position from 0 (idle) to 1 (press)
        uint8_t newPosition = 1;

        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(kLightGenericSwitchEndpointId);
        VerifyOrReturn(switchCluster != nullptr);

        CHIP_ERROR status = switchCluster->SetCurrentPosition(newPosition);
        VerifyOrReturn(CHIP_NO_ERROR == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

        RETURN_SAFELY_IGNORED switchCluster->OnInitialPress(newPosition);
    });
}

void LightSwitch::GenericSwitchReleasePress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Release moves Position from 1 (press) to 0 (idle)
        uint8_t previousPosition = 1;
        uint8_t newPosition      = 0;

        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(kLightGenericSwitchEndpointId);
        VerifyOrReturn(switchCluster != nullptr);

        CHIP_ERROR status = switchCluster->SetCurrentPosition(newPosition);
        VerifyOrReturn(CHIP_NO_ERROR == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

        RETURN_SAFELY_IGNORED switchCluster->OnShortRelease(previousPosition);
    });
}

#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
void LightSwitch::SubscribeRequestForOneNode(chip::EndpointId endpointId)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();

    if (!bindingTable.Size())
    {
        ChipLogError(DeviceLayer, "bindingTable empty");
        return;
    }
    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerFunction2, endpointId);
}

void LightSwitch::ShutdownSubscribeRequestForOneNode(chip::EndpointId endpointId)
{
    Binding::Table & bindingTable = Binding::Table::GetInstance();
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
