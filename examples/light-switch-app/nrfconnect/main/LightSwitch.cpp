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

#include <app/server/Server.h>
#include <app/util/binding-table.h>
#include <controller/InvokeInteraction.h>

#include <app/clusters/switch-server/switch-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void LightSwitch::Init(chip::EndpointId aLightDimmerSwitchEndpoint, chip::EndpointId aLightGenericSwitchEndpointId)
{
    BindingHandler::GetInstance().Init();
    mLightSwitchEndpoint          = aLightDimmerSwitchEndpoint;
    mLightGenericSwitchEndpointId = aLightGenericSwitchEndpointId;
}

void LightSwitch::InitiateActionSwitch(Action mAction)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId = mLightSwitchEndpoint;
        data->ClusterId  = Clusters::OnOff::Id;
        switch (mAction)
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
        data->IsGroup = BindingHandler::GetInstance().IsGroupBound();
        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::DimmerChangeBrightness()
{
    static uint16_t sBrightness;
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId = mLightSwitchEndpoint;
        data->CommandId  = Clusters::LevelControl::Commands::MoveToLevel::Id;
        data->ClusterId  = Clusters::LevelControl::Id;
        // add to brightness 3 to approximate 1% step of brightness after each call dimmer change.
        sBrightness += kOnePercentBrightnessApproximation;
        if (sBrightness > kMaximumBrightness)
        {
            sBrightness = 0;
        }
        data->Value   = (uint8_t) sBrightness;
        data->IsGroup = BindingHandler::GetInstance().IsGroupBound();
        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    }
}

void LightSwitch::GenericSwitchInitialPress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Press moves Position from 0 (idle) to 1 (press)
        uint8_t newPosition = 1;

        Clusters::Switch::Attributes::CurrentPosition::Set(mLightGenericSwitchEndpointId, newPosition);
        // InitialPress event takes newPosition as event data
        Clusters::SwitchServer::Instance().OnInitialPress(mLightGenericSwitchEndpointId, newPosition);
    });
}

void LightSwitch::GenericSwitchReleasePress()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        // Release moves Position from 1 (press) to 0 (idle)
        uint8_t previousPosition = 1;
        uint8_t newPosition      = 0;

        Clusters::Switch::Attributes::CurrentPosition::Set(mLightGenericSwitchEndpointId, newPosition);
        // ShortRelease event takes previousPosition as event data
        Clusters::SwitchServer::Instance().OnShortRelease(mLightGenericSwitchEndpointId, previousPosition);
    });
}
