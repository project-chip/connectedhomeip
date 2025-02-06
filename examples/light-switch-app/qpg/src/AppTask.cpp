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

// TODO - Intentionally grouped together
#include "binding-handler.h"
#include <lib/support/CHIPMem.h>
using namespace ::chip;

#include "gpSched.h"
#include "qvIO.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include <setup_payload/OnboardingCodesUtil.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/Instance.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#if defined(QORVO_QPINCFG_ENABLE)
#include "qPinCfg.h"
#endif // QORVO_QPINCFG_ENABLE

#include <inet/EndPointStateOpenThread.h>

#include <DeviceInfoProviderImpl.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define SWITCH_MULTIPRESS_WINDOW_MS 500
#define SWITCH_LONGPRESS_WINDOW_MS 3000
#define SWITCH_BUTTON_PRESSED 1
#define SWITCH_BUTTON_UNPRESSED 0

#define APP_TASK_STACK_SIZE (2 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define SECONDS_IN_HOUR (3600)                                              // we better keep this 3600
#define TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS (1 * SECONDS_IN_HOUR) // increment every hour

namespace {
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sHaveBLEConnections      = false;
bool sIsBLEAdvertisingEnabled = false;
bool sIsMultipressOngoing     = false;
bool sLongPressDetected       = false;
uint8_t sSwitchButtonState    = SWITCH_BUTTON_UNPRESSED;

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];

StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
} // namespace

AppTask AppTask::sAppTask;

namespace {
constexpr int extDiscTimeoutSecs = 20;
}

Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

/**********************************************************
 * Identify Callbacks
 *********************************************************/

namespace {
void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
}
} // namespace

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

Identify gIdentifyEp2 = {
    chip::EndpointId{ 2 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnTriggerIdentifyEffect,
};

void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), nullptr, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void AppTask::InitServer(intptr_t arg)
{
    static chip::CommonCaseDeviceServerInitParams initParams;

    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

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

    chip::Server::GetInstance().Init(initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        PlatformMgr().ScheduleWork(OpenCommissioning, 0);
    }
}

void AppTask::OpenCommissioning(intptr_t arg)
{
    // Enable BLE advertisements
    chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if defined(QORVO_QPINCFG_ENABLE)
    qResult_t res = Q_OK;
    res           = qPinCfg_Init(NULL);
    if (res != Q_OK)
    {
        ChipLogError(NotSpecified, "qPinCfg_Init failed: %d", res);
    }
#endif // QORVO_QPINCFG_ENABLE
    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);

    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    SwitchMgr().Init();
    UpdateClusterState();

    // Setup button handler
    qvIO_SetBtnCallback(ButtonEventHandler);

    DeviceLayer::PlatformMgr().ScheduleWork(InitBindingManager);

    // Log device configuration
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
    sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
    sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
    sIsBLEAdvertisingEnabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    UpdateLEDs();

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS),
                                                      TotalHoursTimerHandler, this);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
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

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    switch (btnIdx)
    {
    case APP_FUNCTION1_BUTTON: {
        if (!btnPressed)
        {
            return;
        }

        ChipLogProgress(NotSpecified, "Level Button pressed");
        button_event.Handler = SwitchMgr().LevelHandler;
        break;
    }
    case APP_FUNCTION2_SWITCH: {
        if (!btnPressed)
        {
            ChipLogDetail(NotSpecified, "Switch button released");

            button_event.Handler =
                sLongPressDetected ? SwitchMgr().GenericSwitchLongReleaseHandler : SwitchMgr().GenericSwitchShortReleaseHandler;

            sIsMultipressOngoing = true;
            sSwitchButtonState   = SWITCH_BUTTON_UNPRESSED;
            sLongPressDetected   = false;

            chip::DeviceLayer::SystemLayer().CancelTimer(MultiPressTimeoutHandler, NULL);
            // we start the MultiPress feature window after releasing the button
            err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(SWITCH_MULTIPRESS_WINDOW_MS),
                                                              MultiPressTimeoutHandler, NULL);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
            }
        }
        else
        {
            ChipLogDetail(NotSpecified, "Switch button pressed");

            sSwitchButtonState = SWITCH_BUTTON_PRESSED;

            chip::DeviceLayer::SystemLayer().CancelTimer(LongPressTimeoutHandler, NULL);
            // we need to check if this is short or long press
            err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(SWITCH_LONGPRESS_WINDOW_MS),
                                                              LongPressTimeoutHandler, NULL);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
            }

            // if we have active multipress window we need to send extra event
            if (sIsMultipressOngoing)
            {
                ChipLogDetail(NotSpecified, "Switch MultipressOngoing");
                button_event.Handler = SwitchMgr().GenericSwitchInitialPressHandler;
                sAppTask.PostEvent(&button_event);
                chip::DeviceLayer::SystemLayer().CancelTimer(MultiPressTimeoutHandler, NULL);
                button_event.Handler = SwitchMgr().GenericSwitchMultipressOngoingHandler;
            }
            else
            {
                button_event.Handler = SwitchMgr().GenericSwitchInitialPressHandler;
            }
        }
        break;
    }
    case APP_FUNCTION3_BUTTON: {
        if (!btnPressed)
        {
            return;
        }
        ChipLogProgress(NotSpecified, "Color button pressed");
        button_event.Handler = SwitchMgr().ColorHandler;
        break;
    }
    case APP_FUNCTION4_BUTTON: {
        if (!btnPressed)
        {
            return;
        }

        ChipLogProgress(NotSpecified, "Toggle Button pressed");
        button_event.Handler = SwitchMgr().ToggleHandler;
        break;
    }
    case APP_FUNCTION5_BUTTON: {
        button_event.Handler = FunctionHandler;
        break;
    }
    default: {
        // invalid button
        return;
    }
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = aAppState;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::MultiPressTimeoutHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogDetail(NotSpecified, "MultiPressTimeoutHandler");

    sIsMultipressOngoing = false;

    AppEvent multipress_event = {};
    multipress_event.Type     = AppEvent::kEventType_Button;
    multipress_event.Handler  = SwitchMgr().GenericSwitchMultipressCompleteHandler;

    sAppTask.PostEvent(&multipress_event);
}

void AppTask::LongPressTimeoutHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogDetail(NotSpecified, "LongPressTimeoutHandler");

    // if the button is still pressed after threshold time, this is a LongPress, otherwise jsut ignore it
    if (sSwitchButtonState == SWITCH_BUTTON_PRESSED)
    {
        sLongPressDetected       = true;
        AppEvent longpress_event = {};
        longpress_event.Type     = AppEvent::kEventType_Button;
        longpress_event.Handler  = SwitchMgr().GenericSwitchLongPressHandler;

        sAppTask.PostEvent(&longpress_event);
    }
}

void AppTask::TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogDetail(NotSpecified, "HourlyTimer");

    CHIP_ERROR err;
    uint32_t totalOperationalHours = 0;

    err = ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);

    if (err == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS / SECONDS_IN_HOUR));
    }
    else if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        totalOperationalHours = 0; // set this explicitly to 0 for safety
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS / SECONDS_IN_HOUR));
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS),
                                                      TotalHoursTimerHandler, nullptr);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        qvIO_LedSet(SYSTEM_STATE_LED, false);

        qvIO_LedBlink(SYSTEM_STATE_LED, 500, 500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION5_BUTTON)
    {
        return;
    }

    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == true)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            ChipLogProgress(NotSpecified, "[BTN] Hold to select function:");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger OTA (0-3s)");
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6s)");

            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a
        // software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();

            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "[BTN] Triggering OTA Query");

            TriggerOTAQuery();
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "[BTN] Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    CHIP_ERROR err;

    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutInMs), TimerEventHandler, this);
    SuccessOrExit(err);

    mFunctionTimerActive = true;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
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

void AppTask::DispatchEvent(AppEvent * aEvent)
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

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void)
{
    ChipLogProgress(NotSpecified, "UpdateClusterState");

    /* write the new attribute value based on attribute setter API.
       example API usage of on-off attribute:

        Protocols::InteractionModel::Status status = Clusters::OnOff::Attributes::OnOff::Set(QPG_LIGHT_ENDPOINT_ID,
       LightingMgr().IsTurnedOn());

        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating on/off %x", to_underlying(status));
        }
    */
}

void AppTask::UpdateLEDs(void)
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
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        qvIO_LedSet(SYSTEM_STATE_LED, true);
    }
    else if (sIsThreadProvisioned && !sIsThreadEnabled)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 950, 50);
    }
    else if (sHaveBLEConnections)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 100, 100);
    }
    else if (sIsBLEAdvertisingEnabled)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 50, 50);
    }
    else
    {
        // not commissioned yet
        qvIO_LedSet(SYSTEM_STATE_LED, false);
    }
}

void AppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kServiceProvisioningChange: {
        sIsThreadProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kThreadConnectivityChange: {
        sIsThreadEnabled = (event->ThreadConnectivityChange.Result == kConnectivity_Established);
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
