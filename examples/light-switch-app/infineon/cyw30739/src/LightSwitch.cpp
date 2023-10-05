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

#include <LightSwitch.h>

#include <BindingHandler.h>
#include <app/util/binding-table.h>
#include <controller/InvokeInteraction.h>

#define CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM 254

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void LightSwitch::Init(chip::EndpointId aLightDimmerSwitchEndpoint)
{
    BindingHandler::GetInstance().Init();
    mLightSwitchEndpoint = aLightDimmerSwitchEndpoint;
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
            mStatus         = LightSwitch::Status::Toggle;
            break;
        case Action::On:
            data->CommandId = Clusters::OnOff::Commands::On::Id;
            mStatus         = LightSwitch::Status::On;
            break;
        case Action::Off:
            data->CommandId = Clusters::OnOff::Commands::Off::Id;
            mStatus         = LightSwitch::Status::Off;
            break;
        default:
            Platform::Delete(data);
            mStatus = LightSwitch::Status::Unknown;
            return;
        }
        data->IsGroup = BindingHandler::GetInstance().IsGroupBound();
        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    }
}

LightSwitch::Status LightSwitch::GetSwitchStatus()
{
    return mStatus;
}

void LightSwitch::DimmerChangeBrightness(uint16_t kValue)
{
    static uint16_t sBrightness;
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    if (data)
    {
        data->EndpointId = mLightSwitchEndpoint;
        data->CommandId  = Clusters::LevelControl::Commands::MoveToLevel::Id;
        data->ClusterId  = Clusters::LevelControl::Id;
        // add to brightness vluse to change brightness after call dimmer change.
        sBrightness = kValue;
        data->Value =
            (uint8_t) (sBrightness > CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM ? CHIP_DEVICE_CONFIG_BRIGHTNESS_MAXIMUM : sBrightness);
        data->IsGroup = BindingHandler::GetInstance().IsGroupBound();
        DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    }
}
