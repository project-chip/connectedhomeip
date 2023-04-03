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

#include "ButtonManager.h"

#include "ThreadUtil.h"

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/ErrorStr.h>
#include <system/SystemClock.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

#include <algorithm>

#if CONFIG_CHIP_LIB_SHELL
#include <sys.h>
#include <zephyr/shell/shell.h>

static int cmd_telink_reboot(const struct shell * shell, size_t argc, char ** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "Performing board reboot...");
    sys_reboot();
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_telink, SHELL_CMD(reboot, NULL, "Reboot board command", cmd_telink_reboot),
                               SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(telink, &sub_telink, "Telink commands", NULL);
#endif // CONFIG_CHIP_LIB_SHELL

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
constexpr int kFactoryResetCalcTimeout          = 3000;
constexpr int kFactoryResetTriggerCntr          = 3;
constexpr int kAppEventQueueSize                = 10;
constexpr uint8_t kButtonPushEvent              = 1;
constexpr uint8_t kButtonReleaseEvent           = 0;

#if APP_USE_IDENTIFY_PWM
constexpr EndpointId kLightEndpointId           = 1;
constexpr uint8_t kDefaultMinLevel              = 0;
constexpr uint8_t kDefaultMaxLevel              = 254;
constexpr uint32_t kIdentifyBlinkRateMs         = 200;
constexpr uint32_t kIdentifyOkayOnRateMs        = 50;
constexpr uint32_t kIdentifyOkayOffRateMs       = 950;
constexpr uint32_t kIdentifyFinishOnRateMs      = 950;
constexpr uint32_t kIdentifyFinishOffRateMs     = 50;
constexpr uint32_t kIdentifyChannelChangeRateMs = 1000;
constexpr uint32_t kIdentifyBreatheRateMs       = 1000;
const struct pwm_dt_spec sPwmIdentifySpecGreenLed = LIGHTING_PWM_SPEC_IDENTIFY_GREEN;
#endif

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFactoryResetTimer;
uint8_t sFactoryResetCntr = 0;

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sStatusLED;
#if APP_USE_IDENTIFY_PWM
LEDWidget sIdentifyLED;
#endif
#endif

Button sFactoryResetButton;
Button sBleAdvStartButton;

bool sIsThreadProvisioned = false;
bool sIsThreadEnabled     = false;
bool sIsThreadAttached    = false;
bool sHaveBLEConnections  = false;

#if APP_USE_IDENTIFY_PWM
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

void OnIdentifyTriggerEffect(Identify * identify)
{
    AppTask::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

Identify sIdentify = {
    kLightEndpointId,
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};
#endif
} // namespace

AppTask AppTask::sAppTask;

class AppFabricTableDelegate : public FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
    {
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
    }
};


CHIP_ERROR AppTask::AppTaskCommon::StartApp()
{
    CHIP_ERROR err = AppTask::Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask Init fail");
        return err;
    }

    AppEvent event = {};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(&event);
    }
}

#if APP_USE_IDENTIFY_PWM
void AppTask::IdentifyEffectHandler(EmberAfIdentifyEffectIdentifier aEffect)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_IdentifyStart;

    switch (aEffect)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        event.Handler = [](AppEvent *) {
            sAppTask.mPwmIdentifyLed.InitiateBlinkAction(kIdentifyBlinkRateMs, kIdentifyBlinkRateMs);
        };
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        event.Handler = [](AppEvent *) {
            sAppTask.mPwmIdentifyLed.InitiateBreatheAction(PWMDevice::kBreatheType_Both, kIdentifyBreatheRateMs);
        };
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        event.Handler = [](AppEvent *) {
            sAppTask.mPwmIdentifyLed.InitiateBlinkAction(kIdentifyOkayOnRateMs, kIdentifyOkayOffRateMs);
        };
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        event.Handler = [](AppEvent *) {
            sAppTask.mPwmIdentifyLed.InitiateBlinkAction(kIdentifyChannelChangeRateMs, kIdentifyChannelChangeRateMs);
        };
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT");
        event.Handler = [](AppEvent *) {
            sAppTask.mPwmIdentifyLed.InitiateBlinkAction(kIdentifyFinishOnRateMs, kIdentifyFinishOffRateMs);
        };
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT");
        event.Handler = [](AppEvent *) { sAppTask.mPwmIdentifyLed.StopAction(); };
        event.Type    = AppEvent::kEventType_IdentifyStop;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }

    sAppTask.PostEvent(&event);
}
#endif

void AppTask::FactoryResetButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = FactoryResetHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FactoryResetHandler(AppEvent * aEvent)
{
    if (sFactoryResetCntr == 0)
    {
        k_timer_start(&sFactoryResetTimer, K_MSEC(kFactoryResetCalcTimeout), K_NO_WAIT);
    }

    sFactoryResetCntr++;
    LOG_INF("Factory Reset Trigger Counter: %d/%d", sFactoryResetCntr, kFactoryResetTriggerCntr);

    if (sFactoryResetCntr == kFactoryResetTriggerCntr)
    {
        k_timer_stop(&sFactoryResetTimer);
        sFactoryResetCntr = 0;

        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::StartBleAdvButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartBleAdvHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::StartBleAdvHandler(AppEvent * aEvent)
{
    LOG_INF("StartBleAdvHandler");

    // Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("Device already commissioned");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE adv already enabled");
        return;
    }

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow fail");
    }
}

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
void AppTask::UpdateLedStateEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_UpdateLedState)
    {
        aEvent->UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTask::LEDStateUpdateHandler(LEDWidget * ledWidget)
{
    AppEvent event;
    event.Type                          = AppEvent::kEventType_UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = ledWidget;
    sAppTask.PostEvent(&event);
}

void AppTask::UpdateStatusLED(void)
{
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        if (sIsThreadAttached)
        {
            sStatusLED.Blink(950, 50);
        }
        else
        {
            sStatusLED.Blink(100, 100);
        }
    }
    else
    {
        sStatusLED.Blink(50, 950);
    }
}
#endif

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        UpdateStatusLED();
#endif
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        sIsThreadAttached    = ConnectivityMgr().IsThreadAttached();
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        UpdateStatusLED();
#endif
        break;
    case DeviceEventType::kThreadConnectivityChange:
#if CONFIG_CHIP_OTA_REQUESTOR
        if (event->ThreadConnectivityChange.Result == kConnectivity_Established)
        {
            InitBasicOTARequestor();
        }
#endif
        break;
    default:
        break;
    }
}

#if APP_USE_IDENTIFY_PWM
void AppTask::ActionIdentifyStateUpdateHandler(k_timer * timer)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_UpdateLedState;
    event.Handler = UpdateIdentifyStateEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::UpdateIdentifyStateEventHandler(AppEvent * aEvent)
{
    sAppTask.mPwmIdentifyLed.UpdateAction();
}
#endif
void AppTask::PostEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("PostEvent fail");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Dropping event without handler");
    }
}

void AppTask::FactoryResetTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = FactoryResetTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FactoryResetTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    sFactoryResetCntr = 0;
    LOG_INF("Factory Reset Trigger Counter is cleared");
}
