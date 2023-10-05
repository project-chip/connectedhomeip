/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AppTask.h"
#include <app/server/Server.h>

#include "ColorFormat.h"
#include "PWMDevice.h"

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
const struct pwm_dt_spec sPwmRgbSpecBlueLed = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(AirQualityActionEventHandler);
#endif
    InitCommonParts();

   CHIP_ERROR err = ConnectivityMgr().SetBLEDeviceName("TelinkAirQuality");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

// void AppTask::AirQualityActionEventHandler(AppEvent * aEvent)
// {
//     PWMDevice::Action_t action = PWMDevice::INVALID_ACTION;
//     int32_t actor              = 0;

//     if (aEvent->Type == AppEvent::kEventType_Lighting)
//     {
//         action = static_cast<PWMDevice::Action_t>(aEvent->LightingEvent.Action);
//         actor  = aEvent->LightingEvent.Actor;
//     }
//     else if (aEvent->Type == AppEvent::kEventType_Button)
//     {
//         action = sAppTask.mPwmRgbBlueLed.IsTurnedOn() ? PWMDevice::OFF_ACTION : PWMDevice::ON_ACTION;
//         actor = AppEvent::kEventType_Button;
//     }

//     if (action != PWMDevice::INVALID_ACTION &&
//         (
//             !sAppTask.mPwmRgbBlueLed.InitiateAction(action, actor, NULL)))
//     {
//         LOG_INF("Action is in progress or active");
//     }
// }

void AppTask::SelfTestHandler(AppEvent * aEvent)
{

}

void AppTask::AirQualityActionEventHandler(AppEvent * aEvent)
{
    AppEvent event;
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        event.ButtonEvent.Action = kButtonPushEvent;
        event.Handler            = SelfTestHandler;
        GetAppTask().PostEvent(&event);
    }
}

void AppTask::ActionInitiated(PWMDevice::Action_t aAction, int32_t aActor)
{
    if (aAction == PWMDevice::ON_ACTION)
    {
        LOG_DBG("ON_ACTION initiated");
    }
    else if (aAction == PWMDevice::OFF_ACTION)
    {
        LOG_DBG("OFF_ACTION initiated");
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
        LOG_DBG("LEVEL_ACTION initiated");
    }
}

void AppTask::ActionCompleted(PWMDevice::Action_t aAction, int32_t aActor)
{
    if (aAction == PWMDevice::ON_ACTION)
    {
        LOG_DBG("ON_ACTION completed");
    }
    else if (aAction == PWMDevice::OFF_ACTION)
    {
        LOG_DBG("OFF_ACTION completed");
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
        LOG_DBG("LEVEL_ACTION completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::UpdateClusterState(void)
{
    bool isTurnedOn = sAppTask.mPwmRgbBlueLed.IsTurnedOn();
    // write the new on/off value
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(kExampleEndpointId, isTurnedOn);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Update OnOff fail: %x", status);
    }

    uint8_t setLevel = sAppTask.mPwmRgbBlueLed.GetLevel();
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, setLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Update CurrentLevel fail: %x", status);
    }
}

void AppTask::SetInitiateAction(PWMDevice::Action_t aAction, int32_t aActor, uint8_t * value)
{

    if (aAction == PWMDevice::ON_ACTION || aAction == PWMDevice::OFF_ACTION)
    {
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
    }

}

