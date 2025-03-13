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
#include "FabricTableDelegate.h"
#include "LEDUtil.h"
#include "LEDWidget.h"
#include "PumpManager.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <system/SystemClock.h>

#if CONFIG_CHIP_OTA_REQUESTOR
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
using namespace ::chip::app::Clusters;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
constexpr uint32_t kFactoryResetTriggerTimeout      = 3000;
constexpr uint32_t kFactoryResetCancelWindowTimeout = 3000;
constexpr size_t kAppEventQueueSize                 = 10;
constexpr EndpointId kPccClusterEndpoint            = 1;
constexpr EndpointId kOnOffClusterEndpoint          = 1;

// NOTE! This key is for test/certification only and should not be available in production devices!
// If CONFIG_CHIP_FACTORY_DATA is enabled, this value is read from the factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
k_timer sFunctionTimer;

LEDWidget sStatusLED;
LEDWidget sPumpStateLED;
FactoryResetLEDsWrapper<2> sFactoryResetLEDs{ { FACTORY_RESET_SIGNAL_LED, FACTORY_RESET_SIGNAL_LED1 } };

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled     = false;
bool sHaveBLEConnections   = false;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#ifdef CONFIG_CHIP_CRYPTO_PSA
chip::Crypto::PSAOperationalKeystore sPSAOperationalKeystore{};
#endif
} // namespace

namespace LedConsts {
constexpr uint32_t kBlinkRate_ms{ 500 };
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

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed");
        return err;
    }
#elif !defined(CONFIG_WIFI_NRF70)
    return CHIP_ERROR_INTERNAL;
#endif

    // Initialize LEDs
    LEDWidget::InitGpio();
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sPumpStateLED.Init(PUMP_STATE_LED);
    sPumpStateLED.Set(!PumpMgr().IsStopped());

    UpdateStatusLED();

    PumpMgr().Init();
    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize buttons
    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return chip::System::MapErrorZephyr(ret);
    }

    // Initialize function button timer
    k_timer_init(&sFunctionTimer, &AppTask::FunctionTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
    // Initialize DFU over SMP
    GetDFUOverSMP().Init();
    GetDFUOverSMP().ConfirmNewImage();
#endif

#ifdef CONFIG_CHIP_OTA_REQUESTOR
    /* OTA image confirmation must be done before the factory data init. */
    OtaConfirmNewImage();
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
    initParams.dataModelProvider        = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
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
        DispatchEvent(event);
    }

    return CHIP_NO_ERROR;
}

void AppTask::StartActionEventHandler(const AppEvent & event)
{
    PumpManager::Action_t action = PumpManager::INVALID_ACTION;
    int32_t actor                = 0;

    if (event.Type == AppEventType::Start)
    {
        action = static_cast<PumpManager::Action_t>(event.StartEvent.Action);
        actor  = event.StartEvent.Actor;
    }
    else if (event.Type == AppEventType::Button)
    {
        action = PumpMgr().IsStopped() ? PumpManager::START_ACTION : PumpManager::STOP_ACTION;
        actor  = static_cast<uint8_t>(AppEventType::Button);
    }

    if (action != PumpManager::INVALID_ACTION && !PumpMgr().InitiateAction(actor, action))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
    AppEvent event;
    event.Type = AppEventType::Button;

    if (START_BUTTON_MASK & buttonState & hasChanged)
    {
        event.ButtonEvent.PinNo  = START_BUTTON;
        event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::Button);
        event.Handler            = StartActionEventHandler;
        PostEvent(event);
    }

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

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
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
    case DeviceEventType::kThreadStateChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
    case DeviceEventType::kDnssdInitialized:
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

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    Instance().mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(PumpManager::Action_t action, int32_t actor)
{
    // If the action has been initiated by the pump, update the pump trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (action == PumpManager::START_ACTION)
    {
        LOG_INF("Pump Start Action has been initiated");
    }
    else if (action == PumpManager::STOP_ACTION)
    {
        LOG_INF("Pump Stop Action has been initiated");
    }

    sPumpStateLED.Blink(50, 50);
}

void AppTask::ActionCompleted(PumpManager::Action_t action, int32_t actor)
{
    // If the action has been completed by the pump, update the pump trait.
    // Turn on the pump state LED if in a STARTED state OR
    // Turn off the pump state LED if in a STOPPED state.
    if (action == PumpManager::START_ACTION)
    {
        LOG_INF("Pump Start Action has been completed");
        sPumpStateLED.Set(true);
    }
    else if (action == PumpManager::STOP_ACTION)
    {
        LOG_INF("Pump Stop Action has been completed");
        sPumpStateLED.Set(false);
    }

    if (actor == static_cast<uint8_t>(AppEventType::Button))
    {
        Instance().UpdateClusterState();
    }
}

void AppTask::PostStartActionRequest(int32_t actor, PumpManager::Action_t action)
{
    AppEvent event;
    event.Type              = AppEventType::Start;
    event.StartEvent.Actor  = actor;
    event.StartEvent.Action = action;
    event.Handler           = StartActionEventHandler;
    PostEvent(event);
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
    Protocols::InteractionModel::Status status;

    ChipLogProgress(NotSpecified, "UpdateClusterState");

    // Write the new values

    bool onOffState = !PumpMgr().IsStopped();

    status = OnOff::Attributes::OnOff::Set(kOnOffClusterEndpoint, onOffState);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating On/Off state  %x", to_underlying(status));
    }

    int16_t maxPressure = PumpMgr().GetMaxPressure();
    status              = PumpConfigurationAndControl::Attributes::MaxPressure::Set(kPccClusterEndpoint, maxPressure);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxPressure  %x", to_underlying(status));
    }

    uint16_t maxSpeed = PumpMgr().GetMaxSpeed();
    status            = PumpConfigurationAndControl::Attributes::MaxSpeed::Set(kPccClusterEndpoint, maxSpeed);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxSpeed  %x", to_underlying(status));
    }

    uint16_t maxFlow = PumpMgr().GetMaxFlow();
    status           = PumpConfigurationAndControl::Attributes::MaxFlow::Set(kPccClusterEndpoint, maxFlow);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxFlow  %x", to_underlying(status));
    }

    int16_t minConstPress = PumpMgr().GetMinConstPressure();
    status                = PumpConfigurationAndControl::Attributes::MinConstPressure::Set(kPccClusterEndpoint, minConstPress);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstPressure  %x", to_underlying(status));
    }

    int16_t maxConstPress = PumpMgr().GetMaxConstPressure();
    status                = PumpConfigurationAndControl::Attributes::MaxConstPressure::Set(kPccClusterEndpoint, maxConstPress);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstPressure  %x", to_underlying(status));
    }

    int16_t minCompPress = PumpMgr().GetMinCompPressure();
    status               = PumpConfigurationAndControl::Attributes::MinCompPressure::Set(kPccClusterEndpoint, minCompPress);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinCompPressure  %x", to_underlying(status));
    }

    int16_t maxCompPress = PumpMgr().GetMaxCompPressure();
    status               = PumpConfigurationAndControl::Attributes::MaxCompPressure::Set(kPccClusterEndpoint, maxCompPress);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxCompPressure  %x", to_underlying(status));
    }

    uint16_t minConstSpeed = PumpMgr().GetMinConstSpeed();
    status                 = PumpConfigurationAndControl::Attributes::MinConstSpeed::Set(kPccClusterEndpoint, minConstSpeed);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstSpeed  %x", to_underlying(status));
    }

    uint16_t maxConstSpeed = PumpMgr().GetMaxConstSpeed();
    status                 = PumpConfigurationAndControl::Attributes::MaxConstSpeed::Set(kPccClusterEndpoint, maxConstSpeed);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstSpeed  %x", to_underlying(status));
    }

    uint16_t minConstFlow = PumpMgr().GetMinConstFlow();
    status                = PumpConfigurationAndControl::Attributes::MinConstFlow::Set(kPccClusterEndpoint, minConstFlow);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstFlow  %x", to_underlying(status));
    }

    uint16_t maxConstFlow = PumpMgr().GetMaxConstFlow();
    status                = PumpConfigurationAndControl::Attributes::MaxConstFlow::Set(kPccClusterEndpoint, maxConstFlow);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstFlow  %x", to_underlying(status));
    }

    int16_t minConstTemp = PumpMgr().GetMinConstTemp();
    status               = PumpConfigurationAndControl::Attributes::MinConstTemp::Set(kPccClusterEndpoint, minConstTemp);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MinConstTemp  %x", to_underlying(status));
    }

    int16_t maxConstTemp = PumpMgr().GetMaxConstTemp();
    status               = PumpConfigurationAndControl::Attributes::MaxConstTemp::Set(kPccClusterEndpoint, maxConstTemp);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: Updating MaxConstTemp  %x", to_underlying(status));
    }
}
