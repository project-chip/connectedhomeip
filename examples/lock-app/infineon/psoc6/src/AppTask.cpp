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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "ButtonHandler.h"
#include "LEDWidget.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <cy_wcm.h>
#include <data-model-providers/codegen/Instance.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Infineon/PSOC6/NetworkCommissioningDriver.h>

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>

#if ENABLE_DEVICE_ATTESTATION
#include <DeviceAttestationCredsExampleTrustM.h>
#endif

/* OTA related includes */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/Infineon/PSOC6/OTAImageProcessorImpl.h>

using chip::BDXDownloader;
using chip::CharSpan;
using chip::DefaultOTARequestor;
using chip::FabricIndex;
using chip::GetRequestorInstance;
using chip::NodeId;
using chip::OTADownloader;
using chip::DeviceLayer::OTAImageProcessorImpl;
using chip::System::Layer;

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

#endif

#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 5000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;
namespace {

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
LEDWidget sLockLED;

bool sIsWiFiStationProvisioned = false;
bool sIsWiFiStationEnabled     = false;
bool sIsWiFiStationConnected   = false;
bool sHaveBLEConnections       = false;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
#endif

} // namespace

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;
using namespace P6DoorLock::LockInitParams;

AppTask AppTask::sAppTask;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::P6WiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}

void OnIdentifyStart(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

static Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    OnIdentifyStart,
    OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kNone,
};

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    // Initialize device attestation config
#if ENABLE_DEVICE_ATTESTATION
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleTrustMDACProvider());
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    GetAppTask().InitOTARequestor();
#endif

    GetAppTask().lockMgr_Init();
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PSOC6_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, MATTER_ARRAY_SIZE(appStack), NULL, 1, appStack, &appTaskStruct);
    return (sAppTaskHandle == nullptr) ? APP_ERROR_CREATE_TASK_FAILED : CHIP_NO_ERROR;
}

void AppTask::lockMgr_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NetWorkCommissioningInstInit();
    PSOC6_LOG("Current Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    // Initial lock state
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
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
        PSOC6_LOG("LockMgr().Init() failed");
        appError(err);
    }

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);

    if (state.Value() == DlLockState::kUnlocked)
    {
        sLockLED.Set(false);
    }
    else
    {
        sLockLED.Set(true);
    }

    ConfigurationMgr().LogDeviceConfig();
    // Users and credentials should be checked once from flash on boot
    LockMgr().ReadConfigValues();
    // Print setup info
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
}

void AppTask::Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    int rc = flash_area_boot_set_confirmed();
    if (rc != 0)
    {
        PSOC6_LOG("flash_area_boot_set_confirmed failed");
        appError(CHIP_ERROR_UNINITIALIZED);
    }
#endif
    // Initialise WSTK buttons PB0 and PB1 (including debounce).
    ButtonHandler::Init();

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        PSOC6_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    // Register the callback to init the MDNS server when connectivity is available
    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t arg) {
            // Restart the server whenever an ip address is renewed
            if (event->Type == DeviceEventType::kInternetConnectivityChange)
            {
                if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established ||
                    event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
                {
                    chip::app::DnssdServer::Instance().StartServer();
                }
            }
        },
        0);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    sAppTask.Init();
    PSOC6_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        if (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiStationEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiStationConnected   = ConnectivityMgr().IsWiFiStationConnected();
            sIsWiFiStationProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sHaveBLEConnections       = (ConnectivityMgr().NumBLEConnections() != 0);
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
        if (sAppTask.mFunction != Function::kFactoryReset)
        {
            if (sIsWiFiStationEnabled && sIsWiFiStationProvisioned && !sIsWiFiStationConnected)
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }
        sStatusLED.Animate();
        sLockLED.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * event)
{
    LockManager::Action_t action;
    int32_t actor;

    switch (event->Type)
    {
    case AppEvent::kEventType_Lock: {
        action = static_cast<LockManager::Action_t>(event->LockEvent.Action);
        actor  = event->LockEvent.Actor;
        break;
    }

    case AppEvent::kEventType_Button: {

        PSOC6_LOG("%s [Action: %d]", __FUNCTION__, event->ButtonEvent.Action);

        if (event->ButtonEvent.Action == APP_BUTTON_LONG_PRESS)
        {
            PSOC6_LOG("Sending a lock jammed event");

            /* Generating Door Lock Jammed event */
            DoorLockServer::Instance().SendLockAlarmEvent(1 /* Endpoint Id */, AlarmCodeEnum::kLockJammed);

            return;
        }
        else
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
        break;
    }

    default:
        return;
    }

    if (!LockMgr().InitiateAction(actor, action))
    {
        PSOC6_LOG("Action is already in progress or active.");
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    if (btnIdx != APP_LOCK_BUTTON_IDX && btnIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnAction;

    if (btnIdx == APP_LOCK_BUTTON_IDX)
    {
        button_event.Handler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON_IDX)
    {
        button_event.Handler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(TimerHandle_t timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) timer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * event)
{
    if (event->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = Function::kNoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * event)
{
    if (event->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }

    if (event->ButtonEvent.Action == APP_BUTTON_PRESSED)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kNoneSelected)
        {
            PSOC6_LOG("Factory Reset Triggered. Press button again within %us to cancel.",
                      FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);
            // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
            // cancel, if required.
            sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

            sAppTask.mFunction = Function::kFactoryReset;

            // Turn off all LEDs before starting blink to make sure blink is
            // co-ordinated.
            sStatusLED.Set(false);
            sLockLED.Set(false);

            sStatusLED.Blink(500);
            sLockLED.Blink(500);
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
        {
            // Set lock status LED back to show state of lock.
            sLockLED.Set(!LockMgr().NextState());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = Function::kNoneSelected;

            PSOC6_LOG("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        PSOC6_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        PSOC6_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        PSOC6_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the  lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LockManager::LOCK_ACTION)
    {
        PSOC6_LOG("Lock Action has been initiated");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        PSOC6_LOG("Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the  lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        PSOC6_LOG("Lock Action has been completed");

        sLockLED.Set(true);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        PSOC6_LOG("Unlock Action has been completed");

        sLockLED.Set(false);
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
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

void AppTask::PostEvent(const AppEvent * event)
{
    if (sAppEventQueue != NULL)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(sAppEventQueue, event, &higherPrioTaskWoken);

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
            status = xQueueSend(sAppEventQueue, event, 1);
        }

        if (!status)
            PSOC6_LOG("Failed to post event to app task event queue");
    }
    else
    {
        PSOC6_LOG("Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent * event)
{
    if (event->Handler)
    {
        event->Handler(event);
    }
    else
    {
        PSOC6_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateCluster(intptr_t context)
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
        PSOC6_LOG("ERR: updating lock state %x", to_underlying(status));
    }
}

void AppTask::UpdateClusterState(void)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateCluster, reinterpret_cast<intptr_t>(nullptr));
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char * pcTaskName)
{
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    printf("ERROR: stack overflow with task %s\r\n", pcTaskName);
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void AppTask::InitOTARequestor()
{
    SetRequestorInstance(&gRequestorCore);
    ConfigurationMgr().StoreSoftwareVersion(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    PSOC6_LOG("Current Software Version: %u", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PSOC6_LOG("Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
}
#endif
