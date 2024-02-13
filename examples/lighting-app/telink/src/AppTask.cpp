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
#ifdef CONFIG_WS2812_STRIP
const struct device * const ws2812_dev = DEVICE_DT_GET(DT_ALIAS(led_strip));
#else
const struct pwm_dt_spec sPwmRgbSpecBlueLed  = PWM_DT_SPEC_GET_OR(DT_ALIAS(pwm_led0), {});
#if USE_RGB_PWM
const struct pwm_dt_spec sPwmRgbSpecGreenLed = PWM_DT_SPEC_GET_OR(DT_ALIAS(pwm_led1), {});
const struct pwm_dt_spec sPwmRgbSpecRedLed   = PWM_DT_SPEC_GET_OR(DT_ALIAS(pwm_led2), {});
#endif
#endif // CONFIG_WS2812_STRIP

#if defined(CONFIG_WS2812_STRIP) || USE_RGB_PWM
uint8_t sBrightness;
PWMDevice::Action_t sColorAction = PWMDevice::INVALID_ACTION;
XyColor_t sXY;
HsvColor_t sHSV;
CtColor_t sCT;
#endif // CONFIG_WS2812_STRIP || USE_RGB_PWM
} // namespace

AppTask AppTask::sAppTask;

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
void AppTask::PowerOnFactoryReset(void)
{
    LOG_INF("Lighting App Power On Factory Reset");
    AppEvent event;
    event.Type    = AppEvent::kEventType_Lighting;
    event.Handler = PowerOnFactoryResetEventHandler;
    GetAppTask().PostEvent(&event);
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

CHIP_ERROR AppTask::Init(void)
{
    CHIP_ERROR err;

    // Init lighting manager
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(kExampleEndpointId, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(kExampleEndpointId, &maxLightLevel);

#ifdef CONFIG_WS2812_STRIP
    err = sAppTask.mWS2812Device.Init(ws2812_dev, STRIP_NUM_PIXELS(led_strip));
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("WS2812 Device Init fail");
        return err;
    }
#else
    err = sAppTask.mPwmRgbBlueLed.Init(&sPwmRgbSpecBlueLed, minLightLevel, maxLightLevel, maxLightLevel);
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
#endif // CONFIG_WS2812_STRIP

#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(LightingActionEventHandler);
#endif
    InitCommonParts();

    return CHIP_NO_ERROR;
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
#ifdef CONFIG_WS2812_STRIP
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (sAppTask.mWS2812Device.IsTurnedOn())
        {
            sAppTask.mWS2812Device.Set(SET_RGB_TURN_OFF);
        }
        else
        {
            sAppTask.mWS2812Device.Set(SET_RGB_TURN_ON);
        }

        sAppTask.UpdateClusterState();
    }
#else
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
#endif // CONFIG_WS2812_STRIP
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
    Protocols::InteractionModel::Status status;
    bool isTurnedOn;
    uint8_t setLevel;

#if defined(CONFIG_WS2812_STRIP) || USE_RGB_PWM
#ifdef CONFIG_WS2812_STRIP
    isTurnedOn = sAppTask.mWS2812Device.IsTurnedOn();
#else
    isTurnedOn =
        sAppTask.mPwmRgbRedLed.IsTurnedOn() || sAppTask.mPwmRgbGreenLed.IsTurnedOn() || sAppTask.mPwmRgbBlueLed.IsTurnedOn();
#endif // CONFIG_WS2812_STRIP

    if (sColorAction == PWMDevice::COLOR_ACTION_XY || sColorAction == PWMDevice::COLOR_ACTION_HSV ||
        sColorAction == PWMDevice::COLOR_ACTION_CT)
    {
        setLevel = sBrightness;
    }
    else
    {
#ifdef CONFIG_WS2812_STRIP
        setLevel = sAppTask.mWS2812Device.GetBlueLevel();
        if (setLevel > kDefaultMaxLevel)
        {
            setLevel = kDefaultMaxLevel;
        }
#else
        setLevel = sAppTask.mPwmRgbBlueLed.GetLevel();
#endif // CONFIG_WS2812_STRIP
    }
#else
    isTurnedOn = sAppTask.mPwmRgbBlueLed.IsTurnedOn();
    setLevel   = sAppTask.mPwmRgbBlueLed.GetLevel();
#endif // CONFIG_WS2812_STRIP || USE_RGB_PWM

    // write the new on/off value
    status = Clusters::OnOff::Attributes::OnOff::Set(kExampleEndpointId, isTurnedOn);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Update OnOff fail: %x", to_underlying(status));
    }

    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, setLevel);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Update CurrentLevel fail: %x", to_underlying(status));
    }
}

void AppTask::SetInitiateAction(PWMDevice::Action_t aAction, int32_t aActor, uint8_t * value)
{
#if defined(CONFIG_WS2812_STRIP) || USE_RGB_PWM
    bool setRgbAction = false;
    RgbColor_t rgb;
#endif // CONFIG_WS2812_STRIP || USE_RGB_PWM

    if (aAction == PWMDevice::ON_ACTION || aAction == PWMDevice::OFF_ACTION)
    {
#ifdef CONFIG_WS2812_STRIP
        if (aAction == PWMDevice::ON_ACTION)
        {
            sAppTask.mWS2812Device.Set(SET_RGB_TURN_ON);
        }
        else if (aAction == PWMDevice::OFF_ACTION)
        {
            sAppTask.mWS2812Device.Set(SET_RGB_TURN_OFF);
        }
#else
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
#if USE_RGB_PWM
        sAppTask.mPwmRgbRedLed.InitiateAction(aAction, aActor, value);
        sAppTask.mPwmRgbGreenLed.InitiateAction(aAction, aActor, value);
#endif
#endif // CONFIG_WS2812_STRIP
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
#if defined(CONFIG_WS2812_STRIP) || USE_RGB_PWM
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
            memset(&rgb, sBrightness, sizeof(RgbColor_t));
        }

        ChipLogProgress(Zcl, "New brightness: %u | R: %u, G: %u, B: %u", sBrightness, rgb.r, rgb.g, rgb.b);
        setRgbAction = true;
#else
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, value);
#endif // CONFIG_WS2812_STRIP || USE_RGB_PWM
    }
#if defined(CONFIG_WS2812_STRIP) || USE_RGB_PWM
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
#ifdef CONFIG_WS2812_STRIP
        sAppTask.mWS2812Device.SetLevel(&rgb);
#else
        sAppTask.mPwmRgbRedLed.InitiateAction(aAction, aActor, &rgb.r);
        sAppTask.mPwmRgbGreenLed.InitiateAction(aAction, aActor, &rgb.g);
        sAppTask.mPwmRgbBlueLed.InitiateAction(aAction, aActor, &rgb.b);
#endif // CONFIG_WS2812_STRIP
    }
#endif // CONFIG_WS2812_STRIP || USE_RGB_PWM
}

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
static constexpr uint32_t kPowerOnFactoryResetIndicationMax    = 4;
static constexpr uint32_t kPowerOnFactoryResetIndicationTimeMs = 1000;

unsigned int AppTask::sPowerOnFactoryResetTimerCnt;
k_timer AppTask::sPowerOnFactoryResetTimer;

void AppTask::PowerOnFactoryResetEventHandler(AppEvent * aEvent)
{
    LOG_INF("Lighting App Power On Factory Reset Handler");
    sPowerOnFactoryResetTimerCnt = 1;
#ifdef CONFIG_WS2812_STRIP
    sAppTask.mWS2812Device.Set(sPowerOnFactoryResetTimerCnt % 2);
#else
    sAppTask.mPwmRgbBlueLed.Set(sPowerOnFactoryResetTimerCnt % 2);
#if USE_RGB_PWM
    sAppTask.mPwmRgbRedLed.Set(sPowerOnFactoryResetTimerCnt % 2);
    sAppTask.mPwmRgbGreenLed.Set(sPowerOnFactoryResetTimerCnt % 2);
#endif
#endif // CONFIG_WS2812_STRIP
    k_timer_init(&sPowerOnFactoryResetTimer, PowerOnFactoryResetTimerEvent, nullptr);
    k_timer_start(&sPowerOnFactoryResetTimer, K_MSEC(kPowerOnFactoryResetIndicationTimeMs),
                  K_MSEC(kPowerOnFactoryResetIndicationTimeMs));
}

void AppTask::PowerOnFactoryResetTimerEvent(struct k_timer * timer)
{
    sPowerOnFactoryResetTimerCnt++;
    LOG_INF("Lighting App Power On Factory Reset Handler %u", sPowerOnFactoryResetTimerCnt);
#ifdef CONFIG_WS2812_STRIP
    sAppTask.mWS2812Device.Set(sPowerOnFactoryResetTimerCnt % 2);
#else
    sAppTask.mPwmRgbBlueLed.Set(sPowerOnFactoryResetTimerCnt % 2);
#if USE_RGB_PWM
    sAppTask.mPwmRgbRedLed.Set(sPowerOnFactoryResetTimerCnt % 2);
    sAppTask.mPwmRgbGreenLed.Set(sPowerOnFactoryResetTimerCnt % 2);
#endif
#endif // CONFIG_WS2812_STRIP
    if (sPowerOnFactoryResetTimerCnt > kPowerOnFactoryResetIndicationMax)
    {
        k_timer_stop(timer);
        LOG_INF("schedule factory reset");
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */
