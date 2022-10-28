/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "LEDWidget.h"
#include "PWMDevice.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <system/SystemClock.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {

constexpr int kFactoryResetTriggerTimeout      = 3000;
constexpr int kFactoryResetCancelWindowTimeout = 3000;
constexpr int kAppEventQueueSize               = 10;
constexpr uint8_t kButtonPushEvent             = 1;
constexpr uint8_t kButtonReleaseEvent          = 0;
constexpr EndpointId kLightEndpointId          = 1;
constexpr uint32_t kIdentifyBlinkRateMs        = 500;
constexpr uint8_t kDefaultMinLevel             = 0;
constexpr uint8_t kDefaultMaxLevel             = 254;

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

Identify sIdentify = { kLightEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED };

LEDWidget sStatusLED;
LEDWidget sIdentifyLED;
LEDWidget sUnusedLED;

const struct pwm_dt_spec sLightPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));
bool sIsThreadProvisioned                = false;
bool sIsThreadEnabled                    = false;
bool sHaveBLEConnections                 = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    // Initialize CHIP stack
    LOG_INF("Init CHIP stack");

    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Platform::MemoryInit() failed");
        return err;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().InitChipStack() failed");
        return err;
    }

    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ThreadStackMgr().InitThreadStack() failed");
        return err;
    }

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed");
        return err;
    }

    // Initialize LEDs
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sIdentifyLED.Init(DK_LED3);
    sUnusedLED.Init(DK_LED4);

    UpdateStatusLED();

    // Initialize lighting device (PWM)
    uint8_t minLightLevel = kDefaultMinLevel;
    Clusters::LevelControl::Attributes::MinLevel::Get(kLightEndpointId, &minLightLevel);

    uint8_t maxLightLevel = kDefaultMaxLevel;
    Clusters::LevelControl::Attributes::MaxLevel::Get(kLightEndpointId, &maxLightLevel);

    int ret = mPWMDevice.Init(&sLightPwmDevice, minLightLevel, maxLightLevel, maxLightLevel);
    if (ret != 0)
    {
        return chip::System::MapErrorZephyr(ret);
    }
    mPWMDevice.SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize buttons
    ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

    // Initialize function button timer
    k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

#ifdef CONFIG_MCUMGR_SMP_BT
    // Initialize DFU over SMP
    GetDFUOverSMP().Init(RequestSMPAdvertisingStart);
    GetDFUOverSMP().ConfirmNewImage();
#endif

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
    static OTATestEventTriggerDelegate testEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
    }

    return err;
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    AppEvent event = {};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(&event);
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
        action = GetAppTask().mPWMDevice.IsTurnedOn() ? PWMDevice::OFF_ACTION : PWMDevice::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != PWMDevice::INVALID_ACTION && GetAppTask().mPWMDevice.InitiateAction(action, actor, NULL))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent button_event;
    button_event.Type = AppEvent::kEventType_Button;

    if (LIGHTING_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = LIGHTING_BUTTON;
        button_event.ButtonEvent.Action = kButtonPushEvent;
        button_event.Handler            = LightingActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (FUNCTION_BUTTON_MASK & has_changed)
    {
        button_event.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        button_event.ButtonEvent.Action = (FUNCTION_BUTTON_MASK & button_state) ? kButtonPushEvent : kButtonReleaseEvent;
        button_event.Handler            = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        button_event.ButtonEvent.Action = kButtonPushEvent;
        button_event.Handler            = StartBLEAdvertisementHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(k_timer * timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::IdentifyStartHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_IdentifyStart;
    event.Handler = [](AppEvent *) { sIdentifyLED.Blink(kIdentifyBlinkRateMs); };
    sAppTask.PostEvent(&event);
}

void AppTask::IdentifyStopHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_IdentifyStop;
    event.Handler = [](AppEvent *) { sIdentifyLED.Set(false); };
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

        // Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required.
        sAppTask.StartTimer(kFactoryResetCancelWindowTimeout);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sIdentifyLED.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sIdentifyLED.Blink(500);
        sUnusedLED.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;

        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

#ifdef CONFIG_MCUMGR_SMP_BT
void AppTask::RequestSMPAdvertisingStart(void)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_StartSMPAdvertising;
    event.Handler = [](AppEvent *) { GetDFUOverSMP().StartBLEAdvertising(); };
    sAppTask.PostEvent(&event);
}
#endif

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< kFactoryResetTriggerTimeout)
    // To initiate factory reset: press the FUNCTION_BUTTON for kFactoryResetTriggerTimeout + kFactoryResetCancelWindowTimeout
    // All LEDs start blinking after kFactoryResetTriggerTimeout to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // kFactoryResetCancelWindowTimeout
    if (aEvent->ButtonEvent.Action == kButtonPushEvent)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(kFactoryResetTriggerTimeout);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

#ifdef CONFIG_MCUMGR_SMP_BT
            GetDFUOverSMP().StartServer();
#else
            LOG_INF("Software update is disabled");
#endif
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sIdentifyLED.Set(false);
            sUnusedLED.Set(false);
            UpdateStatusLED();
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            LOG_INF("Factory Reset has been Canceled");
        }
    }
}

void AppTask::StartBLEAdvertisementHandler(AppEvent *)
{
    if (Server::GetInstance().GetFabricTable().FabricCount() != 0)
    {
        LOG_INF("Matter service BLE advertising not started - device is already commissioned");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE advertising is already enabled");
        return;
    }

    if (Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
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

void AppTask::LEDStateUpdateHandler(LEDWidget & ledWidget)
{
    AppEvent event;
    event.Type                          = AppEvent::kEventType_UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = &ledWidget;
    sAppTask.PostEvent(&event);
}

void AppTask::UpdateStatusLED()
{
    /* Update the status LED.
     *
     * If thread and service provisioned, keep the LED On constantly.
     *
     * If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
     * rate of 100ms.
     *
     * Otherwise, blink the LED On for a very short time. */
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        sStatusLED.Set(true);
    }
    else if (sHaveBLEConnections)
    {
        sStatusLED.Blink(100, 100);
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
#ifdef CONFIG_CHIP_NFC_COMMISSIONING
        if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
        {
            if (NFCMgr().IsTagEmulationStarted())
            {
                LOG_INF("NFC Tag emulation is already started");
            }
            else
            {
                ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            }
        }
        else if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
        {
            NFCMgr().StopTagEmulation();
        }
#endif
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
    case DeviceEventType::kDnssdPlatformInitialized:
#if CONFIG_CHIP_OTA_REQUESTOR
        InitBasicOTARequestor();
#endif
        break;
    default:
        break;
    }
}

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(PWMDevice::Action_t aAction, int32_t aActor)
{
    if (aAction == PWMDevice::ON_ACTION)
    {
        LOG_INF("Turn On Action has been initiated");
    }
    else if (aAction == PWMDevice::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been initiated");
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(PWMDevice::Action_t aAction, int32_t aActor)
{
    if (aAction == PWMDevice::ON_ACTION)
    {
        LOG_INF("Turn On Action has been completed");
    }
    else if (aAction == PWMDevice::OFF_ACTION)
    {
        LOG_INF("Turn Off Action has been completed");
    }
    else if (aAction == PWMDevice::LEVEL_ACTION)
    {
        LOG_INF("Level Action has been completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostLightingActionRequest(PWMDevice::Action_t aAction)
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
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, mPWMDevice.IsTurnedOn());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating on/off cluster failed: %x", status);
    }

    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, mPWMDevice.GetLevel());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating level cluster failed: %x", status);
    }
}
