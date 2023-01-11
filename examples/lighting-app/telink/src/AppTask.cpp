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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
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

LOG_MODULE_DECLARE(app);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
constexpr int kFactoryResetTriggerTimeout = 2000;
constexpr int kAppEventQueueSize          = 10;
constexpr uint8_t kButtonPushEvent        = 1;
constexpr uint8_t kButtonReleaseEvent     = 0;
constexpr uint8_t kDefaultMinLevel        = 0;
constexpr uint8_t kDefaultMaxLevel        = 254;

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFactoryResetTimer;

LEDWidget sStatusLED;

Button sFactoryResetButton;
Button sLightingButton;
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
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
    return;
}

Identify sIdentify = {
    chip::EndpointId{ 1 },
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

CHIP_ERROR AppTask::Init()
{
    LOG_INF("Current Software Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION,
            CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize status LED
    LEDWidget::InitGpio(SYSTEM_STATE_LED_PORT);
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(SYSTEM_STATE_LED_PIN);

    UpdateStatusLED();

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Init lighting manager
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(1, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(1, &maxLightLevel);

    CHIP_ERROR err = LightingMgr().Init(LIGHTING_PWM_DEVICE, LIGHTING_PWM_CHANNEL, minLightLevel, maxLightLevel, maxLightLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to int lighting manager");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

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
        LOG_ERR("mFactoryDataProvider.GetEnableKey() failed. Could not delegate a test event trigger");
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

    err = ConnectivityMgr().SetBLEDeviceName("TelinkLight");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Fail to set BLE device name");
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

CHIP_ERROR AppTask::StartApp()
{
    CHIP_ERROR err = Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask.Init() failed");
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
    LightingManager::Action_t action = LightingManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightingEvent.Action);
        actor  = aEvent->LightingEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = LightingMgr().IsTurnedOn() ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != LightingManager::INVALID_ACTION && !LightingMgr().InitiateAction(action, actor, 0, NULL))
        LOG_INF("Action is already in progress or active.");
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

    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // Switch context from BLE to Thread
        Internal::BLEManagerImpl sInstance;
        sInstance.SwitchToIeee802154();
        StartDefaultThreadNetwork();
        LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration.");
    }
    else
    {
        LOG_INF("Device is commissioned to a Thread network.");
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
    LOG_INF("BLE advertising start button pressed");

    // Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("Matter service BLE advertising not started - device is commissioned to a Thread network.");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE advertising is already enabled");
        return;
    }

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow() failed");
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

void AppTask::UpdateStatusLED()
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

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_DBG("Turn On Action has been initiated");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_DBG("Turn Off Action has been initiated");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_DBG("Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        LOG_DBG("Turn On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        LOG_DBG("Turn Off Action has been completed");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        LOG_DBG("Level Action has been completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostLightingActionRequest(LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type                 = AppEvent::kEventType_Lighting;
    event.LightingEvent.Action = aAction;
    event.Handler              = LightingActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
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
        LOG_INF("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState()
{
    // write the new on/off value
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(1, LightingMgr().IsTurnedOn());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating on/off cluster failed: %x", status);
    }

    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(1, LightingMgr().GetLevel());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating level cluster failed: %x", status);
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
    sLightingButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_1, false, LightingActionButtonEventHandler);
    sThreadStartButton.Configure(BUTTON_PORT, BUTTON_PIN_3, BUTTON_PIN_2, false, StartThreadButtonEventHandler);
    sBleAdvStartButton.Configure(BUTTON_PORT, BUTTON_PIN_4, BUTTON_PIN_2, false, StartBleAdvButtonEventHandler);

    ButtonManagerInst().AddButton(sFactoryResetButton);
    ButtonManagerInst().AddButton(sLightingButton);
    ButtonManagerInst().AddButton(sThreadStartButton);
    ButtonManagerInst().AddButton(sBleAdvStartButton);
}
