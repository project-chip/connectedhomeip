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

#include <lib/support/CHIPMem.h>
using namespace ::chip;

#include "gpSched.h"
#include "qvIO.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include <app/server/OnboardingCodesUtil.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <inet/EndPointStateOpenThread.h>

#include <DeviceInfoProviderImpl.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "qPinCfg.h"

#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
#include "combo-handler.h"
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

#include "App_Battery.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
#define FACTORY_RESET_WINDOW_TIMEOUT 3000
#define STACK_SWITCH_CANCEL_WINDOW_TIMEOUT 3000
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

#define APP_TASK_STACK_SIZE (2 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define SECONDS_IN_HOUR (3600)                                              // we better keep this 3600
#define TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS (1 * SECONDS_IN_HOUR) // increment every hour
#define ONE_SECOND 1000000                                                  // 1 s

namespace {
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sHaveBLEConnections      = false;
bool sIsBLEAdvertisingEnabled = false;

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

Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
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
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

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
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    chip::Server::GetInstance().Init(initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        ChipLogProgress(NotSpecified, "No fabrics, starting commissioning.");
        AppTask::OpenCommissioning((intptr_t) 0);
    }
}

void AppTask::OpenCommissioning(intptr_t arg)
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

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qResult_t res  = Q_OK;

    res = qPinCfg_Init(NULL);
    if (res != Q_OK)
    {
        ChipLogError(NotSpecified, "qPinCfg_Init failed: %d", res);
    }

    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);

    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    DoorWindowMgr().Init();
    DoorWindowMgr().SetCallback(OnStateChanged);
    UpdateDeviceState(); // update DoorWindowLED
    UpdateClusterState();

    // Setup button handler
    qvIO_SetBtnCallback(ButtonEventHandler);

    // Log device configuration
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
    sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
    sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
    sIsBLEAdvertisingEnabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    UpdateLEDs();

    gpSched_ScheduleEvent(1 * ONE_SECOND, Application_StartPeriodicBatteryUpdate);

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

void AppTask::OnStateChanged(DoorWindowManager::State aState)
{
    // If the door/window state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the door/window LED if door/window sensor is in closed state.
    //  - turn off the lock LED if door/window sensor is in opened state.
    if (DoorWindowManager::State::kDoorWindowClosed == aState)
    {
        ChipLogProgress(NotSpecified, "DoorWindow state changed to CLOSED");
#if CONFIG_ENABLE_DOORWINDOW_SENSOR_STATUS_LED
        DoorWindowLedSet(true);
#endif
    }
    else if (DoorWindowManager::State::kDoorWindowOpened == aState)
    {
        ChipLogProgress(NotSpecified, "DoorWindow state changed to OPEN");
#if CONFIG_ENABLE_DOORWINDOW_SENSOR_STATUS_LED
        DoorWindowLedSet(false);
#endif
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    ChipLogProgress(NotSpecified, "ButtonEventHandler Btn[%d], %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    switch (btnIdx)
    {
    case DOORWINDOW_SENSOR_BUTTON: {
        if (btnPressed)
        {
            button_event.DoorWindowEvent.Action = static_cast<uint8_t>(DoorWindowManager::Action::kSignalDetected);
            ChipLogProgress(NotSpecified, "ButtonEventHandler signal detected");
            button_event.Handler = DoorWindowActionEventHandler;
        }
        else
        {
            button_event.DoorWindowEvent.Action = static_cast<uint8_t>(DoorWindowManager::Action::kSignalLost);
            ChipLogProgress(NotSpecified, "ButtonEventHandler signal lost");
            button_event.Handler = DoorWindowActionEventHandler;
        }

        break;
    }
    case APP_FUNCTION_BUTTON: {
        button_event.Handler = FunctionHandler;
        break;
    }
    default: {
        /* No handler implemented for this button */
        // invalid button
        return;
    }
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = DoorWindowMgr().IsDoorWindowClosed();

    ChipLogProgress(NotSpecified, "UpdateClusterStateInternal StateValue::Set : %d", newValue);

    // write the new boolean state value
    Protocols::InteractionModel::Status status =
        app::Clusters::BooleanState::Attributes::StateValue::Set(DOORWINDOW_ENDPOINT_ID, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating boolean status value %x", to_underlying(status));
    }
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = aAppState;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogProgress(NotSpecified, "HourlyTimer");

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
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
        // In case of combo app, trigger factory reset if button is released
        ChipLogProgress(NotSpecified, "[BTN] Release within %us to perform Factory Reset. Continue hold for stack switching",
                        FACTORY_RESET_WINDOW_TIMEOUT / 1000);

        sAppTask.StartTimer(FACTORY_RESET_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryResetTrigger;
#else
        // Actually trigger Factory Reset
        ChipLogProgress(NotSpecified, "Trigger Factory Reset");
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT
    }

#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryResetTrigger)
    {
        ChipLogProgress(NotSpecified, "[BTN] Stack switching selected. Release within %us to cancel.",
                        STACK_SWITCH_CANCEL_WINDOW_TIMEOUT / 1000);

        sAppTask.StartTimer(STACK_SWITCH_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_StackSwitch;
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StackSwitch)
    {
        // Trigger Combo Stack Switch
        ChipLogProgress(NotSpecified, "Trigger Stack Switch");
        sAppTask.mFunction = kFunction_NoneSelected;
        if (ComboHandler_SwitchStack() == gpUpgrade_StatusSuccess)
        {
            /* Trigger Factory reset only in case of stack switching succeeds */
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        else
        {
            ChipLogProgress(NotSpecified, "!!! Combo Stack Switch Failed");
        }
    }
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT
}

void AppTask::DoorWindowActionEventHandler(AppEvent * aEvent)
{
    DoorWindowManager::Action action = DoorWindowManager::Action::kInvalid;
    CHIP_ERROR err                   = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_DoorWindow)
    {
        action = static_cast<DoorWindowManager::Action>(aEvent->DoorWindowEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = static_cast<DoorWindowManager::Action>(aEvent->DoorWindowEvent.Action);
        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = DoorWindowManager::Action::kInvalid;
    }

    if (err == CHIP_NO_ERROR)
    {
        DoorWindowMgr().InitiateAction(action);
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON)
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
#ifndef GP_UPGRADE_DIVERSITY_DUAL_BOOT
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6s)");
#else
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (6>9s)");
            ChipLogProgress(NotSpecified, "[BTN] - Combo Stack Switch (>12s)");
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

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
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryResetTrigger)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            // In case of combo app, trigger Factory Reset
            ChipLogProgress(NotSpecified, "Trigger Factory Reset");
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StackSwitch)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "[BTN] Stack Switch has been Canceled");
        }
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT
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

    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    bool stateValueAttrValue = 0;

    /* get boolean state attribute value */
    (void) app::Clusters::BooleanState::Attributes::StateValue::Get(1, &stateValueAttrValue);

    ChipLogProgress(NotSpecified, "StateValue::Get : %d", stateValueAttrValue);
#if CONFIG_ENABLE_DOORWINDOW_SENSOR_STATUS_LED
    DoorWindowLedSet(stateValueAttrValue); // stateValueAttrValue: closed-1, open-0
#endif
}

/** @brief Set DoorWindow Sensor LED ON or OFF.
 *
 *   @param state     LED on (true) or off (false).
 */
void AppTask::DoorWindowLedSet(bool state)
{
#if CONFIG_ENABLE_DOORWINDOW_SENSOR_STATUS_LED
    qvIO_LedSet(DOORWINDOW_SENSOR_LED, state);
#endif
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
    // Otherwise, blink the LED ON for a very short time.
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

void AppTask::PostDoorWindowActionRequest(DoorWindowManager::Action aAction)
{
    AppEvent event;
    event.Type                   = AppEvent::kEventType_DoorWindow;
    event.DoorWindowEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler                = DoorWindowActionEventHandler;

    sAppTask.PostEvent(&event);
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
