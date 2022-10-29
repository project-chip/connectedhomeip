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
#include "LEDWidget.h"
#include "LightSwitch.h"

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);
namespace {
constexpr EndpointId kLightDimmerSwitchEndpointId  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId = 2;
constexpr EndpointId kLightEndpointId              = 1;

constexpr uint32_t kFactoryResetTriggerTimeout = 3000;
constexpr uint32_t kFactoryResetCancelWindow   = 3000;
constexpr uint32_t kDimmerTriggeredTimeout     = 500;
constexpr uint32_t kDimmerInterval             = 300;
constexpr uint32_t kIdentifyBlinkRateMs        = 500;
constexpr size_t kAppEventQueueSize            = 10;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

Identify sIdentify = { kLightEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED };

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

LEDWidget sStatusLED;
LEDWidget sBleLED;
LEDWidget sIdentifyLED;
LEDWidget sUnusedLED;

bool sIsThreadProvisioned    = false;
bool sIsThreadEnabled        = false;
bool sIsThreadBLEAdvertising = false;
#ifdef CONFIG_MCUMGR_SMP_BT
bool sIsSMPAdvertising = false;
#endif
bool sHaveBLEConnections = false;
bool sWasDimmerTriggered = false;

k_timer sFunctionTimer;
k_timer sDimmerPressKeyTimer;
k_timer sDimmerTimer;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

} /* namespace */

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    // Initialize CHIP
    LOG_INF("Init CHIP stack");

    CHIP_ERROR err = Platform::MemoryInit();
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
        LOG_ERR("ThreadStackMgr().InitThreadStack() failed: %s", ErrorStr(err));
        return err;
    }

#ifdef CONFIG_OPENTHREAD_MTD_SED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed: %s", ErrorStr(err));
        return err;
    }

    LightSwitch::GetInstance().Init(kLightDimmerSwitchEndpointId, kLightGenericSwitchEndpointId);

    // Initialize UI components
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(SYSTEM_STATE_LED);
    sBleLED.Init(DFU_BLE_LED);
    sIdentifyLED.Init(IDENTIFY_LED);
    sUnusedLED.Init(DK_LED4);
    UpdateStatusLED();

    int ret = dk_buttons_init(ButtonEventHandler);

    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return System::MapErrorZephyr(ret);
    }

    // Initialize Timers
    k_timer_init(&sFunctionTimer, AppTask::TimerEventHandler, nullptr);
    k_timer_init(&sDimmerPressKeyTimer, AppTask::TimerEventHandler, nullptr);
    k_timer_init(&sDimmerTimer, AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sDimmerTimer, this);
    k_timer_user_data_set(&sDimmerPressKeyTimer, this);
    k_timer_user_data_set(&sFunctionTimer, this);

    // Initialize DFU
#ifdef CONFIG_MCUMGR_SMP_BT
    GetDFUOverSMP().Init(RequestSMPAdvertisingStart);
    GetDFUOverSMP().ConfirmNewImage();
#endif

    // Print initial configs
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
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point
    // to avoid data races between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
        return err;
    }

    return err;
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    AppEvent event{};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(&event);
    }

    return CHIP_NO_ERROR;
}

void AppTask::ButtonPushHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        switch (aEvent->ButtonEvent.PinNo)
        {
        case FUNCTION_BUTTON:
            sAppTask.StartTimer(Timer::Function, kFactoryResetTriggerTimeout);
            sAppTask.mFunction = TimerFunction::SoftwareUpdate;
            break;
        case DIMMER_SWITCH_BUTTON:
            LOG_INF("Button has been pressed, keep in this state for at least 500 ms to change light sensitivity of binded "
                    "lighting devices.");
            sAppTask.StartTimer(Timer::DimmerTrigger, kDimmerTriggeredTimeout);
            break;
        case GENERIC_SWITCH_BUTTON:
            LOG_INF("GenericSwitch: InitialPress");
            LightSwitch::GetInstance().GenericSwitchInitialPress();
            break;
        default:
            break;
        }
    }
}

void AppTask::ButtonReleaseHandler(AppEvent * aEvent)
{

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        switch (aEvent->ButtonEvent.PinNo)
        {
        case FUNCTION_BUTTON:
            if (sAppTask.mFunction == TimerFunction::SoftwareUpdate)
            {
                sAppTask.CancelTimer(Timer::Function);
                sAppTask.mFunction = TimerFunction::NoneSelected;

#ifdef CONFIG_MCUMGR_SMP_BT
                GetDFUOverSMP().StartServer();
                sIsSMPAdvertising = true;
                UpdateStatusLED();
#else
                LOG_INF("Software update is disabled");
#endif
            }
            else if (sAppTask.mFunction == TimerFunction::FactoryReset)
            {
                UpdateStatusLED();

                sAppTask.CancelTimer(Timer::Function);
                sAppTask.mFunction = TimerFunction::NoneSelected;
                LOG_INF("Factory Reset has been canceled");
            }
            break;
        case DIMMER_SWITCH_BUTTON:
            if (!sWasDimmerTriggered)
            {
                LightSwitch::GetInstance().InitiateActionSwitch(LightSwitch::Action::Toggle);
            }
            sAppTask.CancelTimer(Timer::Dimmer);
            sAppTask.CancelTimer(Timer::DimmerTrigger);
            sWasDimmerTriggered = false;
            break;
        case GENERIC_SWITCH_BUTTON:
            LOG_INF("GenericSwitch: ShortRelease");
            LightSwitch::GetInstance().GenericSwitchReleasePress();
            break;
        default:
            break;
        }
    }
}

void AppTask::TimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Timer)
    {
        switch ((Timer) aEvent->TimerEvent.TimerType)
        {
        case Timer::Function:
            if (sAppTask.mFunction == TimerFunction::SoftwareUpdate)
            {
                LOG_INF("Factory Reset has been triggered. Release button within %u ms to cancel.", kFactoryResetCancelWindow);
                sAppTask.StartTimer(Timer::Function, kFactoryResetCancelWindow);
                sAppTask.mFunction = TimerFunction::FactoryReset;

#ifdef CONFIG_STATE_LEDS
                // reset all LEDs to synchronize factory reset blinking
                sStatusLED.Set(false);
                sIdentifyLED.Set(false);
                sBleLED.Set(false);
                sUnusedLED.Set(false);

                sStatusLED.Blink(500);
                sIdentifyLED.Blink(500);
                sBleLED.Blink(500);
                sUnusedLED.Blink(500);
#endif
            }
            else if (sAppTask.mFunction == TimerFunction::FactoryReset)
            {
                sAppTask.mFunction = TimerFunction::NoneSelected;
                LOG_INF("Factory Reset triggered");
                ConfigurationMgr().InitiateFactoryReset();
            }
            break;
        case Timer::DimmerTrigger:
            LOG_INF("Dimming started...");
            sWasDimmerTriggered = true;
            LightSwitch::GetInstance().InitiateActionSwitch(LightSwitch::Action::On);
            sAppTask.StartTimer(Timer::Dimmer, kDimmerInterval);
            sAppTask.CancelTimer(Timer::DimmerTrigger);
            break;
        case Timer::Dimmer:
            LightSwitch::GetInstance().DimmerChangeBrightness();
            break;
        default:
            break;
        }
    }
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

void AppTask::StartBLEAdvertisingHandler(AppEvent * aEvent)
{
    /// Don't allow on starting Matter service BLE advertising after Thread provisioning.
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

    LOG_INF("Enabling BLE advertising...");
    if (Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow() failed");
    }
}

void AppTask::ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t /* arg */)
{
    switch (aEvent->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sIsThreadBLEAdvertising = true;
        UpdateStatusLED();
#ifdef CONFIG_CHIP_NFC_COMMISSIONING
        if (aEvent->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
        {
            if (NFCMgr().IsTagEmulationStarted())
            {
                LOG_INF("NFC Tag emulation is already started");
            }
            else
            {
                ShareQRCodeOverNFC(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));
            }
        }
        else if (aEvent->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
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
        if ((ConnectivityMgr().NumBLEConnections() == 0) && (!sIsThreadProvisioned || !sIsThreadEnabled))
        {
            LOG_ERR("Commissioning with a Thread network has not been done. An error occurred...");
            sIsThreadBLEAdvertising = false;
            sHaveBLEConnections     = false;
            UpdateStatusLED();
        }
        break;
    }
}

void AppTask::UpdateStatusLED()
{
#ifdef CONFIG_STATE_LEDS
    sUnusedLED.Set(false);

    // Status LED indicates:
    // - blinking 1 s - advertising, ready to commission
    // - blinking 200 ms - commissioning in progress
    // - constant lightning means commissioned with Thread network
    if (sIsThreadBLEAdvertising && !sHaveBLEConnections)
    {
        sStatusLED.Blink(50, 950);
    }
    else if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        sStatusLED.Set(true);
    }
    else if (sHaveBLEConnections)
    {
        sStatusLED.Blink(30, 170);
    }
    else
    {
        sStatusLED.Set(false);
    }

// Ble LED indicates BLE connectivity:
//- blinking 200 ms means BLE advertising
#ifdef CONFIG_MCUMGR_SMP_BT
    if (sIsSMPAdvertising)
    {
        sBleLED.Blink(30, 170);
    }
    else
    {
        sBleLED.Set(false);
    }
#else
    sBleLED.Set(false);
#endif
#endif
}

void AppTask::ButtonEventHandler(uint32_t aButtonState, uint32_t aHasChanged)
{

    AppEvent buttonEvent;
    buttonEvent.Type = AppEvent::kEventType_Button;

    if (FUNCTION_BUTTON_MASK & aButtonState & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonPushEvent;
        buttonEvent.Handler            = ButtonPushHandler;
        sAppTask.PostEvent(&buttonEvent);
    }
    else if (FUNCTION_BUTTON_MASK & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonReleaseEvent;
        buttonEvent.Handler            = ButtonReleaseHandler;
        sAppTask.PostEvent(&buttonEvent);
    }

    if (DIMMER_SWITCH_BUTTON_MASK & aButtonState & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = DIMMER_SWITCH_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonPushEvent;
        buttonEvent.Handler            = ButtonPushHandler;
        sAppTask.PostEvent(&buttonEvent);
    }
    else if (DIMMER_SWITCH_BUTTON_MASK & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = DIMMER_SWITCH_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonReleaseEvent;
        buttonEvent.Handler            = ButtonReleaseHandler;
        sAppTask.PostEvent(&buttonEvent);
    }

    if (GENERIC_SWITCH_BUTTON_MASK & aButtonState & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = GENERIC_SWITCH_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonPushEvent;
        buttonEvent.Handler            = ButtonPushHandler;
        sAppTask.PostEvent(&buttonEvent);
    }
    else if (GENERIC_SWITCH_BUTTON_MASK & aHasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = GENERIC_SWITCH_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonReleaseEvent;
        buttonEvent.Handler            = ButtonReleaseHandler;
        sAppTask.PostEvent(&buttonEvent);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & aHasChanged & aButtonState)
    {
        buttonEvent.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        buttonEvent.ButtonEvent.Action = AppEvent::kButtonPushEvent;
        buttonEvent.Handler            = StartBLEAdvertisingHandler;
        sAppTask.PostEvent(&buttonEvent);
    }
}

void AppTask::StartTimer(Timer aTimer, uint32_t aTimeoutMs)
{
    switch (aTimer)
    {
    case Timer::Function:
        k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutMs), K_NO_WAIT);
        break;
    case Timer::DimmerTrigger:
        k_timer_start(&sDimmerPressKeyTimer, K_MSEC(aTimeoutMs), K_NO_WAIT);
        break;
    case Timer::Dimmer:
        k_timer_start(&sDimmerTimer, K_MSEC(aTimeoutMs), K_MSEC(aTimeoutMs));
        break;
    default:
        break;
    }
}

void AppTask::CancelTimer(Timer aTimer)
{
    switch (aTimer)
    {
    case Timer::Function:
        k_timer_stop(&sFunctionTimer);
        break;
    case Timer::DimmerTrigger:
        k_timer_stop(&sDimmerPressKeyTimer);
        break;
    case Timer::Dimmer:
        k_timer_stop(&sDimmerTimer);
        break;
    default:
        break;
    }
}

void AppTask::UpdateLedStateEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_UpdateLedState)
    {
        aEvent->UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTask::LEDStateUpdateHandler(LEDWidget & aLedWidget)
{
    AppEvent event;
    event.Type                          = AppEvent::kEventType_UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = &aLedWidget;
    sAppTask.PostEvent(&event);
}

void AppTask::TimerEventHandler(k_timer * aTimer)
{
    AppEvent event;
    if (aTimer == &sFunctionTimer)
    {
        event.Type                 = AppEvent::kEventType_Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::Function;
        event.TimerEvent.Context   = k_timer_user_data_get(aTimer);
        event.Handler              = TimerEventHandler;
        sAppTask.PostEvent(&event);
    }
    if (aTimer == &sDimmerPressKeyTimer)
    {
        event.Type                 = AppEvent::kEventType_Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::DimmerTrigger;
        event.TimerEvent.Context   = k_timer_user_data_get(aTimer);
        event.Handler              = TimerEventHandler;
        sAppTask.PostEvent(&event);
    }
    if (aTimer == &sDimmerTimer)
    {
        event.Type                 = AppEvent::kEventType_Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::Dimmer;
        event.TimerEvent.Context   = k_timer_user_data_get(aTimer);
        event.Handler              = TimerEventHandler;
        sAppTask.PostEvent(&event);
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
