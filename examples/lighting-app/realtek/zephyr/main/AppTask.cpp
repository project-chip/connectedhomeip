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
#include "RealtekObserver.h"
#include "matter_button.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <system/SystemClock.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAInitializer.h>
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {

constexpr int kAppEventQueueSize      = 10;
constexpr EndpointId kLightEndpointId = 1;

constexpr uint32_t kResetTriggerTimeout      = 1500;
constexpr uint32_t kBleAdvTriggerTimeout     = 1500;
constexpr uint32_t kFactoryResetCancelWindow = 3500;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

Identify sIdentify = { kLightEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                       Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

LEDWidget lightStatusLED;
LEDWidget identifyLED;
LEDWidget systemStatusLED;

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;

// Set once the app event loop is about to run. Button events produced while
// the stack is still starting up are dropped.
bool sIsInitialized = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
chip::app::DefaultAttributePersistenceProvider gSimpleAttributePersistence;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static bool isOTAInitialized                 = false;
constexpr uint32_t kInitOTARequestorDelaySec = 3;

void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "InitOTARequestorHandler");
    OTAInitializer::Instance().InitOTARequestor();
}
#endif

#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}
#endif // CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT

} // namespace

#if CHIP_ENABLE_OPENTHREAD
app::Clusters::NetworkCommissioning::InstanceAndDriver<NetworkCommissioning::GenericThreadDriver> sThreadNetworkDriver(0);
#endif // CHIP_ENABLE_OPENTHREAD

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
    // Initialize LEDs
    lightStatusLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {}));
    identifyLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {}));
    systemStatusLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {}));

    UpdateStatusLED();

    // Initialize buttons
    int ret = matter_button_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("matter_button_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

    // Initialize function timer
    k_timer_init(&sFunctionTimer, AppTask::FunctionTimerTimeoutCallback, nullptr);

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

    (void) sThreadNetworkDriver.Init();
#else
    return CHIP_ERROR_INTERNAL;
#endif // CHIP_ENABLE_OPENTHREAD

#ifdef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* OTA image confirmation must be done before the factory data init. */
    // OtaConfirmNewImage();
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
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    VerifyOrDie(gSimpleAttributePersistence.Init(initParams.persistentStorageDelegate) == CHIP_NO_ERROR);

    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    // Set up OpenThread configuration when OpenThread is included
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams{};
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif

    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    // Register observer to trigger a factory reset when the last fabric is removed (unpaired).
    static RealtekObserver sRealtekObserver;
    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sRealtekObserver);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("The RealtekObserver could not be registered with the FabricTable: %" CHIP_ERROR_FORMAT, err.Format());
    }

    chip::app::SetAttributePersistenceProvider(&gSimpleAttributePersistence);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Add CHIP event handler and start CHIP thread.
    TEMPORARY_RETURN_IGNORED PlatformMgr().AddEventHandler(ChipEventHandler, 0);

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

    // The stack is up, so from here on button events are meaningful. Anything
    // pressed during init was dropped by ButtonEventHandler.
    sIsInitialized = true;

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
    event.Handler = [](const AppEvent &) { identifyLED.Blink(LedConsts::kIdentifyBlinkRate_ms); };
    PostEvent(event);
}

void AppTask::IdentifyStopHandler(Identify *)
{
    AppEvent event;
    event.Type    = AppEventType::IdentifyStop;
    event.Handler = [](const AppEvent &) { identifyLED.BlinkStop(); };
    PostEvent(event);
}

LEDWidget & AppTask::GetLightLED(void)
{
    return lightStatusLED;
}

void AppTask::ButtonEventHandler(uint8_t buttonIdx, uint8_t state)
{
    if (buttonIdx != APP_TOGGLE_BUTTON && buttonIdx != APP_FUNCTION_BUTTON && buttonIdx != APP_LEVEL_BUTTON &&
        buttonIdx != APP_BLE_ADV_BUTTON)
    {
        return;
    }

    if (!sIsInitialized)
    {
        return;
    }

    AppEvent button_event;

    button_event.Type                  = AppEventType::Button;
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

    // Three stage state machine, matching the FreeRTOS variant:
    //   hold < 1.5s        : reset (applied on release)
    //   1.5s .. 3.0s       : toggle BLE advertising (applied on release)
    //   3.0s and beyond    : factory reset, cancelable for another 3.5s
    if (Instance().mFunction == FunctionEvent::Reset)
    {
        LOG_INF("[BTN] Start/Stop BLE advertising selected. Release to apply.");

        Instance().StartTimer(kBleAdvTriggerTimeout);
        Instance().mFunction = FunctionEvent::AdvertisingStart;

        // Turn the LED off before starting the blink so the pattern is visible
        // from its first cycle.
        systemStatusLED.Set(false);
        systemStatusLED.Blink(LedConsts::StatusLed::Provisioned::kOn_ms, LedConsts::StatusLed::Provisioned::kOff_ms);
    }
    else if (Instance().mFunction == FunctionEvent::AdvertisingStart)
    {
        LOG_INF("[BTN] Factory Reset selected. Release within %ums to cancel.", kFactoryResetCancelWindow);

        Instance().StartTimer(kFactoryResetCancelWindow);
        Instance().mFunction = FunctionEvent::FactoryReset;

        systemStatusLED.Set(false);
        systemStatusLED.Blink(LedConsts::kBlinkRate_ms);
    }
    else if (Instance().mFunction == FunctionEvent::FactoryReset)
    {
        Instance().mFunction = FunctionEvent::NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(const AppEvent & event)
{
    LOG_INF("Button state changed: index %d, action %d", event.ButtonEvent.ButtonIdx, event.ButtonEvent.Action);

    const bool pushed = event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed);

    switch (event.ButtonEvent.ButtonIdx)
    {
    case APP_TOGGLE_BUTTON:
        if (pushed)
        {
            bool on = false;
            if (Clusters::OnOff::Attributes::OnOff::Get(kLightEndpointId, &on) != Protocols::InteractionModel::Status::Success)
            {
                LOG_ERR("[BTN] Failed to read the OnOff attribute");
                break;
            }

            lightStatusLED.Set(!on);
            Instance().UpdateClusterState();
        }
        break;

    case APP_LEVEL_BUTTON:
        // Dimming is not implemented: PWM output for the LEDs is disabled, so
        // there is nothing to drive. Left as a no-op rather than repurposed, so
        // the button map keeps matching the FreeRTOS variant.
        break;

    case APP_BLE_ADV_BUTTON:
        if (pushed)
        {
            RETURN_SAFELY_IGNORED PlatformMgr().ScheduleWork(BLEStartAdvertising, 0);
        }
        break;

    case APP_FUNCTION_BUTTON:
        if (pushed)
        {
            if (!Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::NoneSelected)
            {
                LOG_INF("[BTN] Hold to select function:");
                LOG_INF("[BTN] - Reset (release within %ums)", kResetTriggerTimeout);
                LOG_INF("[BTN] - Start/Stop BLE advertising (release within %ums)", kResetTriggerTimeout + kBleAdvTriggerTimeout);
                LOG_INF("[BTN] - Factory Reset (release within a further %ums to cancel)", kFactoryResetCancelWindow);

                Instance().StartTimer(kResetTriggerTimeout);
                Instance().mFunction = FunctionEvent::Reset;
            }
        }
        else if (Instance().mFunctionTimerActive)
        {
            switch (Instance().mFunction)
            {
            case FunctionEvent::Reset:
                Instance().CancelTimer();
                Instance().mFunction = FunctionEvent::NoneSelected;

                LOG_INF("[BTN] Reset selected");
                sys_reboot(SYS_REBOOT_COLD);
                break;

            case FunctionEvent::AdvertisingStart:
                Instance().CancelTimer();
                Instance().mFunction = FunctionEvent::NoneSelected;

                RETURN_SAFELY_IGNORED PlatformMgr().ScheduleWork(BLEStartAdvertising, 0);
                break;

            case FunctionEvent::FactoryReset:
                UpdateStatusLED();
                Instance().CancelTimer();
                Instance().mFunction = FunctionEvent::NoneSelected;

                LOG_INF("[BTN] Factory Reset has been Canceled");
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }
}

void AppTask::BLEStartAdvertising(intptr_t /* arg */)
{
    const bool enable = !ConnectivityMgr().IsBLEAdvertisingEnabled();

    CHIP_ERROR err = ConnectivityMgr().SetBLEAdvertisingEnabled(enable);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEAdvertisingEnabled failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    LOG_INF("[BTN] BLE advertising %s", enable ? "enabled" : "disabled");
}

void AppTask::UpdateStatusLED()
{
    // Update the status LED.
    //
    // If IPv6 network and service provisioned, keep the LED Off constantly.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
    // rate of 100ms.
    //
    // Otherwise, blink the LED for a very short time.
    if (sIsNetworkProvisioned && sIsNetworkEnabled)
    {
        systemStatusLED.Set(false);
    }
    else if (sHaveBLEConnections)
    {
        systemStatusLED.Blink(LedConsts::StatusLed::Unprovisioned::kOn_ms, LedConsts::StatusLed::Unprovisioned::kOff_ms);
    }
    else
    {
        systemStatusLED.Blink(LedConsts::StatusLed::Provisioned::kOn_ms, LedConsts::StatusLed::Provisioned::kOff_ms);
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
            CHIP_ERROR startTimerErr = chip::DeviceLayer::SystemLayer().StartTimer(
                chip::System::Clock::Seconds32(kInitOTARequestorDelaySec), InitOTARequestorHandler, nullptr);
            if (startTimerErr != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "StartTimer failed: %" CHIP_ERROR_FORMAT, startTimerErr.Format());
            }
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
            Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, lightStatusLED.IsTurnedOn());

        if (status != Protocols::InteractionModel::Status::Success)
        {
            LOG_ERR("Updating on/off cluster failed: %x", to_underlying(status));
        }
    });
}
