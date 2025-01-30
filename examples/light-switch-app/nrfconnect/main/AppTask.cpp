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
#include "BoardUtil.h"
#include "FabricTableDelegate.h"
#include "LEDUtil.h"
#include "LightSwitch.h"

#include <DeviceInfoProviderImpl.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>

#ifdef CONFIG_CHIP_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/nrfconnect/wifi/NrfWiFiDriver.h>
#endif

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <crypto/PSAOperationalKeystore.h>
#ifdef CONFIG_CHIP_MIGRATE_OPERATIONAL_KEYS_TO_ITS
#include "MigrationManager.h"
#endif
#endif

#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
constexpr EndpointId kLightDimmerSwitchEndpointId  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId = 2;
constexpr EndpointId kLightEndpointId              = 1;

constexpr uint32_t kFactoryResetTriggerTimeout      = 3000;
constexpr uint32_t kFactoryResetCancelWindowTimeout = 3000;
constexpr uint32_t kDimmerTriggeredTimeout          = 500;
constexpr uint32_t kDimmerInterval                  = 300;
constexpr size_t kAppEventQueueSize                 = 10;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

Identify sIdentify = { kLightEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

LEDWidget sStatusLED;
LEDWidget sIdentifyLED;
#if NUMBER_OF_LEDS == 4
FactoryResetLEDsWrapper<2> sFactoryResetLEDs{ { FACTORY_RESET_SIGNAL_LED, FACTORY_RESET_SIGNAL_LED1 } };
#endif

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;
bool sWasDimmerTriggered   = false;

k_timer sFunctionTimer;
k_timer sDimmerPressKeyTimer;
k_timer sDimmerTimer;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#ifdef CONFIG_CHIP_CRYPTO_PSA
chip::Crypto::PSAOperationalKeystore sPSAOperationalKeystore{};
#endif
} // namespace

namespace LedConsts {
constexpr uint32_t kBlinkRate_ms{ 500 };
constexpr uint32_t kIdentifyBlinkRate_ms{ 500 };

namespace StatusLed {
namespace Unprovisioned {
constexpr uint32_t kOn_ms{ 100 };
constexpr uint32_t kOff_ms{ kOn_ms };
} // namespace Unprovisioned
namespace Provisioned {
constexpr uint32_t kOn_ms{ 50 };
constexpr uint32_t kOff_ms{ 950 };
} // namespace Provisioned

} // namespace StatusLed
} // namespace LedConsts

#ifdef CONFIG_CHIP_WIFI
app::Clusters::NetworkCommissioning::Instance sWiFiCommissioningInstance(0, &(NetworkCommissioning::NrfWiFiDriver::Instance()));
#endif

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

#if defined(CONFIG_NET_L2_OPENTHREAD)
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
#elif defined(CONFIG_CHIP_WIFI)
    sWiFiCommissioningInstance.Init();
#else
    return CHIP_ERROR_INTERNAL;
#endif

    LightSwitch::GetInstance().Init(kLightDimmerSwitchEndpointId, kLightGenericSwitchEndpointId);

    // Initialize UI components
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sIdentifyLED.Init(IDENTIFY_LED);
    UpdateStatusLED();

    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return System::MapErrorZephyr(ret);
    }

#ifdef CONFIG_CHIP_OTA_REQUESTOR
    /* OTA image confirmation must be done before the factory data init. */
    OtaConfirmNewImage();
#endif

    // Initialize Timers
    k_timer_init(&sFunctionTimer, AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_init(&sDimmerPressKeyTimer, AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_init(&sDimmerTimer, AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sDimmerTimer, this);
    k_timer_user_data_set(&sDimmerPressKeyTimer, this);
    k_timer_user_data_set(&sFunctionTimer, this);

    // Initialize DFU
#ifdef CONFIG_MCUMGR_TRANSPORT_BT
    GetDFUOverSMP().Init();
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
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    static CommonCaseDeviceServerInitParams initParams;
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
#ifdef CONFIG_CHIP_CRYPTO_PSA
    initParams.operationalKeystore = &sPSAOperationalKeystore;
#endif
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider        = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));
    AppFabricTableDelegate::Init();

#ifdef CONFIG_CHIP_MIGRATE_OPERATIONAL_KEYS_TO_ITS
    err = MoveOperationalKeysFromKvsToIts(sLocalInitData.mServerInitParams->persistentStorageDelegate,
                                          sLocalInitData.mServerInitParams->operationalKeystore);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("MoveOperationalKeysFromKvsToIts() failed");
        return err;
    }
#endif

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
        DispatchEvent(event);
    }

    return CHIP_NO_ERROR;
}

void AppTask::ButtonPushHandler(const AppEvent & event)
{
    if (event.Type == AppEventType::Button)
    {
        switch (event.ButtonEvent.PinNo)
        {
        case FUNCTION_BUTTON:
            Instance().StartTimer(Timer::Function, kFactoryResetTriggerTimeout);
            Instance().mFunction = FunctionEvent::SoftwareUpdate;
            break;
        case
#if NUMBER_OF_BUTTONS == 2
            BLE_ADVERTISEMENT_START_AND_SWITCH_BUTTON:
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled() && Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                break;
            }
#else
            SWITCH_BUTTON:
#endif
            LOG_INF("Button has been pressed, keep in this state for at least 500 ms to change light sensitivity of binded "
                    "lighting devices.");
            Instance().StartTimer(Timer::DimmerTrigger, kDimmerTriggeredTimeout);
            break;
        default:
            break;
        }
    }
}

void AppTask::ButtonReleaseHandler(const AppEvent & event)
{
    if (event.Type == AppEventType::Button)
    {
        switch (event.ButtonEvent.PinNo)
        {
        case FUNCTION_BUTTON:
            if (Instance().mFunction == FunctionEvent::SoftwareUpdate)
            {
                Instance().CancelTimer(Timer::Function);
                Instance().mFunction = FunctionEvent::NoneSelected;

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
                GetDFUOverSMP().StartServer();
                UpdateStatusLED();
#else
                LOG_INF("Software update is disabled");
#endif
            }
            else if (Instance().mFunction == FunctionEvent::FactoryReset)
            {
                UpdateStatusLED();

                Instance().CancelTimer(Timer::Function);
                Instance().mFunction = FunctionEvent::NoneSelected;
                LOG_INF("Factory Reset has been canceled");
            }
            break;
#if NUMBER_OF_BUTTONS == 4
        case SWITCH_BUTTON:
#else
        case BLE_ADVERTISEMENT_START_AND_SWITCH_BUTTON:
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled() && Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                AppEvent buttonEvent;
                buttonEvent.Type               = AppEventType::Button;
                buttonEvent.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_AND_SWITCH_BUTTON;
                buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
                buttonEvent.Handler            = StartBLEAdvertisementHandler;
                PostEvent(buttonEvent);
                break;
            }
#endif
            if (!sWasDimmerTriggered)
            {
                LightSwitch::GetInstance().InitiateActionSwitch(LightSwitch::Action::Toggle);
            }
            Instance().CancelTimer(Timer::Dimmer);
            Instance().CancelTimer(Timer::DimmerTrigger);
            sWasDimmerTriggered = false;
            break;
        default:
            break;
        }
    }
}

void AppTask::TimerEventHandler(const AppEvent & event)
{
    if (event.Type == AppEventType::Timer)
    {
        switch (static_cast<Timer>(event.TimerEvent.TimerType))
        {
        case Timer::Function:
            if (Instance().mFunction == FunctionEvent::SoftwareUpdate)
            {
                LOG_INF("Factory Reset has been triggered. Release button within %u ms to cancel.",
                        kFactoryResetCancelWindowTimeout);
                Instance().StartTimer(Timer::Function, kFactoryResetCancelWindowTimeout);
                Instance().mFunction = FunctionEvent::FactoryReset;

#ifdef CONFIG_STATE_LEDS
                // reset all LEDs to synchronize factory reset blinking
                sStatusLED.Set(false);
                sIdentifyLED.Set(false);
#if NUMBER_OF_LEDS == 4
                sFactoryResetLEDs.Set(false);
#endif

                sStatusLED.Blink(LedConsts::kBlinkRate_ms);
                sIdentifyLED.Blink(LedConsts::kBlinkRate_ms);
#if NUMBER_OF_LEDS == 4
                sFactoryResetLEDs.Blink(LedConsts::kBlinkRate_ms);
#endif
#endif
            }
            else if (Instance().mFunction == FunctionEvent::FactoryReset)
            {
                Instance().mFunction = FunctionEvent::NoneSelected;
                LOG_INF("Factory Reset triggered");
                chip::Server::GetInstance().ScheduleFactoryReset();
            }
            break;
        case Timer::DimmerTrigger:
            LOG_INF("Dimming started...");
            sWasDimmerTriggered = true;
            LightSwitch::GetInstance().InitiateActionSwitch(LightSwitch::Action::On);
            Instance().StartTimer(Timer::Dimmer, kDimmerInterval);
            Instance().CancelTimer(Timer::DimmerTrigger);
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
    event.Type    = AppEventType::IdentifyStart;
    event.Handler = [](const AppEvent &) { sIdentifyLED.Blink(LedConsts::kIdentifyBlinkRate_ms); };
    PostEvent(event);
}

void AppTask::IdentifyStopHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEventType::IdentifyStop;
    event.Handler = [](const AppEvent &) { sIdentifyLED.Set(false); };
    PostEvent(event);
}

void AppTask::StartBLEAdvertisementHandler(const AppEvent &)
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

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        UpdateStatusLED();
#ifdef CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD
        if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
        {
            if (NFCOnboardingPayloadMgr().IsTagEmulationStarted())
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
            NFCOnboardingPayloadMgr().StopTagEmulation();
        }
#endif
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
#if defined(CONFIG_NET_L2_OPENTHREAD)
    case DeviceEventType::kDnssdInitialized:
#if CONFIG_CHIP_OTA_REQUESTOR
        InitBasicOTARequestor();
#endif // CONFIG_CHIP_OTA_REQUESTOR
        break;
    case DeviceEventType::kThreadStateChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
#elif defined(CONFIG_CHIP_WIFI)
    case DeviceEventType::kWiFiConnectivityChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
#if CONFIG_CHIP_OTA_REQUESTOR
        if (event->WiFiConnectivityChange.Result == kConnectivity_Established)
        {
            InitBasicOTARequestor();
        }
#endif // CONFIG_CHIP_OTA_REQUESTOR
#endif
        UpdateStatusLED();
        break;
    default:
        break;
    }
}

void AppTask::UpdateStatusLED()
{
#ifdef CONFIG_STATE_LEDS
#if NUMBER_OF_LEDS == 4
    sFactoryResetLEDs.Set(false);
#endif

    // Update the status LED.
    //
    // If IPv6 network and service provisioned, keep the LED on constantly.
    //
    // If the system has BLE connection(s) up till the stage above, THEN blink the LED at an even
    // rate of 100ms.
    //
    // Otherwise, blink the LED for a very short time.
    if (sIsNetworkProvisioned && sIsNetworkEnabled)
    {
        sStatusLED.Set(true);
    }
    else if (sHaveBLEConnections)
    {
        sStatusLED.Blink(LedConsts::StatusLed::Unprovisioned::kOn_ms, LedConsts::StatusLed::Unprovisioned::kOff_ms);
    }
    else
    {
        sStatusLED.Blink(LedConsts::StatusLed::Provisioned::kOn_ms, LedConsts::StatusLed::Provisioned::kOff_ms);
    }
#endif
}

void AppTask::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
    AppEvent buttonEvent;
    buttonEvent.Type = AppEventType::Button;

    if (FUNCTION_BUTTON_MASK & buttonState & hasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        buttonEvent.Handler            = ButtonPushHandler;
        PostEvent(buttonEvent);
    }
    else if (FUNCTION_BUTTON_MASK & hasChanged)
    {
        buttonEvent.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonReleased);
        buttonEvent.Handler            = ButtonReleaseHandler;
        PostEvent(buttonEvent);
    }

#if NUMBER_OF_BUTTONS == 2
    uint32_t buttonMask           = BLE_ADVERTISEMENT_START_AND_SWITCH_BUTTON_MASK;
    buttonEvent.ButtonEvent.PinNo = BLE_ADVERTISEMENT_START_AND_SWITCH_BUTTON;
#else
    uint32_t buttonMask           = SWITCH_BUTTON_MASK;
    buttonEvent.ButtonEvent.PinNo = SWITCH_BUTTON;
#endif

    if (buttonMask & buttonState & hasChanged)
    {
        buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        buttonEvent.Handler            = ButtonPushHandler;
        PostEvent(buttonEvent);
    }
    else if (buttonMask & hasChanged)
    {
        buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonReleased);
        buttonEvent.Handler            = ButtonReleaseHandler;
        PostEvent(buttonEvent);
    }

#if NUMBER_OF_BUTTONS == 4
    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & hasChanged & buttonState)
    {
        buttonEvent.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        buttonEvent.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        buttonEvent.Handler            = StartBLEAdvertisementHandler;
        PostEvent(buttonEvent);
    }
#endif
}

void AppTask::StartTimer(Timer timer, uint32_t timeoutMs)
{
    switch (timer)
    {
    case Timer::Function:
        k_timer_start(&sFunctionTimer, K_MSEC(timeoutMs), K_NO_WAIT);
        break;
    case Timer::DimmerTrigger:
        k_timer_start(&sDimmerPressKeyTimer, K_MSEC(timeoutMs), K_NO_WAIT);
        break;
    case Timer::Dimmer:
        k_timer_start(&sDimmerTimer, K_MSEC(timeoutMs), K_MSEC(timeoutMs));
        break;
    default:
        break;
    }
}

void AppTask::CancelTimer(Timer timer)
{
    switch (timer)
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

void AppTask::UpdateLedStateEventHandler(const AppEvent & event)
{
    if (event.Type == AppEventType::UpdateLedState)
    {
        event.UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTask::LEDStateUpdateHandler(LEDWidget & aLedWidget)
{
    AppEvent event;
    event.Type                          = AppEventType::UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = &aLedWidget;
    PostEvent(event);
}

void AppTask::FunctionTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    if (timer == &sFunctionTimer)
    {
        event.Type                 = AppEventType::Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::Function;
        event.TimerEvent.Context   = k_timer_user_data_get(timer);
        event.Handler              = TimerEventHandler;
        PostEvent(event);
    }
    if (timer == &sDimmerPressKeyTimer)
    {
        event.Type                 = AppEventType::Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::DimmerTrigger;
        event.TimerEvent.Context   = k_timer_user_data_get(timer);
        event.Handler              = TimerEventHandler;
        PostEvent(event);
    }
    if (timer == &sDimmerTimer)
    {
        event.Type                 = AppEventType::Timer;
        event.TimerEvent.TimerType = (uint8_t) Timer::Dimmer;
        event.TimerEvent.Context   = k_timer_user_data_get(timer);
        event.Handler              = TimerEventHandler;
        PostEvent(event);
    }
}

void AppTask::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void AppTask::DispatchEvent(const AppEvent & event)
{
    if (event.Handler)
    {
        event.Handler(event);
    }
    else
    {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}
