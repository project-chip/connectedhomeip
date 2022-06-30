/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED
#include "sl_simple_led_instances.h"
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <assert.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/EFR32/freertos_bluetooth.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>
#if CHIP_ENABLE_OPENTHREAD
#include <platform/EFR32/ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#endif
#ifdef SL_WIFI
#include "wfx_host_events.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/EFR32/NetworkCommissioningWiFiDriver.h>
#endif /* SL_WIFI */

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

#define SYSTEM_STATE_LED &sl_led_led0
#define LOCK_STATE_LED &sl_led_led1
#define APP_FUNCTION_BUTTON &sl_button_btn0
#define APP_LOCK_SWITCH &sl_button_btn1

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlOperationError;
using chip::app::Clusters::DoorLock::DlOperationSource;

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace EFR32DoorLock::LockInitParams;

namespace {
TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
LEDWidget sLockLED;

#ifdef SL_WIFI
bool sIsWiFiProvisioned = false;
bool sIsWiFiEnabled     = false;
bool sIsWiFiAttached    = false;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif /* SL_WIFI */

#if CHIP_ENABLE_OPENTHREAD
bool sIsThreadProvisioned = false;
bool sIsThreadEnabled     = false;
#endif /* CHIP_ENABLE_OPENTHREAD */
bool sHaveBLEConnections = false;
bool configValueSet      = false;

EmberAfIdentifyEffectIdentifier sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

/**********************************************************
 * Identify Callbacks
 *********************************************************/

namespace {
void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;
}
} // namespace

void OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mCurrentEffectIdentifier == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE)
    {
        ChipLogProgress(Zcl, "IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE - Not supported, use effect varriant %d",
                        identify->mEffectVariant);
        sIdentifyEffect = static_cast<EmberAfIdentifyEffectIdentifier>(identify->mEffectVariant);
    }

    switch (sIdentifyEffect)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
    }
}

Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnTriggerIdentifyEffect,
};

} // namespace

using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == NULL)
    {
        EFR32_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, appStack, &appTaskStruct);
    return (sAppTaskHandle == nullptr) ? APP_ERROR_CREATE_TASK_FAILED : CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef SL_WIFI
    /*
     * Wait for the WiFi to be initialized
     */
    EFR32_LOG("APP: Wait WiFi Init");
    while (!wfx_hw_ready())
    {
        vTaskDelay(10);
    }
    EFR32_LOG("APP: Done WiFi Init");
    /* We will init server when we get IP */

    sWiFiNetworkCommissioningInstance.Init();
#endif

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        EFR32_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    EFR32_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initial lock state
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

    uint8_t numberOfCredentialsPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, numberOfCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to 5 "
                     "[endpointId=%d]",
                     endpointId);
        numberOfCredentialsPerUser = 5;
    }

    uint16_t numberOfUsers = 0;
    if (!DoorLockServer::Instance().GetNumberOfUserSupported(endpointId, numberOfUsers))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported users when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
                     endpointId);
        numberOfUsers = 10;
    }

    uint8_t numberOfWeekdaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekdaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported weekday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfWeekdaySchedulesPerUser = 10;
    }

    uint8_t numberOfYeardaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYeardaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported yearday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfYeardaySchedulesPerUser = 10;
    }

    uint8_t numberOfHolidaySchedules = 0;
    if (!DoorLockServer::Instance().GetNumberOfHolidaySchedulesSupported(endpointId, numberOfHolidaySchedules))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported holiday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfHolidaySchedules = 10;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    err = LockMgr().Init(state,
                         ParamBuilder()
                             .SetNumberOfUsers(numberOfUsers)
                             .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                             .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                             .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                             .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                             .GetLockParam());

    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("LockMgr().Init() failed");
        appError(err);
    }

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize LEDs
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);

    if (state.Value() == DlLockState::kUnlocked)
    {
        sLockLED.Set(true);
    }
    else
    {
        sLockLED.Set(false);
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));

    ConfigurationMgr().LogDeviceConfig();

// Print setup info on LCD if available
#ifdef QR_CODE_ENABLED
    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan QRCode(qrCodeBuffer);

    if (GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR)
    {
        LCDWriteQRCode((uint8_t *) QRCode.data());
    }
    else
    {
        EFR32_LOG("Getting QR code failed!");
    }
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#endif // QR_CODE_ENABLED

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        EFR32_LOG("AppTask.Init() failed");
        appError(err);
    }

    EFR32_LOG("App Task started");

    while (true)
    {
        // Users and credentials should be checked once from nvm flash on boot
        if (!configValueSet)
        {
            LockMgr().ReadConfigValues();
            configValueSet = true;
        }

        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
#ifdef SL_WIFI
            sIsWiFiProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiAttached    = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
            sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
#endif /* CHIP_ENABLE_OPENTHREAD */
            sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
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
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (gIdentify.mActive)
            {
                sStatusLED.Blink(250, 250);
            }
            if (sIdentifyEffect != EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT)
            {
                if (sIdentifyEffect == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK)
                {
                    sStatusLED.Blink(50, 50);
                }
                if (sIdentifyEffect == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE)
                {
                    sStatusLED.Blink(1000, 1000);
                }
                if (sIdentifyEffect == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY)
                {
                    sStatusLED.Blink(300, 700);
                }
            }
#if CHIP_ENABLE_OPENTHREAD
            if (sIsThreadProvisioned && sIsThreadEnabled)
#else
            if (sIsWiFiProvisioned && sIsWiFiEnabled && !sIsWiFiAttached)
#endif
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections) { sStatusLED.Blink(100, 100); }
            else { sStatusLED.Blink(50, 950); }
        }

        sStatusLED.Animate();
        sLockLED.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LockManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<LockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LockMgr().NextState() == true)
        {
            action = LockManager::LOCK_ACTION;
        }
        else
        {
            action = LockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            EFR32_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(const sl_button_t * buttonHandle, uint8_t btnAction)
{
    if (buttonHandle == NULL)
    {
        return;
    }

    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (buttonHandle == APP_LOCK_SWITCH && btnAction == SL_SIMPLE_BUTTON_PRESSED)
    {
        button_event.Handler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (buttonHandle == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
        EFR32_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
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
    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == SL_SIMPLE_BUTTON_PRESSED)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, start BLE advertissement in fast mode
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

#ifdef SL_WIFI
            if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
            if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
            {
                // Enable BLE advertisements
                ConnectivityMgr().SetBLEAdvertisingEnabled(true);
                ConnectivityMgr().SetBLEAdvertisingMode(ConnectivityMgr().kFastAdvertising);
            }
            else { EFR32_LOG("Network is already provisioned, Ble advertissement not enabled"); }
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Set lock status LED back to show state of lock.
            sLockLED.Set(!LockMgr().NextState());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            EFR32_LOG("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        EFR32_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        EFR32_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        EFR32_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    if (aAction == LockManager::LOCK_ACTION)
    {
        EFR32_LOG("Lock Action has been initiated")
        sLockLED.Set(false);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        EFR32_LOG("Unlock Action has been initiated")
        sLockLED.Set(true);
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the lock trait.
    // Turn off the lock LED if in a LOCKED state OR
    // Turn on the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        EFR32_LOG("Lock Action has been completed")
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        EFR32_LOG("Unlock Action has been completed")
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::ActionRequest(int32_t aActor, LockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);

#ifdef portYIELD_FROM_ISR
            portYIELD_FROM_ISR(higherPrioTaskWoken);
#elif portEND_SWITCHING_ISR // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
            portEND_SWITCHING_ISR(higherPrioTaskWoken);
#else                       // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
        }
        else
        {
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }

        if (!status)
            EFR32_LOG("Failed to post event to app task event queue");
    }
    else
    {
        EFR32_LOG("Event Queue is NULL should never happen");
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
        EFR32_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    DlOperationSource source = DlOperationSource::kUnspecified;

    // write the new lock value
    EmberAfStatus status =
        DoorLockServer::Instance().SetLockState(1, newState, source) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        EFR32_LOG("ERR: updating lock state %x", status);
    }
}
