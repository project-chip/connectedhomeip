/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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
#include "LEDManager.h"
#include "PWMManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
bool sfixture_on;
uint8_t sBrightness;
AppTask::Fixture_Action sColorAction = AppTask::INVALID_ACTION;
XyColor_t sXY;
HsvColor_t sHSV;
CtColor_t sCT;
RgbColor_t sLedRgb;
} // namespace

AppTask AppTask::sAppTask;

bool AppTask::IsTurnedOn() const
{
    return sfixture_on;
}

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
void AppTask::PowerOnFactoryReset(void)
{
    LOG_INF("Lighting App Power On Factory Reset");
    AppEvent event;
    event.Type    = AppEvent::kEventType_DeviceAction;
    event.Handler = PowerOnFactoryResetEventHandler;
    GetAppTask().PostEvent(&event);
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(LightingActionEventHandler);
    InitCommonParts();

    Protocols::InteractionModel::Status status;

    app::DataModel::Nullable<uint8_t> brightness;
    // Read brightness value
    status = Clusters::LevelControl::Attributes::CurrentLevel::Get(kExampleEndpointId, brightness);
    if (status == Protocols::InteractionModel::Status::Success && !brightness.IsNull())
    {
        sBrightness = brightness.Value();
    }

    memset(&sLedRgb, sBrightness, sizeof(RgbColor_t));

    bool storedValue;
    // Read storedValue on/off value
    status = Clusters::OnOff::Attributes::OnOff::Get(1, &storedValue);
    if (status == Protocols::InteractionModel::Status::Success)
    {
        // Set actual state to stored before reboot
        SetInitiateAction(storedValue ? ON_ACTION : OFF_ACTION, static_cast<int32_t>(AppEvent::kEventType_DeviceAction), nullptr);
    }

    return CHIP_NO_ERROR;
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    Fixture_Action action = INVALID_ACTION;
    int32_t actor         = 0;

    if (aEvent->Type == AppEvent::kEventType_DeviceAction)
    {
        action = static_cast<Fixture_Action>(aEvent->DeviceEvent.Action);
        actor  = aEvent->DeviceEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        sfixture_on = !sfixture_on;

        sAppTask.UpdateClusterState();
    }
}

void AppTask::UpdateClusterState(void)
{
    Protocols::InteractionModel::Status status;
    bool isTurnedOn  = sfixture_on;
    uint8_t setLevel = sBrightness;

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

void AppTask::SetInitiateAction(Fixture_Action aAction, int32_t aActor, uint8_t * value)
{
    bool setRgbAction = false;

    if (aAction == ON_ACTION || aAction == OFF_ACTION)
    {
        if (aAction == ON_ACTION)
        {
            sfixture_on = true;
#ifdef CONFIG_PWM
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (((uint32_t) sLedRgb.r * 1000) / UINT8_MAX));
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (((uint32_t) sLedRgb.g * 1000) / UINT8_MAX));
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (((uint32_t) sLedRgb.b * 1000) / UINT8_MAX));
#else
            LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
#endif
        }
        else
        {
            sfixture_on = false;
#ifdef CONFIG_PWM
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, false);
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, false);
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, false);
#else
            LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
#endif
        }
    }
    else if (aAction == LEVEL_ACTION)
    {
        // Save a new brightness for ColorControl
        sBrightness = *value;

        if (sColorAction == COLOR_ACTION_XY)
        {
            sLedRgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        }
        else if (sColorAction == COLOR_ACTION_HSV)
        {
            sHSV.v  = sBrightness;
            sLedRgb = HsvToRgb(sHSV);
        }
        else
        {
            memset(&sLedRgb, sBrightness, sizeof(RgbColor_t));
        }

        ChipLogProgress(Zcl, "New brightness: %u | R: %u, G: %u, B: %u", sBrightness, sLedRgb.r, sLedRgb.g, sLedRgb.b);
        setRgbAction = true;
    }
    else if (aAction == COLOR_ACTION_XY)
    {
        sXY     = *reinterpret_cast<XyColor_t *>(value);
        sLedRgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        ChipLogProgress(Zcl, "XY to RGB: X: %u, Y: %u, Level: %u | R: %u, G: %u, B: %u", sXY.x, sXY.y, sBrightness, sLedRgb.r,
                        sLedRgb.g, sLedRgb.b);
        setRgbAction = true;
        sColorAction = COLOR_ACTION_XY;
    }
    else if (aAction == COLOR_ACTION_HSV)
    {
        sHSV    = *reinterpret_cast<HsvColor_t *>(value);
        sHSV.v  = sBrightness;
        sLedRgb = HsvToRgb(sHSV);
        ChipLogProgress(Zcl, "HSV to RGB: H: %u, S: %u, V: %u | R: %u, G: %u, B: %u", sHSV.h, sHSV.s, sHSV.v, sLedRgb.r, sLedRgb.g,
                        sLedRgb.b);
        setRgbAction = true;
        sColorAction = COLOR_ACTION_HSV;
    }
    else if (aAction == COLOR_ACTION_CT)
    {
        sCT = *reinterpret_cast<CtColor_t *>(value);
        if (sCT.ctMireds)
        {
            sLedRgb = CTToRgb(sCT);
            ChipLogProgress(Zcl, "ColorTemp to RGB: CT: %u | R: %u, G: %u, B: %u", sCT.ctMireds, sLedRgb.r, sLedRgb.g, sLedRgb.b);
            setRgbAction = true;
            sColorAction = COLOR_ACTION_CT;
        }
    }

    if (setRgbAction)
    {
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (((uint32_t) sLedRgb.r * 1000) / UINT8_MAX));
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (((uint32_t) sLedRgb.g * 1000) / UINT8_MAX));
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (((uint32_t) sLedRgb.b * 1000) / UINT8_MAX));
    }
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
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (bool) (sPowerOnFactoryResetTimerCnt % 2));
#if !CONFIG_PWM
    LedManager::getInstance().setLed(LedManager::EAppLed_App0, (bool) (sPowerOnFactoryResetTimerCnt % 2));
#endif
    k_timer_init(&sPowerOnFactoryResetTimer, PowerOnFactoryResetTimerEvent, nullptr);
    k_timer_start(&sPowerOnFactoryResetTimer, K_MSEC(kPowerOnFactoryResetIndicationTimeMs),
                  K_MSEC(kPowerOnFactoryResetIndicationTimeMs));
}

void AppTask::PowerOnFactoryResetTimerEvent(struct k_timer * timer)
{
    sPowerOnFactoryResetTimerCnt++;
    LOG_INF("Lighting App Power On Factory Reset Handler %u", sPowerOnFactoryResetTimerCnt);
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    if (sPowerOnFactoryResetTimerCnt > kPowerOnFactoryResetIndicationMax)
    {
        k_timer_stop(timer);
        LOG_INF("schedule factory reset");
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
#endif

#if !CONFIG_PWM
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#endif /* !CONFIG_PWM */
}
