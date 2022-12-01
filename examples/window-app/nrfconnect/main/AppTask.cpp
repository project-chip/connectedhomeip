/*
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
#include "LEDUtil.h"
#include "WindowCovering.h"

#include <DeviceInfoProviderImpl.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include "OTAUtil.h"
#endif

#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
namespace {
constexpr uint32_t kFactoryResetTriggerTimeout      = 3000;
constexpr uint32_t kFactoryResetCancelWindowTimeout = 3000;
constexpr size_t kAppEventQueueSize                 = 10;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

Identify sIdentify = { WindowCovering::Endpoint(), AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED };

LEDWidget sStatusLED;
LEDWidget sIdentifyLED;
FactoryResetLEDsWrapper<1> sFactoryResetLEDs{ { FACTORY_RESET_SIGNAL_LED } };

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;
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

#if defined(CONFIG_NET_L2_OPENTHREAD)
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ThreadStackMgr().InitThreadStack() failed");
        return err;
    }

#if CONFIG_CHIP_THREAD_SSED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
#elif CONFIG_OPENTHREAD_MTD_SED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed");
        return err;
    }
#elif !defined(CONFIG_WIFI_NRF700X)
    return CHIP_ERROR_INTERNAL;
#endif

    // Initialize LEDs
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sIdentifyLED.Init(LIFT_STATE_LED);
    sIdentifyLED.Set(false);

    UpdateStatusLED();

    // Initialize buttons
    auto ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

    // Initialize function timer user data
    k_timer_init(&sFunctionTimer, &AppTask::FunctionTimerTimeoutCallback, nullptr);
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
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
        return err;
    }

    WindowCovering::Instance().PositionLEDUpdate(WindowCovering::MoveType::LIFT);
    WindowCovering::Instance().PositionLEDUpdate(WindowCovering::MoveType::TILT);

    return CHIP_NO_ERROR;
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

void AppTask::IdentifyStartHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEventType::IdentifyStart;
    event.Handler = [](const AppEvent &) {
        WindowCovering::Instance().GetLiftIndicator().SuppressOutput();
        sIdentifyLED.Blink(LedConsts::kIdentifyBlinkRate_ms);
    };
    PostEvent(event);
}

void AppTask::IdentifyStopHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEventType::IdentifyStop;
    event.Handler = [](const AppEvent &) {
        sIdentifyLED.Set(false);
        WindowCovering::Instance().GetLiftIndicator().ApplyLevel();
    };
    PostEvent(event);
}

void AppTask::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
    AppEvent event;
    event.Type = AppEventType::Button;

    if (FUNCTION_BUTTON_MASK & hasChanged)
    {
        event.ButtonEvent.PinNo = FUNCTION_BUTTON;
        event.ButtonEvent.Action =
            static_cast<uint8_t>((FUNCTION_BUTTON_MASK & buttonState) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased);
        event.Handler = FunctionHandler;
        PostEvent(event);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & buttonState & hasChanged)
    {
        event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        event.Handler            = StartBLEAdvertisementHandler;
        PostEvent(event);
    }

    if (OPEN_BUTTON_MASK & hasChanged)
    {
        event.ButtonEvent.PinNo = OPEN_BUTTON;
        event.ButtonEvent.Action =
            static_cast<uint8_t>((OPEN_BUTTON_MASK & buttonState) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased);
        event.Handler = OpenHandler;
        PostEvent(event);
    }

    if (CLOSE_BUTTON_MASK & hasChanged)
    {
        event.ButtonEvent.PinNo = CLOSE_BUTTON;
        event.ButtonEvent.Action =
            static_cast<uint8_t>((CLOSE_BUTTON_MASK & buttonState) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased);
        event.Handler = CloseHandler;
        PostEvent(event);
    }
}

#ifdef CONFIG_MCUMGR_SMP_BT
void AppTask::RequestSMPAdvertisingStart(void)
{
    AppEvent event;
    event.Type    = AppEventType::StartSMPAdvertising;
    event.Handler = [](const AppEvent &) { GetDFUOverSMP().StartBLEAdvertising(); };
    PostEvent(event);
}
#endif

void AppTask::FunctionTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type               = AppEventType::Timer;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = FunctionTimerEventHandler;
    PostEvent(event);
}

void AppTask::FunctionTimerEventHandler(const AppEvent & event)
{
    if (event.Type != AppEventType::Timer)
        return;

    // If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset
    if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

        // Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required.
        Instance().StartTimer(kFactoryResetCancelWindowTimeout);
        Instance().mFunction = FunctionEvent::FactoryReset;

#ifdef CONFIG_STATE_LEDS
        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sFactoryResetLEDs.Set(false);

        sStatusLED.Blink(LedConsts::kBlinkRate_ms);
        sFactoryResetLEDs.Blink(LedConsts::kBlinkRate_ms);
#endif
    }
    else if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::FactoryReset)
    {
        // Actually trigger Factory Reset
        Instance().mFunction = FunctionEvent::NoneSelected;

        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(const AppEvent & event)
{
    if (event.ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed))
    {
        if (!Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::NoneSelected)
        {
            Instance().StartTimer(kFactoryResetTriggerTimeout);

            Instance().mFunction = FunctionEvent::SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::SoftwareUpdate)
        {
            Instance().CancelTimer();

#ifdef CONFIG_MCUMGR_SMP_BT
            GetDFUOverSMP().StartServer();
#else
            LOG_INF("Software update is disabled");
#endif
            Instance().mFunction = FunctionEvent::NoneSelected;
        }
        else if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::FactoryReset)
        {
            sFactoryResetLEDs.Set(false);

            UpdateStatusLED();
            Instance().CancelTimer();
            Instance().mFunction = FunctionEvent::NoneSelected;
            LOG_INF("Factory Reset has been Canceled");
        }
        else if (Instance().mFunctionTimerActive)
        {
            CancelTimer();
            Instance().mFunction = FunctionEvent::NoneSelected;
        }
    }
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

void AppTask::OpenHandler(const AppEvent & event)
{
    if (event.ButtonEvent.PinNo != OPEN_BUTTON || Instance().mFunction != FunctionEvent::NoneSelected)
        return;

    if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed))
    {
        Instance().mOpenButtonIsPressed = true;
        if (Instance().mCloseButtonIsPressed)
        {
            Instance().ToggleMoveType();
        }
    }
    else if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonReleased))
    {
        if (!Instance().mCloseButtonIsPressed)
        {
            if (!Instance().mMoveTypeRecentlyChanged)
            {
                WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingUpOrOpen);
            }
            else
            {
                Instance().mMoveTypeRecentlyChanged = false;
            }
        }
        Instance().mOpenButtonIsPressed = false;
    }
}

void AppTask::CloseHandler(const AppEvent & event)
{
    if (event.ButtonEvent.PinNo != CLOSE_BUTTON || Instance().mFunction != FunctionEvent::NoneSelected)
        return;

    if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed))
    {
        Instance().mCloseButtonIsPressed = true;
        if (Instance().mOpenButtonIsPressed)
        {
            Instance().ToggleMoveType();
        }
    }
    else if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonReleased))
    {
        if (!Instance().mOpenButtonIsPressed)
        {
            if (!Instance().mMoveTypeRecentlyChanged)
            {
                WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingDownOrClose);
            }
            else
            {
                Instance().mMoveTypeRecentlyChanged = false;
            }
        }
        Instance().mCloseButtonIsPressed = false;
    }
}

void AppTask::ToggleMoveType()
{
    if (WindowCovering::Instance().GetMoveType() == WindowCovering::MoveType::LIFT)
    {
        WindowCovering::Instance().SetMoveType(WindowCovering::MoveType::TILT);
        LOG_INF("Window covering move: tilt");
    }
    else
    {
        WindowCovering::Instance().SetMoveType(WindowCovering::MoveType::LIFT);
        LOG_INF("Window covering move: lift");
    }
    mMoveTypeRecentlyChanged = true;
}

void AppTask::UpdateLedStateEventHandler(const AppEvent & event)
{
    if (event.Type == AppEventType::UpdateLedState)
    {
        event.UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTask::LEDStateUpdateHandler(LEDWidget & ledWidget)
{
    AppEvent event;
    event.Type                          = AppEventType::UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = &ledWidget;
    PostEvent(event);
}

void AppTask::UpdateStatusLED()
{
#ifdef CONFIG_STATE_LEDS
    // Update the status LED.
    //
    // If thread and service provisioned, keep the LED On constantly.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
    // rate of 100ms.
    //
    // Otherwise, blink the LED On for a very short time.
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
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
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
    Instance().mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t timeoutMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(timeoutMs), K_NO_WAIT);
    Instance().mFunctionTimerActive = true;
}

void AppTask::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT))
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
