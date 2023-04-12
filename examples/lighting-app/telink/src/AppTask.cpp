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

#include "AppConfig.h"
#include "AppEvent.h"
#include "ButtonManager.h"
#include "ColorFormat.h"

#include "ThreadUtil.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
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

const struct pwm_dt_spec sPwmRgbSpecBlueLed = LIGHTING_PWM_SPEC_RGB_BLUE;
#if USE_RGB_PWM
const struct pwm_dt_spec sPwmRgbSpecGreenLed = LIGHTING_PWM_SPEC_RGB_GREEN;
const struct pwm_dt_spec sPwmRgbSpecRedLed   = LIGHTING_PWM_SPEC_RGB_RED;
#endif
const struct pwm_dt_spec sPwmIdentifySpecGreenLed = LIGHTING_PWM_SPEC_IDENTIFY_GREEN;

#if CONFIG_CHIP_FACTORY_DATA
// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
#endif

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFactoryResetTimer;
uint8_t sFactoryResetCntr = 0;

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sStatusLED;
#endif
#if USE_RGB_PWM
uint8_t sBrightness;
PWMDevice::Action_t sColorAction = PWMDevice::INVALID_ACTION;
XyColor_t sXY;
HsvColor_t sHSV;
CtColor_t sCT;
#endif

Button sFactoryResetButton;
Button sLightingButton;
Button sThreadStartButton;
Button sBleAdvStartButton;

bool sIsThreadProvisioned = false;
bool sIsThreadEnabled     = false;
bool sIsThreadAttached    = false;
bool sHaveBLEConnections  = false;

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

CHIP_ERROR AppTask::Init(void)
{
    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize LEDs
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    LEDWidget::InitGpio(LEDS_PORT);
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);

    UpdateStatusLED();
#endif

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Init lighting manager
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(kLightEndpointId, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(kLightEndpointId, &maxLightLevel);

    // Initialize PWM LEDs
    CHIP_ERROR err = sAppTask.mPwmIdentifyLed.Init(&sPwmIdentifySpecGreenLed, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMaxLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }

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
    sAppTask.mPwmIdentifyLed.SetCallbacks(nullptr, nullptr, ActionIdentifyStateUpdateHandler);

    // Initialize CHIP server
#if CONFIG_CHIP_FACTORY_DATA
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
    // Read EnableKey from the factory data.
    MutableByteSpan enableKey(sTestEventTriggerEnableKey);
    err = mFactoryDataProvider.GetEnableKey(enableKey);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("GetEnableKey fail");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    static CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

#if CONFIG_CHIP_OTA_REQUESTOR
    InitBasicOTARequestor();
#endif

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = ConnectivityMgr().SetBLEDeviceName("TelinkLight");
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
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(&event);
    }
}

void AppTask::LightingActionButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = LightingActionEventHandler;
    sAppTask.PostEvent(&event);
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

void AppTask::UpdateClusterState(void)
{
#if USE_RGB_PWM
    bool isTurnedOn =
        sAppTask.mPwmRgbRedLed.IsTurnedOn() || sAppTask.mPwmRgbGreenLed.IsTurnedOn() || sAppTask.mPwmRgbBlueLed.IsTurnedOn();
#else
    bool isTurnedOn  = sAppTask.mPwmRgbBlueLed.IsTurnedOn();
#endif
    // write the new on/off value
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, isTurnedOn);

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
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, setLevel);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Update CurrentLevel fail: %x", status);
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

void AppTask::ButtonEventHandler(ButtonId_t btnId, bool btnPressed)
{
    if (!btnPressed)
    {
        return;
    }

    switch (btnId)
    {
    case kButtonId_LightingAction:
        LightingActionButtonEventHandler();
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

void AppTask::InitButtons(void)
{
#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_1, FactoryResetButtonEventHandler);
    sLightingButton.Configure(BUTTON_PORT, BUTTON_PIN_2, LightingActionButtonEventHandler);
    sThreadStartButton.Configure(BUTTON_PORT, BUTTON_PIN_3, StartThreadButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, StartBleAdvButtonEventHandler);
#else
    sFactoryResetButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_1, FactoryResetButtonEventHandler);
    sLightingButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_1, LightingActionButtonEventHandler);
    sThreadStartButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_2, StartThreadButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_2, StartBleAdvButtonEventHandler);
#endif

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sLightingButton);
    ButtonManagerInst().AddButton(sThreadStartButton);
    ButtonManagerInst().AddButton(sBleAdvStartButton);
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
