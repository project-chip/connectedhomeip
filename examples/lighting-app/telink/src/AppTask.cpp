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

#include "ColorFormat.h"
#include "PWMDevice.h"

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
const struct pwm_dt_spec sPwmRgbSpecBlueLed = LIGHTING_PWM_SPEC_RGB_BLUE;
#if USE_RGB_PWM
const struct pwm_dt_spec sPwmRgbSpecGreenLed = LIGHTING_PWM_SPEC_RGB_GREEN;
const struct pwm_dt_spec sPwmRgbSpecRedLed   = LIGHTING_PWM_SPEC_RGB_RED;

uint8_t sBrightness;
PWMDevice::Action_t sColorAction = PWMDevice::INVALID_ACTION;
XyColor_t sXY;
HsvColor_t sHSV;
CtColor_t sCT;
#endif
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    // Init lighting manager
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(kExampleEndpointId, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(kExampleEndpointId, &maxLightLevel);

    CHIP_ERROR err = sAppTask.mPwmRgbBlueLed.Init(&sPwmRgbSpecBlueLed, minLightLevel, maxLightLevel, maxLightLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Blue RGB PWM Device Init fail");
        return err;
    }
#if USE_RGB_PWM
    err = sAppTask.mPwmRgbRedLed.Init(&sPwmRgbSpecRedLed, minLightLevel, maxLightLevel, maxLightLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Red RGB PWM Device Init fail");
        return err;
    }

    err = sAppTask.mPwmRgbGreenLed.Init(&sPwmRgbSpecGreenLed, minLightLevel, maxLightLevel, maxLightLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green RGB PWM Device Init fail");
        return err;
    }
#endif
    sAppTask.mPwmRgbBlueLed.SetCallbacks(ActionInitiated, ActionCompleted, nullptr);

#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(LightingActionEventHandler);
#endif
    InitCommonParts();

    err = ConnectivityMgr().SetBLEDeviceName("TelinkLight");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    PWMDevice::Action_t action = PWMDevice::INVALID_ACTION;
    int32_t actor              = 0;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<PWMDevice::Action_t>(aEvent->LightingEvent.Action);
        actor  = aEvent->LightingEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
#if USE_RGB_PWM
        if (sAppTask.mPwmRgbRedLed.IsTurnedOn() || sAppTask.mPwmRgbGreenLed.IsTurnedOn() || sAppTask.mPwmRgbBlueLed.IsTurnedOn())
        {
            action = PWMDevice::OFF_ACTION;
        }
        else
        {
            action = PWMDevice::ON_ACTION;
        }
#else
        action = sAppTask.mPwmRgbBlueLed.IsTurnedOn() ? PWMDevice::OFF_ACTION : PWMDevice::ON_ACTION;
#endif
        actor = AppEvent::kEventType_Button;
    }

    if (action != PWMDevice::INVALID_ACTION &&
        (
#if USE_RGB_PWM
            !sAppTask.mPwmRgbRedLed.InitiateAction(action, actor, NULL) ||
            !sAppTask.mPwmRgbGreenLed.InitiateAction(action, actor, NULL) ||
#endif
            !sAppTask.mPwmRgbBlueLed.InitiateAction(action, actor, NULL)))
    {
        LOG_INF("Action is in progress or active");
    }
}

#ifdef CONFIG_CHIP_PW_RPC
void AppTask::ButtonEventHandler(ButtonId_t btnId, bool btnPressed)
{
    if (!btnPressed)
    {
        return;
    }

    switch (btnId)
    {
    case kButtonId_LightingAction:
        ExampleActionButtonEventHandler();
        break;
    case kButtonId_FactoryReset:
        FactoryResetButtonEventHandler();
        break;
    case kButtonId_StartThread:
        StartThreadButtonEventHandler();
        break;
    case kButtonId_StartBleAdv:
        StartBleAdvButtonEventHandler();
        break;
    }
}
#endif

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
#if USE_RGB_PWM
    bool isTurnedOn =
        sAppTask.mPwmRgbRedLed.IsTurnedOn() || sAppTask.mPwmRgbGreenLed.IsTurnedOn() || sAppTask.mPwmRgbBlueLed.IsTurnedOn();
#else
    bool isTurnedOn = sAppTask.mPwmRgbBlueLed.IsTurnedOn();
#endif
    // write the new on/off value
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(kExampleEndpointId, isTurnedOn);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Update OnOff fail: %x", status);
    }

#if USE_RGB_PWM
    uint8_t setLevel;
    if (sColorAction == PWMDevice::COLOR_ACTION_XY || sColorAction == PWMDevice::COLOR_ACTION_HSV ||
        sColorAction == PWMDevice::COLOR_ACTION_CT)
    {
        setLevel = sBrightness;
    }
    else
    {
        setLevel = sAppTask.mPwmRgbBlueLed.GetLevel();
    }
#else
    uint8_t setLevel = sAppTask.mPwmRgbBlueLed.GetLevel();
#endif
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, setLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Update CurrentLevel fail: %x", status);
    }
}

void AppTask::SetInitiateAction(PWMDevice::Action_t aAction, int32_t aActor, uint8_t * value)
{
#if USE_RGB_PWM
    bool setRgbAction = false;
    RgbColor_t rgb;
#endif

    if (aAction == PWMDevice::ON_ACTION || aAction == PWMDevice::OFF_ACTION)
    {
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
#if USE_RGB_PWM
        sAppTask.mPwmRgbRedLed.InitiateAction(aAction, aActor, value);
        sAppTask.mPwmRgbGreenLed.InitiateAction(aAction, aActor, value);
#endif
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
#if USE_RGB_PWM
        // Save a new brightness for ColorControl
        sBrightness = *value;

        if (sColorAction == PWMDevice::COLOR_ACTION_XY)
        {
            rgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        }
        else if (sColorAction == PWMDevice::COLOR_ACTION_HSV)
        {
            sHSV.v = sBrightness;
            rgb    = HsvToRgb(sHSV);
        }
        else
        {
            rgb.r = sBrightness;
            rgb.g = sBrightness;
            rgb.b = sBrightness;
        }

        ChipLogProgress(Zcl, "New brightness: %u | R: %u, G: %u, B: %u", sBrightness, rgb.r, rgb.g, rgb.b);
        setRgbAction = true;
#else
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
#endif
    }

#if USE_RGB_PWM
    else if (aAction == PWMDevice::COLOR_ACTION_XY)
    {
        sXY = *reinterpret_cast<XyColor_t *>(value);
        rgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        ChipLogProgress(Zcl, "XY to RGB: X: %u, Y: %u, Level: %u | R: %u, G: %u, B: %u", sXY.x, sXY.y, sBrightness, rgb.r, rgb.g,
                        rgb.b);
        setRgbAction = true;
        sColorAction = PWMDevice::COLOR_ACTION_XY;
    }
    else if (aAction == PWMDevice::COLOR_ACTION_HSV)
    {
        sHSV   = *reinterpret_cast<HsvColor_t *>(value);
        sHSV.v = sBrightness;
        rgb    = HsvToRgb(sHSV);
        ChipLogProgress(Zcl, "HSV to RGB: H: %u, S: %u, V: %u | R: %u, G: %u, B: %u", sHSV.h, sHSV.s, sHSV.v, rgb.r, rgb.g, rgb.b);
        setRgbAction = true;
        sColorAction = PWMDevice::COLOR_ACTION_HSV;
    }
    else if (aAction == PWMDevice::COLOR_ACTION_CT)
    {
        sCT = *reinterpret_cast<CtColor_t *>(value);
        if (sCT.ctMireds)
        {
            rgb = CTToRgb(sCT);
            ChipLogProgress(Zcl, "ColorTemp to RGB: CT: %u | R: %u, G: %u, B: %u", sCT.ctMireds, rgb.r, rgb.g, rgb.b);
            setRgbAction = true;
            sColorAction = PWMDevice::COLOR_ACTION_CT;
        }
    }

    if (setRgbAction)
    {
        sAppTask.mPwmRgbRedLed.InitiateAction(aAction, aActor, &rgb.r);
        sAppTask.mPwmRgbGreenLed.InitiateAction(aAction, aActor, &rgb.g);
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, &rgb.b);
    }
#endif
}
