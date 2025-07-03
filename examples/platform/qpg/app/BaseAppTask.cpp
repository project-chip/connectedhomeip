/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "gpSched.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "BaseAppTask.h"
#include "ota.h"
#include "powercycle_counting.h"
#if defined(GP_UPGRADE_DIVERSITY_DUAL_BOOT)
#include "combo-handler.h"
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <data-model-providers/codegen/Instance.h>

#include <app/TestEventTriggerDelegate.h>

#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/on-off-server/on-off-server.h>

#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include "ButtonHandler.h"
#include "StatusLed.h"
#include "qPinCfg.h"

#include <inet/EndPointStateOpenThread.h>

#include "DiagnosticLogsProviderDelegateImpl.h"
#include <DeviceInfoProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::DiagnosticLogs;

#include <platform/CHIPDeviceLayer.h>

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sIsThreadProvisioned     = false;
bool sIsThreadAttached        = false;
bool sHaveBLEConnections      = false;
bool sIsBLEAdvertisingEnabled = false;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];

StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

const uint8_t StatusLedGpios[] = QPINCFG_STATUS_LED;
const ButtonConfig_t buttons[] = QPINCFG_BUTTONS;

constexpr int extDiscTimeoutSecs = 20;

Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

BaseAppTask BaseAppTask::sAppTask;

/**********************************************************
 * Identify Callbacks
 *********************************************************/

void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
}

void OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mEffectVariant != Clusters::Identify::EffectVariantEnum::kDefault)
    {
        ChipLogDetail(AppServer, "Identify Effect Variant unsupported. Using default");
    }

    switch (sIdentifyEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        SystemLayer().ScheduleLambda([identify] {
            (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                               identify);
        });
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        SystemLayer().ScheduleLambda([identify] {
            (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
            (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                               identify);
        });
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        SystemLayer().ScheduleLambda(
            [identify] { (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify); });
        sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
    }
}

Identify gIdentifyEp1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerIdentifyEffect,
};

#if ACTIVATE_IDENTIFY_ON_EP2
Identify gIdentifyEp2 = {
    chip::EndpointId{ 2 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerIdentifyEffect,
};
#endif

void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

CHIP_ERROR BaseAppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    sAppTaskHandle =
        xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, MATTER_ARRAY_SIZE(appStack), nullptr, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BaseAppTask::Init()
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    const qResult_t res = qPinCfg_Init(NULL);

    if (res != Q_OK)
    {
        ChipLogError(NotSpecified, "qPinCfg_Init failed: %d", res);
    }

    StatusLed_Init(StatusLedGpios, Q_ARRAY_SIZE(StatusLedGpios), true);

    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
#if BASE_APP_BUILD
    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServerWrapper, 0);
#endif
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);

    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    UpdateClusterState();

    // Setup button handler
    ButtonHandler_Init(buttons, Q_ARRAY_SIZE(buttons), BUTTON_LOW, BaseAppTask::ButtonEventHandlerWrapper);

    // Log device configuration
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
    sIsThreadAttached        = ConnectivityMgr().IsThreadAttached();
    sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
    sIsBLEAdvertisingEnabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    UpdateLEDs();

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SEC),
                                                      TotalHoursTimerHandler, this);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }

    return err;
}

void BaseAppTask::InitServer(intptr_t arg)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    if (arg != 0)
    {
        DefaultAttributePersistenceProvider gSimpleAttributePersistence = *(DefaultAttributePersistenceProvider *) arg;
        VerifyOrDie(gSimpleAttributePersistence.Init(initParams.persistentStorageDelegate) == CHIP_NO_ERROR);
    }
    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);

    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);

    // Use GenericFaultTestEventTriggerHandler to inject faults
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static GenericFaultTestEventTriggerHandler sFaultTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sFaultTestEventTriggerHandler) == CHIP_NO_ERROR);
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider        = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::Server::GetInstance().Init(initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif
}

void BaseAppTask::OpenCommissioning(intptr_t arg)
{
    // Enable BLE advertisements

    SystemLayer().ScheduleLambda([] {
        CHIP_ERROR err;
        err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");
        }
    });
}

void BaseAppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

bool BaseAppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    // Base class handles APP_FUNCTION_BUTTON as common functionality
    if (btnIdx != APP_FUNCTION_BUTTON)
    {
        return false;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    button_event.Handler = FunctionHandler;
    sAppTask.PostEvent(&button_event);

    return true;
}

void BaseAppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = aAppState;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void BaseAppTask::TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogProgress(NotSpecified, "HourlyTimer");

    CHIP_ERROR err;
    uint32_t totalOperationalHours = 0;

    err = ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);

    if (err == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SEC / ONE_HOUR_SEC));
    }
    else if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        totalOperationalHours = 0; // set this explicitly to 0 for safety
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SEC / ONE_HOUR_SEC));
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SEC),
                                                      TotalHoursTimerHandler, nullptr);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}

void BaseAppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer || !sAppTask.mFunctionTimerActive)
    {
        // Ignore this event
        return;
    }

    // If we reached here, the button was held past OTA_START_TRIGGER_TIMEOUT - go to next functionality, OTA update
    if (sAppTask.mFunction == kFunction_StartBleAdv)
    {
        ChipLogProgress(NotSpecified, "[BTN] Release button now to start Software Updater");
        ChipLogProgress(NotSpecified, "[BTN] Hold to trigger Factory Reset");
        sAppTask.mFunction = kFunction_SoftwareUpdate;
        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT_MS - OTA_START_TRIGGER_TIMEOUT_MS);
    }
    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT_MS,
    // initiate factory reset
    else if (sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us sec to cancel",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT_MS / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.mFunction = kFunction_PreFactoryReset;
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT_MS);
    }
    // If we reached here, the button was held past FACTORY_RESET_CANCEL_WINDOW_TIMEOUT_MS,
    // continue with stack switching
    else if (sAppTask.mFunction == kFunction_PreFactoryReset)
    {
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
        ChipLogProgress(NotSpecified, "[BTN] Release to perform Factory Reset. Continue to hold for Stack Switching");
        sAppTask.mFunction = kFunction_FactoryReset;
        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT_MS);
#else
        ChipLogProgress(NotSpecified, "[BTN] Release to trigger Factory Reset");
        sAppTask.mFunction = kFunction_FactoryReset;
#endif
    }
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
    else if (sAppTask.mFunction == kFunction_FactoryReset)
    {
        ChipLogProgress(NotSpecified, "[BTN] Stack switching selected. Release within %us sec to cancel",
                        STACK_SWITCH_CANCEL_WINDOW_TIMEOUT_MS / 1000);

        sAppTask.StartTimer(STACK_SWITCH_CANCEL_WINDOW_TIMEOUT_MS);

        // Start timer for triggering to jump to ZB app
        sAppTask.mFunction = kFunction_PreStackSwitch;
    }
    else if (sAppTask.mFunction == kFunction_PreStackSwitch)
    {
        // Trigger Combo Stack Switch
        ChipLogProgress(NotSpecified, "[BTN] Trigger Stack Switch");
        sAppTask.mFunction = kFunction_NoneSelected;
        if (ComboHandler_SwitchStack() == gpUpgrade_StatusSuccess)
        {
            /* Trigger Factory reset only in case of stack switching succeeds */
            SystemLayer().ScheduleLambda([] { chip::Server::GetInstance().ScheduleFactoryReset(); });
        }
        else
        {
            ChipLogProgress(NotSpecified, "!!! Combo Stack Switch Failed");
        }
    }
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT
}

void BaseAppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    // Handle function button press event
    if (aEvent->ButtonEvent.Action)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            ChipLogProgress(NotSpecified, "[BTN] Hold to select function:");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger BLE adv (0-1.5s)");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger OTA (1.5-3s)");
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (6-9s)");
            ChipLogProgress(NotSpecified, "[BTN] - Stack switching (>12s)");
#else
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6s)");
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

            sAppTask.mFunction = kFunction_StartBleAdv;
            sAppTask.StartTimer(OTA_START_TRIGGER_TIMEOUT_MS);
        }
    }
    // Handle function button release event
    else if (sAppTask.mFunctionTimerActive)
    {
        sAppTask.CancelTimer();

        if (sAppTask.mFunction == kFunction_StartBleAdv)
        {
            if (ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                ChipLogProgress(NotSpecified, "[BTN] BLE advertising already in progress");
            }
            else
            {
                // Enable BLE advertisements and pairing window
                OpenCommissioning((intptr_t) 0);
                ChipLogProgress(NotSpecified, "[BTN] BLE advertising started - waiting for pairing");
            }
        }
        else if (sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            ChipLogProgress(NotSpecified, "[BTN] Triggering OTA query");
            TriggerOTAQuery();
        }
        else if (sAppTask.mFunction == kFunction_PreFactoryReset)
        {
            ChipLogProgress(NotSpecified, "[BTN] Factory reset has been canceled");
        }
        else if (sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Trigger Factory Reset
            ChipLogProgress(NotSpecified, "[BTN] Trigger Factory Reset");
            sAppTask.mFunction = kFunction_NoneSelected;
            SystemLayer().ScheduleLambda([] { chip::Server::GetInstance().ScheduleFactoryReset(); });
        }
        else if (sAppTask.mFunction == kFunction_PreStackSwitch)
        {
            ChipLogProgress(NotSpecified, "[BTN] Stack switch has been canceled");
        }

        sAppTask.mFunction = kFunction_NoneSelected;
    }
}

void BaseAppTask::StartTimer(uint32_t aTimeoutInMs)
{
    SystemLayer().ScheduleLambda([aTimeoutInMs, this] {
        CHIP_ERROR err;
        chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
        err =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutInMs), TimerEventHandler, this);
        SuccessOrExit(err);

        this->mFunctionTimerActive = true;
    exit:
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
        }
    });
}

void BaseAppTask::CancelTimer()
{
    SystemLayer().ScheduleLambda([this] {
        chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
        this->mFunctionTimerActive = false;
    });
}

void BaseAppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != nullptr)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            ChipLogError(NotSpecified, "Failed to post event to app task event queue");
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Event Queue is nullptr should never happen");
    }
}

void BaseAppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

void BaseAppTask::UpdateLEDs(void)
{
    // If system has "full connectivity", keep the LED On constantly.
    //
    // If thread and service provisioned, but not attached to the thread network
    // yet OR no connectivity to the service OR subscriptions are not fully
    // established THEN blink the LED Off for a short period of time.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink
    // the LEDs at an even rate of 100ms.
    //
    // Otherwise, turn the LED OFF.
    if (sIsThreadProvisioned && sIsThreadAttached)
    {
        StatusLed_SetLed(SYSTEM_STATE_LED, true);
    }
    else if (sIsThreadProvisioned && !sIsThreadAttached)
    {
        StatusLed_BlinkLed(SYSTEM_STATE_LED, 950, 50);
    }
    else if (sHaveBLEConnections)
    {
        StatusLed_BlinkLed(SYSTEM_STATE_LED, 100, 100);
    }
    else if (sIsBLEAdvertisingEnabled)
    {
        StatusLed_BlinkLed(SYSTEM_STATE_LED, 50, 50);
    }
    else
    {
        // not commisioned yet
        StatusLed_SetLed(SYSTEM_STATE_LED, false);
    }
}

void BaseAppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kServiceProvisioningChange: {
        sIsThreadProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kThreadConnectivityChange: {
        sIsThreadAttached = (event->ThreadConnectivityChange.Result == kConnectivity_Established);
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionEstablished: {
        sHaveBLEConnections = true;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        sHaveBLEConnections = false;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEAdvertisingChange: {
        sIsBLEAdvertisingEnabled = (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started);
        UpdateLEDs();
        break;
    }

    default:
        break;
    }
}
