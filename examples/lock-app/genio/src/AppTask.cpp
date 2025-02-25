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

#include "qrcodegen.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <assert.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/mt793x/NetworkCommissioningWiFiDriver.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
// #define EXAMPLE_VENDOR_ID                   0xcafe

#ifdef portYIELD_FROM_ISR
#define OS_YIELD_FROM_ISR(yield) portYIELD_FROM_ISR(yield)
#elif portEND_SWITCHING_ISR
#define OS_YIELD_FROM_ISR(yield) portEND_SWITCHING_ISR(yield)
#else
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif

namespace {

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
LEDWidget sLockLED;

bool sIsWiFiProvisioned = false;
bool sIsWiFiEnabled     = false;
bool sIsWiFiAttached    = false;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

bool configValueSet             = false;
bool mSyncClusterToButtonAction = false;

StackType_t appStack[APP_TASK_STACK_SIZE * 2 / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
using namespace chip::DeviceLayer::NetworkCommissioning;
chip::app::Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */,
                                                                                      &GenioWiFiDriver::GetInstance());
#endif

} // namespace

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace MT793XDoorLock::LockInitParams;

using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == NULL)
    {
        MT793X_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, MATTER_ARRAY_SIZE(appStack), NULL, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
        return APP_ERROR_CREATE_TASK_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    // Wait for the WiFi to be initialized
    MT793X_LOG("APP: Wait WiFi Init");
    vTaskDelay(1000); // TODO
    MT793X_LOG("APP: Done WiFi Init");

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    sWiFiNetworkCommissioningInstance.Init();
#endif
    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        MT793X_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    MT793X_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

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

    error = LockMgr().Init(state,
                           ParamBuilder()
                               .SetNumberOfUsers(numberOfUsers)
                               .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                               .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                               .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                               .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                               .GetLockParam());

    if (error != CHIP_NO_ERROR)
    {
        MT793X_LOG("LockMgr().Init() failed");
        appError(error);
    }

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sStatusLED.Init(LED_STATUS);
    sLockLED.Init(LED_LIGHT);

    if (state.Value() == DlLockState::kUnlocked)
    {
        sLockLED.Set(false);
    }
    else
    {
        sLockLED.Set(true);
    }
    chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));

    ConfigurationMgr().LogDeviceConfig();

    // PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kSoftAP));

    return error;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    CHIP_ERROR error = sAppTask.Init();
    if (error != CHIP_NO_ERROR)
    {
        MT793X_LOG("AppTask.Init() failed");
        appError(error);
    }

    MT793X_LOG("AppTask started");

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
            sIsWiFiProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiAttached    = ConnectivityMgr().IsWiFiStationConnected();

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
            if (sIsWiFiProvisioned && sIsWiFiEnabled && !sIsWiFiAttached)
            {
                sStatusLED.Blink(950, 50);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }
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
            MT793X_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer || sAppTask.mFunctionTimerActive == false)
    {
        return;
    }

    switch (sAppTask.mFunction)
    {
    case kFunction_NoneSelected:
        break;

    case kFunction_LightSwitch:
        // Start timer for user to cancel the facotry reset, if needed
        MT793X_LOG("Factory Reset Triggered.");
        MT793X_LOG("Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
        break;

    case kFunction_FactoryReset:
        MT793X_LOG("Factory Reset Start.");
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
        sStatusLED.Set(true);
        break;

    default:
        break;
    }
}

void AppTask::SingleButtonEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        MT793X_LOG("A Non ButtonEvent received %d", aEvent->Type);
        return;
    }

    if (aEvent->ButtonEvent.Pressed)
    {
        if (sAppTask.mFunctionTimerActive == false)
        {
            /* Start the timer to detect how long Button has been pressed */
            MT793X_LOG("AppTask status LED on");
            sStatusLED.Set(true);
            sAppTask.mFunction = kFunction_LightSwitch;
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
        }
        else
        {
            MT793X_LOG("AppTask function timer already started");
        }
    }
    else
    {
        /* Cancel the timer to detect how long Button has been pressed */
        sAppTask.CancelTimer();

        switch (sAppTask.mFunction)
        {
        case kFunction_LightSwitch:
            MT793X_LOG("AppTask light switch");
            AppEvent event;
            event.Type = AppEvent::kEventType_Button;
            LockActionEventHandler(&event);
            break;
        case kFunction_FactoryReset:
            // factory reset cancelled, restore LED
            MT793X_LOG("AppTask factory reset cancelled");
            break;
        default:
            MT793X_LOG("not handled key release event, mFunction = %x", sAppTask.mFunction);
            break;
        }

        sStatusLED.Set(false);

        sAppTask.mFunction = kFunction_NoneSelected;
    }
}

void AppTask::ButtonHandler(const filogic_button_t & button)
{
    AppEvent button_event            = {};
    button_event.Type                = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pressed = button.press;
    button_event.Handler             = SingleButtonEventHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = ButtonTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        MT793X_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        MT793X_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        MT793X_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    if (aAction == LockManager::LOCK_ACTION)
    {
        MT793X_LOG("Lock Action has been initiated")
        sLockLED.Set(true);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        MT793X_LOG("Unlock Action has been initiated")
        sLockLED.Set(false);
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LockManager::Action_t aAction)
{
    // action has been completed bon the light
    if (aAction == LockManager::LOCK_ACTION)
    {
        MT793X_LOG("Lock Action has been completed")
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        MT793X_LOG("Unlock Action has been completed")
    }

    if (mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        mSyncClusterToButtonAction = false;
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
            BaseType_t higherPrioTaskWoken;

            higherPrioTaskWoken = pdFALSE;
            status              = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);
            OS_YIELD_FROM_ISR(higherPrioTaskWoken);
        }
        else
        {
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }

        if (!status)
            MT793X_LOG("Failed to post event to app task event queue");
    }
    else
    {
        MT793X_LOG("Event Queue is NULL should never happen");
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
        MT793X_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    OperationSourceEnum source = OperationSourceEnum::kUnspecified;

    // write the new lock value
    Protocols::InteractionModel::Status status = DoorLockServer::Instance().SetLockState(1, newState, source)
        ? Protocols::InteractionModel::Status::Success
        : Protocols::InteractionModel::Status::Failure;

    if (status != Protocols::InteractionModel::Status::Success)
    {
        MT793X_LOG("ERR: updating lock state %x", to_underlying(status));
    }
}
