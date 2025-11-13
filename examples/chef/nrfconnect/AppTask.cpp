/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "FabricTableDelegate.h"
#include "LEDUtil.h"

#include <DeviceInfoProviderImpl.h>
#include <lib/support/CodeUtils.h> // For chip::to_underlying

#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/Instance.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#ifdef CONFIG_NET_L2_OPENTHREAD
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#endif
#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <crypto/PSAOperationalKeystore.h>
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
constexpr size_t kAppEventQueueSize      = 10;
constexpr EndpointId kIdentifyEndpointId = 1;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
Identify sIdentify = { kIdentifyEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

LEDWidget sStatusLED;
LEDWidget sIdentifyLED;

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;

#ifdef CONFIG_CHIP_CRYPTO_PSA
chip::Crypto::PSAOperationalKeystore sPSAOperationalKeystore{};
#endif

#ifdef CONFIG_CHIP_ICD_DSLS_SUPPORT
bool sIsSitModeRequested = false;
#endif

#ifdef CONFIG_NET_L2_OPENTHREAD
Clusters::NetworkCommissioning::InstanceAndDriver<NetworkCommissioning::GenericThreadDriver> sThreadNetworkDriver(0 /*endpointId*/);
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

extern void ApplicationInit();

void AppTask::MsgQConsume(intptr_t)
{
    AppEvent event{};

    while (true)
    {
        if (k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT) != 0)
        {
            break;
        }
        ChipLogProgress(AppServer, "MsgQ get event.Type = %u", chip::to_underlying(event.Type));
        DispatchEvent(event);
    }
}
void AppTask::InitServer(intptr_t)
{

    ApplicationInit();
}

CHIP_ERROR AppTask::Init()
{
    // Initialize CHIP stack
    ChipLogProgress(AppServer, "Init CHIP stack");

    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Platform::MemoryInit() failed");
        return err;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PlatformMgr().InitChipStack() failed");
        return err;
    }

    // Network connectivity
#if defined(CONFIG_NET_L2_OPENTHREAD)
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ThreadStackMgr().InitThreadStack() failed");
        return err;
    }

#ifdef CONFIG_OPENTHREAD_MTD_SED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ConnectivityMgr().SetThreadDeviceType() failed");
        return err;
    }

    sThreadNetworkDriver.Init();
#else
    return CHIP_ERROR_INTERNAL;
#endif // CONFIG_NET_L2_OPENTHREAD

    // Initialize LEDs
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sIdentifyLED.Init(IDENTIFY_STATE_LED);
    sIdentifyLED.Set(false);

    UpdateStatusLED();

    // Initialize buttons
    auto ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        ChipLogError(AppServer, "dk_buttons_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

    // Initialize timer user data
    k_timer_init(&sFunctionTimer, &AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

    // Device Attestation & Onboarding codes
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(kExtDiscoveryTimeoutSecs);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY */

    // Start IM server
    static CommonCaseDeviceServerInitParams initParams;
#ifdef CONFIG_CHIP_CRYPTO_PSA
    initParams.operationalKeystore = &sPSAOperationalKeystore;
#endif
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    ConfigurationMgr().LogDeviceConfig();
    // When SoftAP support becomes available, it should be added here.
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PlatformMgr().StartEventLoopTask() failed");
    }

    // Starts commissioning window automatically. Starts BLE advertising when BLE enabled
    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OpenBasicCommissioningWindow() failed");
    }

    return err;
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AppTask::InitServer);

    return CHIP_NO_ERROR;
}

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
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
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

void AppTask::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
    AppEvent button_event;
    button_event.Type = AppEventType::Button;

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & buttonState & hasChanged)
    {
        ChipLogProgress(AppServer, "BLE_ADVERTISEMENT_START_BUTTON pushed, buttonState=%u", buttonState);
        button_event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        button_event.Handler            = StartBLEAdvertisementHandler;
        PostEvent(button_event);
    }

    if (FUNCTION_BUTTON_MASK & hasChanged)
    {
        ChipLogProgress(AppServer, "ICD_FUNCTION_BUTTON pushed, buttonState=%u", buttonState);
        button_event.ButtonEvent.PinNo = FUNCTION_BUTTON;
        button_event.ButtonEvent.Action =
            static_cast<uint8_t>((FUNCTION_BUTTON_MASK & buttonState) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased);
        button_event.Handler = FunctionHandler;
        PostEvent(button_event);
    }

#ifdef CONFIG_CHIP_ICD_DSLS_SUPPORT
    if (ICD_DSLS_BUTTON_MASK & buttonState & hasChanged)
    {
        ChipLogProgress(AppServer, "ICD_DSLS_BUTTON pushed, buttonState=%u", buttonState);
        button_event.ButtonEvent.PinNo  = ICD_DSLS_BUTTON;
        button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        button_event.Handler            = IcdDslsEventHandler;
        PostEvent(button_event);
    }
#endif

    if (ICD_UAT_BUTTON_MASK & hasChanged)
    {
        ChipLogProgress(AppServer, "ICD_UAT_BUTTON pushed, buttonState=%u", buttonState);
        button_event.ButtonEvent.PinNo  = ICD_UAT_BUTTON;
        button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
        button_event.Handler            = IcdUatEventHandler;
        PostEvent(button_event);
    }
}

#ifdef CONFIG_CHIP_ICD_DSLS_SUPPORT
void AppTask::IcdDslsEventHandler(const AppEvent &)
{
    if (sIsSitModeRequested)
    {
        ChipLogProgress(AppServer, "IcdDslsEventHandler: NotifySITModeRequestWithdrawal");
        PlatformMgr().ScheduleWork([](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestWithdrawal(); }, 0);
        sIsSitModeRequested = false;
    }
    else
    {
        ChipLogProgress(AppServer, "IcdDslsEventHandler: NotifySITModeRequestNotification");
        PlatformMgr().ScheduleWork([](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestNotification(); },
                                   0);
        sIsSitModeRequested = true;
    }
}
#endif

void AppTask::IcdUatEventHandler(const AppEvent &)
{
#ifdef CONFIG_CHIP_ICD_UAT_SUPPORT
    // Temporarily claim network activity, until we implement a "user trigger" reason for ICD wakeups.
    PlatformMgr().ScheduleWork([](intptr_t) { ICDNotifier::GetInstance().NotifyNetworkActivityNotification(); });
#endif
}

void AppTask::FunctionTimerTimeoutCallback(k_timer * timer)
{
    ChipLogProgress(AppServer, "AppTask::FunctionTimerTimeoutCallback");
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
    ChipLogProgress(AppServer, "AppTask::FunctionTimerEventHandler");
    if (event.Type != AppEventType::Timer || !Instance().mFunctionTimerActive)
    {
        return;
    }
    // Leave FunctionHandler for future usage
}

void AppTask::FunctionHandler(const AppEvent & event)
{
    if (event.ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // Leave FunctionHandler for future usage
}

void AppTask::StartBLEAdvertisementHandler(const AppEvent &)
{
    if (Server::GetInstance().GetFabricTable().FabricCount() != 0)
    {
        ChipLogProgress(AppServer, "Matter service BLE advertising not started - device is already commissioned");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        ChipLogProgress(AppServer, "BLE advertising is already enabled");
        return;
    }

    if (Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OpenBasicCommissioningWindow() failed");
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
    // If the system has BLE connection(s) until the stage above, THEN blink the LED at an even
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
#if defined(CONFIG_NET_L2_OPENTHREAD)
    case DeviceEventType::kThreadStateChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
#endif // CONFIG_NET_L2_OPENTHREAD
    default:
        break;
    }
}

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    Instance().mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    Instance().mFunctionTimerActive = true;
}

void AppTask::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT) != 0)
    {
        ChipLogProgress(AppServer, "Failed to post event to app task event queue");
        return;
    }
    ChipLogProgress(AppServer, "MsgQ set event.Type = %u", chip::to_underlying(event.Type));
    chip::DeviceLayer::PlatformMgr().ScheduleWork(AppTask::MsgQConsume);
}

void AppTask::DispatchEvent(const AppEvent & event)
{
    if (event.Handler)
    {
        event.Handler(event);
    }
    else
    {
        ChipLogProgress(AppServer, "Event received with no handler. Dropping event.");
    }
}
