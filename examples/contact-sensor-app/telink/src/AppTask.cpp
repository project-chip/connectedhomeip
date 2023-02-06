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

#include "ThreadUtil.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

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

#if CONFIG_CHIP_FACTORY_DATA
// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
#endif

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFactoryResetTimer;

LEDWidget sStatusLED;
LEDWidget sContactSensorLED;

Button sFactoryResetButton;
Button sToggleContactStateButton;
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

CHIP_ERROR AppTask::Init(void)
{
    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize status LED
    LEDWidget::InitGpio(LEDS_PORT);
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(SYSTEM_STATE_LED);

    UpdateStatusLED();

    sContactSensorLED.Init(CONTACT_STATE_LED);
    sContactSensorLED.Set(ContactSensorMgr().IsContactClosed());

    UpdateDeviceState();

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Initialize PWM Identify led
    CHIP_ERROR err = sAppTask.mPwmIdentifyLed.Init(&sPwmIdentifySpecGreenLed, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMaxLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }

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
    // static OTATestEventTriggerDelegate testEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    // initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
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

    ContactSensorMgr().SetCallback(OnStateChanged);

    err = ConnectivityMgr().SetBLEDeviceName("TelinkSensor");
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

void AppTask::OnStateChanged(ContactSensorManager::State aState)
{
    // If the contact state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the contact LED if contact sensor is in closed state.
    //  - turn off the lock LED if contact sensor is in opened state.
    if (ContactSensorManager::State::kContactClosed == aState)
    {
        LOG_INF("Contact state changed to CLOSED");
        sContactSensorLED.Set(true);
    }
    else if (ContactSensorManager::State::kContactOpened == aState)
    {
        LOG_INF("Contact state changed to OPEN");
        sContactSensorLED.Set(false);
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }
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

void AppTask::PostContactActionRequest(ContactSensorManager::Action aAction)
{
    AppEvent event;
    event.Type                = AppEvent::kEventType_Contact;
    event.ContactEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler             = ContactActionEventHandler;

    sAppTask.PostEvent(&event);
}

void AppTask::ToggleContactStateButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = ContactActionEventHandler;

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

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = ContactSensorMgr().IsContactClosed();

    ChipLogProgress(NotSpecified, "emberAfWriteAttribute : %d", newValue);

    // write the new boolean state value
    EmberAfStatus status = app::Clusters::BooleanState::Attributes::StateValue::Set(1, newValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating boolean status value %x", status);
    }
}

void AppTask::ContactActionEventHandler(AppEvent * aEvent)
{
    ContactSensorManager::Action action = ContactSensorManager::Action::kInvalid;
    CHIP_ERROR err                      = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "ContactActionEventHandler");

    if (aEvent->Type == AppEvent::kEventType_Contact)
    {
        action = static_cast<ContactSensorManager::Action>(aEvent->ContactEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (ContactSensorMgr().IsContactClosed())
        {
            action = ContactSensorManager::Action::kSignalLost;
        }
        else
        {
            action = ContactSensorManager::Action::kSignalDetected;
        }

        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = ContactSensorManager::Action::kInvalid;
    }

    if (err == CHIP_NO_ERROR)
    {
        ContactSensorMgr().InitiateAction(action);
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

void AppTask::UpdateClusterState(void)
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
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
    sToggleContactStateButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_1, false, ToggleContactStateButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_2, false, StartBleAdvButtonEventHandler);

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sToggleContactStateButton);
    ButtonManagerInst().AddButton(sBleAdvStartButton);
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    bool stateValueAttrValue = 0;

    /* get boolean state attribute value */
    (void) app::Clusters::BooleanState::Attributes::StateValue::Get(1, &stateValueAttrValue);

    ChipLogProgress(NotSpecified, "emberAfReadAttribute : %d", stateValueAttrValue);
    sContactSensorLED.Set(stateValueAttrValue);
}
