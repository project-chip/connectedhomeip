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

#include "AppTask.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "ButtonManager.h"
#include "binding-handler.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <DeviceInfoProviderImpl.h>

#include "ThreadUtil.h"

#include <app/clusters/identify-server/identify-server.h>
#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>
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
constexpr int kFactoryResetTriggerTimeout       = 2000;
constexpr int kAppEventQueueSize                = 10;
constexpr uint8_t kButtonPushEvent              = 1;
constexpr uint8_t kButtonReleaseEvent           = 0;
constexpr EndpointId kEndpointId                = 1;
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

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFactoryResetTimer;

LEDWidget sStatusLED;

Button sFactoryResetButton;
Button sThreadStartButton;
Button sBleAdvStartButton;

bool sIsThreadProvisioned       = false;
bool sIsThreadEnabled           = false;
bool sIsThreadAttached          = false;
bool sHaveBLEConnections        = false;
bool sIsFactoryResetTimerActive = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

void OnIdentifyTriggerEffect(Identify * identify)
{
    AppTask::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

Identify sIdentify = {
    kEndpointId,
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};

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

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

CHIP_ERROR AppTask::Init(void)
{
    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize status LED
    LEDWidget::InitGpio(LEDS_PORT);
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(SYSTEM_STATE_LED);

    UpdateStatusLED();

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize PWM Identify led
    CHIP_ERROR err = sAppTask.mPwmIdentifyLed.Init(&sPwmIdentifySpecGreenLed, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMaxLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }

    sAppTask.mPwmIdentifyLed.SetCallbacks(nullptr, nullptr, ActionIdentifyStateUpdateHandler);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

#if CONFIG_CHIP_OTA_REQUESTOR
    InitBasicOTARequestor();
#endif

    // We only have network commissioning on endpoint 0.
    // Set up a valid Network Commissioning cluster on endpoint 0 is done in
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    ConfigurationMgr().LogDeviceConfig();

    // Configure Bindings
    err = InitBindingHandlers();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("InitBindingHandlers fail");
        return err;
    }

    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = ConnectivityMgr().SetBLEDeviceName("TelinkApp");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(new AppFabricTableDelegate);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppFabricTableDelegate fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartApp(void)
{
    CHIP_ERROR err = Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask Init fail");
        return err;
    }

    AppEvent event = {};

    while (true)
    {
        int ret = k_msgq_get(&sAppEventQueue, &event, K_MSEC(10));

        while (!ret)
        {
            DispatchEvent(&event);
            ret = k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT);
        }
    }
}

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
    if (!sIsFactoryResetTimerActive)
    {
        k_timer_start(&sFactoryResetTimer, K_MSEC(kFactoryResetTriggerTimeout), K_NO_WAIT);
        sIsFactoryResetTimerActive = true;
    }
    else
    {
        k_timer_stop(&sFactoryResetTimer);
        sIsFactoryResetTimerActive = false;
    }
}

void AppTask::StartThreadButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartThreadHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::StartThreadHandler(AppEvent * aEvent)
{
    LOG_INF("StartThreadHandler");
    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // Switch context from BLE to Thread
        Internal::BLEManagerImpl sInstance;
        sInstance.SwitchToIeee802154();
        StartDefaultThreadNetwork();
    }
    else
    {
        LOG_INF("Device already commissioned");
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

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        sIsThreadAttached    = ConnectivityMgr().IsThreadAttached();
        UpdateStatusLED();
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

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("PostEvent fail");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
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

    sIsFactoryResetTimerActive = false;
    LOG_INF("FactoryResetHandler");
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::InitButtons(void)
{
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_1, true, FactoryResetButtonEventHandler);
    sThreadStartButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_2, false, StartThreadButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_2, false, StartBleAdvButtonEventHandler);

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sThreadStartButton);
    ButtonManagerInst().AddButton(sBleAdvStartButton);
}
