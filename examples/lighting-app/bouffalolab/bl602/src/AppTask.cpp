/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "LEDWidget.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/BL602/NetworkCommissioningDriver.h>
#include <platform/bouffalolab/BL602/OTAImageProcessorImpl.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <InitPlatform.h>
#include <async_log.h>
#include <bl_sys_ota.h>
#include <easyflash.h>
#include <hal_sys.h>
#include <lib/support/ErrorStr.h>

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (8192)
#define APP_TASK_PRIORITY 2
#define STATUS_LED_GPIO_NUM GPIO_NUM_2 // Use LED1 (blue LED) as status LED on DevKitC

static const char * const TAG = "lighting-app";
static xTaskHandle OTA_TASK_HANDLE;
static LEDWidget statusLED;
namespace {
TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

BaseType_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sHaveBLEConnections = false;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
} // namespace

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;

namespace {
chip::app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::BLWiFiDriver::GetInstance()));
} // namespace

using namespace ::chip::System;

DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;

AppTask AppTask::sAppTask;
static DeviceCallbacks EchoCallbacks;

CHIP_ERROR AppTask::StartAppTask()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        log_error("Failed to allocate app event queue\r\n");
        return APP_ERROR_EVENT_QUEUE_FAILED;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreate(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, NULL);
    if (sAppTaskHandle)
    {
        err = CHIP_NO_ERROR;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        log_error("funct timer create failed\r\n");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    err = LightMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        log_error("LightMgr().Init() failed\r\n");
        return err;
    }

    LightMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    SetRequestorInstance(&gRequestorCore);
    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.SetPeriodicQueryTimeout(OTA_PERIODIC_QUERY_TIMEOUT);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

    InitButtons();

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    Clock::Timestamp lastChangeTime = Clock::kZero;
    CHIP_ERROR err;

    log_info("App Task entered\r\n");
    enable_async_log();

    err = sWiFiNetworkCommissioningInstance.Init();
    if (CHIP_NO_ERROR != err)
    {
        log_error("Network commissioning failed, err:%d \r\n", err);
    }

    chip::CommonCaseDeviceServerInitParams initParams;
    err = initParams.InitializeStaticResourcesBeforeServerInit();
    if (CHIP_NO_ERROR != err)
    {
        log_error("Resources Init failed, err:%d \r\n", err);
        return;
    }

    err = chip::Server::GetInstance().Init(initParams);
    if (CHIP_NO_ERROR != err)
    {
        log_error("Server Init failed, err:%d \r\n", err);
        return;
    }

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        log_error("AppTask.Init() failed due to %d\r\n", err);
        return;
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            log_info("receiving event type: %d\r\n", event.Type);
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
            sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sHaveBLEConnections)
            {
                // TODO: 3R
                // sStatusLED.Blink(100, 100);
            }
            else
            {
                // TODO: 3R
                // sStatusLED.Blink(50, 950);
            }
        }

        // TODO: 3R
        // sStatusLED.Animate();

        Clock::Timestamp now            = SystemClock().GetMonotonicTimestamp();
        Clock::Timestamp nextChangeTime = lastChangeTime + Clock::Seconds16(5);

        if (nextChangeTime < now)
        {
            lastChangeTime = now;
        }
    }
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LightingManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Light)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightMgr().IsLightOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LightMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            log_info("Action is already in progress or active.\r\n");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
#if 0 // TODO: 3R
    if (btnIdx != APP_LOCK_BUTTON && btnIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }
#endif

    AppEvent button_event = {};
    button_event.Type     = AppEvent::kEventType_Button;
    // button_event.ButtonEvent.PinNo  = btnIdx;
    button_event.ButtonEvent.Action = btnAction;

    /* if (btnIdx == APP_LOCK_BUTTON && btnAction == APP_BUTTON_PRESSED)
    {
        button_event.Handler = LockActionEventHandler;
        log_info("posting button_event(lock button)\r\n");
        sAppTask.PostEvent(&button_event);
    }
    else */
    if (btnIdx == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
        log_info("posting button_event(function button)\r\n");
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
        log_info("Factory Reset Triggered. Release button within %ums to cancel.\r\n", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{

    if (aEvent->ButtonEvent.Action == APP_BUTTON_LONGPRESSED)
    {
        log_info("FactoryReset! please release boutton!!!\r\n");
        statusLED.Toggle();
        vTaskDelay(1000);
        statusLED.Toggle();
        vTaskDelay(1000);
        statusLED.Toggle();
        vTaskDelay(3000);
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
    else if (aEvent->ButtonEvent.Action == APP_BUTTON_PRESSED)
    {
        AppEvent Lightevent = {};
        Lightevent.Type     = AppEvent::kEventType_Button;
        Lightevent.Handler  = LightActionEventHandler;
        sAppTask.PostEvent(&Lightevent);
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        log_error("app timer stop() failed\r\n");
        return;
    }

    mFunctionTimerActive = false;
}
void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        log_warn("app timer already started!\r\n");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        log_error("app timer start() failed\r\n");
        return;
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    if (aAction == LightingManager::ON_ACTION)
    {
        log_info("Turning light ON\r\n");
#if 0 // TODO: 3R
        sLightLED.Set(true);
#endif
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        log_info("Turning light OFF\r\n");
#if 0 // TODO: 3R
        sLightLED.Set(false);
#endif
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // action has been completed bon the light
    if (aAction == LightingManager::ON_ACTION)
    {
        log_info("Light ON\r\n");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        log_info("Light OFF\r\n");
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_Light;
    event.LightEvent.Actor  = aActor;
    event.LightEvent.Action = aAction;
    event.Handler           = LightActionEventHandler;
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
            log_info("sending event type from isr: %d\r\n", aEvent->Type);
            status = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);

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
            log_info("sending event type: %d\r\n", aEvent->Type);
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }

        if (!status)
            log_error("Failed to post event to app task event queue\r\n");
    }
    else
    {
        log_error("Event Queue is NULL should never happen\r\n");
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
        log_warn("Event received with no handler. Dropping event.\r\n");
    }
}

void AppTask::UpdateClusterState(void)
{
    uint8_t newValue = LightMgr().IsLightOn();
    log_info("updating on/off = %x\r\n", newValue);
    EmberAfStatus status =
        emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        log_error("ERR: updating on/off %x\r\n", status);
    }
}

void AppTask::FactoryResetButtonEventHandler(void)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = APP_BUTTON_LONGPRESSED;

    button_event.Handler = FunctionHandler;
    log_info("FactoryResetButtonEventHandler\r\n");
    sAppTask.PostEvent(&button_event);
}

void AppTask::LightingActionButtonEventHandler(void)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = APP_BUTTON_PRESSED;

    button_event.Handler = FunctionHandler;
    log_info("LightingActionButtonEventHandler\r\n");
    sAppTask.PostEvent(&button_event);
}

void AppTask::InitButtons(void)
{
    Button_Configure_FactoryResetEventHandler(&FactoryResetButtonEventHandler);
    Button_Configure_LightingActionEventHandler(&LightingActionButtonEventHandler);
}

void AppTask::LightStateUpdateEventHandler(void)
{
    uint8_t onoff, level;
    do
    {
        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &onoff, sizeof(uint8_t)))
        {
            break;
        }
        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, (uint8_t *) &level,
                                 sizeof(uint8_t)))
        {
            break;
        }
        if (0 == onoff)
        {
            statusLED.SetBrightness(0);
            statusLED.Set(0);
        }
        else
        {
            statusLED.SetBrightness(level);
        }
    } while (0);
}
