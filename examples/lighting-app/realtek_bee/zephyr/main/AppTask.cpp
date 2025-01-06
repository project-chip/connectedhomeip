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
#include "matter_button.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAInitializer.h>
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {

constexpr int kFactoryResetTriggerTimeout      = 3000;
constexpr int kFactoryResetCancelWindowTimeout = 3000;
constexpr int kAppEventQueueSize               = 10;
constexpr EndpointId kLightEndpointId          = 1;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

Identify sIdentify = { kLightEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

LEDWidget sStatusLED;
LEDWidget sIdentifyLED;

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static bool isOTAInitialized = false;
constexpr uint32_t kInitOTARequestorDelaySec = 3;

void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "InitOTARequestorHandler");
    OTAInitializer::Instance().InitOTARequestor();
}
#endif
} // namespace

namespace LedConsts {
constexpr uint32_t kBlinkRate_ms{ 500 };
constexpr uint32_t kIdentifyBlinkRate_ms{ 500 };

namespace StatusLed {
namespace Unprovisioned {
constexpr uint32_t kOn_ms{ 100 };
constexpr uint32_t kOff_ms{ kOn_ms };
} /* namespace Unprovisioned */
namespace Provisioned {
constexpr uint32_t kOn_ms{ 50 };
constexpr uint32_t kOff_ms{ 950 };
} /* namespace Provisioned */

} /* namespace StatusLed */
} /* namespace LedConsts */

CHIP_ERROR AppTask::Init()
{
    // Initialize LEDs
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {}));
    sIdentifyLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {}));
    sIdentifyLED.Set(false);

    UpdateStatusLED();

    // Initialize buttons
    int ret = matter_button_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("matter_button_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

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

#if CHIP_ENABLE_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ThreadStackMgr().InitThreadStack() failed");
        return err;
    }

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#elif CHIP_DEVICE_CONFIG_THREAD_SSED
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#endif

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed");
        return err;
    }
#else
    return CHIP_ERROR_INTERNAL;
#endif // CHIP_ENABLE_OPENTHREAD

    // Initialize function button timer
    k_timer_init(&sFunctionTimer, &AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

#ifdef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* OTA image confirmation must be done before the factory data init. */
    //OtaConfirmNewImage();
#endif

    // Initialize CHIP server
#if CONFIG_CHIP_FACTORY_DATA
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
#else
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    initParams.InitializeStaticResourcesBeforeServerInit();
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    ChipLogProgress(DeviceLayer, "Start BLE Adv");
    if (CONFIG_NETWORK_LAYER_BLE)
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    }
#endif

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
    }

#if CONFIG_ENABLE_CHIP_SHELL
    chip::Shell::Engine::Root().Init();
    chip::Shell::Engine::Root().RunMainLoop();
#endif

    return err;
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    AppEvent event = {};

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
    };
    PostEvent(event);
}

LEDWidget & AppTask::GetLightingDevice(void)
{
    return sIdentifyLED;
}

void AppTask::ButtonEventHandler(uint8_t buttonIdx, uint8_t state)
{
    AppEvent button_event;

    button_event.Type = AppEventType::Button;
    button_event.ButtonEvent.ButtonIdx = buttonIdx;
    button_event.ButtonEvent.Action = 
        static_cast<uint8_t>((state == MATTER_BUTTON_STATE_PRESS) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased);
    button_event.Handler = FunctionHandler;
    PostEvent(button_event);
}

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
    {
        return;
    }

    // If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset
    if (Instance().mFunction == FunctionEvent::SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

        // Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required.
        Instance().StartTimer(kFactoryResetCancelWindowTimeout);
        Instance().mFunction = FunctionEvent::FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sStatusLED.Blink(LedConsts::kBlinkRate_ms);
    }
    else if (Instance().mFunction == FunctionEvent::FactoryReset)
    {
        // Actually trigger Factory Reset
        Instance().mFunction = FunctionEvent::NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(const AppEvent & event)
{
    LOG_INF("Button state changed: index %d, action %d", event.ButtonEvent.ButtonIdx, event.ButtonEvent.Action);

    switch(event.ButtonEvent.ButtonIdx)
    {
    case 0:
        {
            // To trigger software update: press the FUNCTION_BUTTON button briefly (< kFactoryResetTriggerTimeout)
            // To initiate factory reset: press the FUNCTION_BUTTON for kFactoryResetTriggerTimeout + kFactoryResetCancelWindowTimeout
            // All LEDs start blinking after kFactoryResetTriggerTimeout to signal factory reset has been initiated.
            // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
            // kFactoryResetCancelWindowTimeout
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
                    Instance().mFunction = FunctionEvent::NoneSelected;

                    //TODO: rock: software reset?
                    LOG_INF("Software update is disabled");
                }
                else if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::FactoryReset)
                {
                    UpdateStatusLED();
                    Instance().CancelTimer();
                    Instance().mFunction = FunctionEvent::NoneSelected;
                    LOG_INF("Factory Reset has been Canceled");
                }
            }
        }
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
    // Update the status LED.
    //
    // If IPv6 network and service provisioned, keep the LED On constantly.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
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
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;

    case DeviceEventType::kServerReady:
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        if (!isOTAInitialized)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
            isOTAInitialized = true;
        }
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

void AppTask::StartTimer(uint32_t timeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(timeoutInMs), K_NO_WAIT);
    mFunctionTimerActive = true;
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

void AppTask::UpdateClusterState()
{
    SystemLayer().ScheduleLambda([this] {
        // write the new on/off value
        Protocols::InteractionModel::Status status =
            Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, sIdentifyLED.IsTurnedOn());

        if (status != Protocols::InteractionModel::Status::Success)
        {
            LOG_ERR("Updating on/off cluster failed: %x", to_underlying(status));
        }
    });
}
