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
#include <app/server/Server.h>

#include "FreeRTOS.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <examples/platform/cc13x4_26x4/CC13X4_26X4DeviceAttestationCreds.h>

#include <app/util/af-types.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/cc13xx_26xx/OTAImageProcessorImpl.h>
#endif
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#include <setup_payload/OnboardingCodesUtil.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <src/platform/cc13xx_26xx/DefaultTestEventTriggerDelegate.h>

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#if CHIP_CONFIG_ENABLE_ICD_UAT
#include "app/icd/server/ICDNotifier.h"
#endif

/* syscfg */
#include <ti_drivers_config.h>

#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 4
#define APP_EVENT_QUEUE_SIZE 10

#if (CHIP_CONFIG_ENABLE_ICD_SERVER == 1)
#define LED_ENABLE 0
#else
#define LED_ENABLE 1
#endif
#define BUTTON_ENABLE 1

#define OTAREQUESTOR_INIT_TIMER_DELAY_MS 10000

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;
#if (LED_ENABLE == 1)
static LED_Handle sAppRedHandle;
static LED_Handle sAppGreenHandle;
#endif
static Button_Handle sAppLeftHandle;
static Button_Handle sAppRightHandle;

AppTask AppTask::sAppTask;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void StartTimer(uint32_t aTimeoutMs);
void CancelTimer(void);
#endif

uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static DefaultOTARequestor sRequestorCore;
static DefaultOTARequestorStorage sRequestorStorage;
static DefaultOTARequestorDriver sRequestorUser;
static BDXDownloader sDownloader;
static OTAImageProcessorImpl sImageProcessor;

void InitializeOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects
    SetRequestorInstance(&sRequestorCore);

    sRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    sRequestorCore.Init(chip::Server::GetInstance(), sRequestorStorage, sRequestorUser, sDownloader);
    sImageProcessor.SetOTADownloader(&sDownloader);
    sDownloader.SetImageProcessorDelegate(&sImageProcessor);
    sRequestorUser.Init(&sRequestorCore, &sImageProcessor);
}

TimerHandle_t sOTAInitTimer = 0;

#endif

// The OTA Init Timer is only started upon the first Thread State Change
// detected if the device is already on a Thread Network, or during the AppTask
// Init sequence if the device is not yet on a Thread Network. Once the timer
// has been started once, it does not need to be started again so the flag will
// be set to false.
bool isAppStarting = true;

static const chip::EndpointId sIdentifyEndpointId = 0;
static const uint32_t sIdentifyBlinkRateMs        = 500;

::Identify stIdentify = { sIdentifyEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
                          Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, AppTask::TriggerIdentifyEffectHandler };

int AppTask::StartAppTask()
{
    int ret = 0;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PLAT_LOG("Failed to allocate app event queue");
        while (true)
            ;
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle) !=
        pdPASS)
    {
        PLAT_LOG("Failed to create app task");
        while (true)
            ;
    }
    return ret;
}

// Identify take action
void identify_TakeAction(void)
{
#if (LED_ENABLE == 1)
    LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppGreenHandle, 1000, LED_BLINK_FOREVER);
#endif // LED_ENABLE
}

// Identify stop action
void identify_StopAction(void)
{
#if (LED_ENABLE == 1)
    LED_stopBlinking(sAppGreenHandle);
    LED_setOff(sAppGreenHandle);
#endif // LED_ENABLE
}

void DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        PLAT_LOG("CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        PLAT_LOG("CHIPoBLE disconnected");
        break;

    case DeviceEventType::kCommissioningComplete:
        PLAT_LOG("Commissioning complete");
        break;
    case DeviceEventType::kThreadStateChange:
        PLAT_LOG("Thread State Change");
        bool isThreadAttached = ThreadStackMgrImpl().IsThreadAttached();

        if (isThreadAttached)
        {
            PLAT_LOG("Device is on the Thread Network");
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            if (isAppStarting)
            {
                StartTimer(OTAREQUESTOR_INIT_TIMER_DELAY_MS);
                isAppStarting = false;
            }
#endif
        }
        break;
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void OTAInitTimerEventHandler(TimerHandle_t xTimer)
{
    InitializeOTARequestor();
}
#endif

int AppTask::Init()
{
    cc13xx_26xxLogInit();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    PLAT_LOG("Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PLAT_LOG("Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().InitChipStack() failed");
        while (true)
            ;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // Create FreeRTOS sw timer for OTA timer.
    sOTAInitTimer = xTimerCreate("OTAInitTmr",                     // Just a text name, not used by the RTOS kernel
                                 OTAREQUESTOR_INIT_TIMER_DELAY_MS, // timer period (mS)
                                 false,                            // no timer reload (==one-shot)
                                 (void *) this,                    // init timer id = light obj context
                                 OTAInitTimerEventHandler          // timer callback handler
    );

    if (sOTAInitTimer == NULL)
    {
        PLAT_LOG("sOTAInitTimer timer create failed");
    }
    else
    {
        PLAT_LOG("sOTAInitTimer timer created successfully ");
    }
#endif

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ThreadStackMgr().InitThreadStack() failed");
        while (true)
            ;
    }

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#elif CHIP_CONFIG_ENABLE_ICD_SERVER
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif

    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ConnectivityMgr().SetThreadDeviceType() failed");
        while (true)
            ;
    }

    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ThreadStackMgr().StartThreadTask() failed");
        while (true)
            ;
    }

    // Initialize device attestation config
#ifdef CC13X4_26X4_ATTESTATION_CREDENTIALS
#ifdef CC13XX_26XX_FACTORY_DATA
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
#else
    SetDeviceAttestationCredentialsProvider(CC13X4_26X4::GetCC13X4_26X4DacProvider());
#endif
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    // Init ZCL Data Model and start server
    PLAT_LOG("Initialize Server");
    static chip::CommonCaseDeviceServerInitParams initParams;
    static DefaultTestEventTriggerDelegate sTestEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().StartEventLoopTask() failed");
        while (true)
            ;
    }

    PlatformMgr().AddEventHandler(DeviceEventCallback, reinterpret_cast<intptr_t>(nullptr));

    uiInit();

    // Initialize Pump module
    PLAT_LOG("Initialize Pump");
    PumpMgr().Init();

    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    ConfigurationMgr().LogDeviceConfig();

    bool isThreadEnabled = ThreadStackMgrImpl().IsThreadEnabled();
    if (!isThreadEnabled && isAppStarting)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        PLAT_LOG("Thread is Disabled, enable OTA Requestor");
        StartTimer(OTAREQUESTOR_INIT_TIMER_DELAY_MS);
        isAppStarting = false;
#endif
    }

    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    return 0;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    sAppTask.Init();

    while (true)
    {
        /* Task pend until we have stuff to do */
        if (xQueueReceive(sAppEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (xQueueSend(sAppEventQueue, aEvent, 0) != pdPASS)
    {
        /* Failed to post the message */
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void StartTimer(uint32_t aTimeoutMs)
{
    PLAT_LOG("Start OTA Init Timer")
    if (xTimerIsTimerActive(sOTAInitTimer))
    {
        PLAT_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sOTAInitTimer, pdMS_TO_TICKS(aTimeoutMs), 100) != pdPASS)
    {
        PLAT_LOG("sOTAInitTimer timer start() failed");
    }
}

void CancelTimer(void)
{
    if (xTimerStop(sOTAInitTimer, 0) == pdFAIL)
    {
        PLAT_LOG("sOTAInitTimer stop() failed");
    }
}
#endif

void AppTask::ActionInitiated(PumpManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the pump, update the pump trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == PumpManager::START_ACTION)
    {
        PLAT_LOG("Pump start initiated");
        ; // TODO
    }
    else if (aAction == PumpManager::STOP_ACTION)
    {
        PLAT_LOG("Stop initiated");
        ; // TODO
    }
#if (LED_ENABLE == 1)
    LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppGreenHandle, 50 /* ms */, LED_BLINK_FOREVER);
    LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
    LED_startBlinking(sAppRedHandle, 110 /* ms */, LED_BLINK_FOREVER);
#endif
}

void AppTask::ActionCompleted(PumpManager::Action_t aAction, int32_t aActor)
{
    // if the action has been completed by the pump, update the pump trait.
    // Turn on the pump state LED if in a STARTED state OR
    // Turn off the pump state LED if in an STOPPED state.
    if (aAction == PumpManager::START_ACTION)
    {
        PLAT_LOG("Pump start completed");
#if (LED_ENABLE == 1)
        LED_stopBlinking(sAppGreenHandle);
        LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
        LED_stopBlinking(sAppRedHandle);
        LED_setOn(sAppRedHandle, LED_BRIGHTNESS_MAX);
#endif
    }
    else if (aAction == PumpManager::STOP_ACTION)
    {
        PLAT_LOG("Pump stop completed");
#if (LED_ENABLE == 1)
        LED_stopBlinking(sAppGreenHandle);
        LED_setOff(sAppGreenHandle);
        LED_stopBlinking(sAppRedHandle);
        LED_setOff(sAppRedHandle);
#endif
    }
    if (aActor == AppEvent::kEventType_ButtonLeft)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ButtonRight:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            // Toggle Pump state
            if (!PumpMgr().IsStopped())
            {
                PumpMgr().InitiateAction(0, PumpManager::STOP_ACTION);
            }
            else
            {
                PumpMgr().InitiateAction(0, PumpManager::START_ACTION);
            }
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
#if CHIP_CONFIG_ENABLE_ICD_UAT
            PlatformMgr().ScheduleWork([](intptr_t) { app::ICDNotifier::GetInstance().NotifyNetworkActivityNotification(); });
#endif
        }
        break;
    case AppEvent::kEventType_ButtonLeft:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
            // Toggle BLE advertisements
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
                {
                    PLAT_LOG("Enabled BLE Advertisement");
                }
                else
                {
                    PLAT_LOG("OpenBasicCommissioningWindow() failed");
                }
            }
            else
            {
                // Disable BLE advertisements
                ConnectivityMgr().SetBLEAdvertisingEnabled(false);
                PLAT_LOG("Disabled BLE Advertisements");
            }
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        break;

    case AppEvent::kEventType_IdentifyStart:
        identify_TakeAction();
        PLAT_LOG("Identify started");
        break;

    case AppEvent::kEventType_IdentifyStop:
#if (LED_ENABLE == 1)
        LED_stopBlinking(sAppGreenHandle);

        if (!PumpMgr().IsStopped())
        {
            LED_setOn(sAppGreenHandle, LED_BRIGHTNESS_MAX);
        }
        else
        {
            LED_setOff(sAppGreenHandle);
        }
#endif
        PLAT_LOG("Identify stopped");
        break;

    case AppEvent::kEventType_AppEvent:
        if (NULL != aEvent->Handler)
        {
            aEvent->Handler(aEvent);
        }
        break;

    case AppEvent::kEventType_None:
    default:
        break;
    }
}

void AppTask::UpdateClusterState() {}

void AppTask::IdentifyStartHandler(::Identify *)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_IdentifyStart;
    sAppTask.PostEvent(&event);
}

void AppTask::IdentifyStopHandler(::Identify *)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_IdentifyStop;
    sAppTask.PostEvent(&event);
}

void AppTask::TriggerIdentifyEffectHandler(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        PLAT_LOG("Starting blink identifier effect");
        IdentifyStartHandler(identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        PLAT_LOG("Breathe identifier effect not implemented");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        PLAT_LOG("Okay identifier effect not implemented");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        PLAT_LOG("Channel Change identifier effect not implemented");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        PLAT_LOG("Finish identifier effect not implemented");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        PLAT_LOG("Stop identifier effect");
        IdentifyStopHandler(identify);
        break;
    default:
        PLAT_LOG("No identifier effect");
    }
}

#if (BUTTON_ENABLE == 1)
void AppTask::ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_ButtonLeft;

    if (events & Button_EV_CLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_Clicked;
    }
    else if (events & Button_EV_LONGCLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_LongClicked;
    }
    // button callbacks are in ISR context
    if (xQueueSendFromISR(sAppEventQueue, &event, NULL) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::ButtonRightEventHandler(Button_Handle handle, Button_EventMask events)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_ButtonRight;

    if (events & Button_EV_CLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_Clicked;
    }
    else if (events & Button_EV_LONGCLICKED)
    {
        event.ButtonEvent.Type = AppEvent::kAppEventButtonType_LongClicked;
    }
    // button callbacks are in ISR context
    if (xQueueSendFromISR(sAppEventQueue, &event, NULL) != pdPASS)
    {
        /* Failed to post the message */
    }
}
#endif // BUTTON_ENABLE

void AppTask::uiInit(void)
{
#if (LED_ENABLE == 1)

    LED_Params ledParams;

    // Initialize LEDs
    PLAT_LOG("Initialize LEDs");
    LED_init();

    LED_Params_init(&ledParams); // default PWM LED
    sAppRedHandle = LED_open(CONFIG_LED_RED, &ledParams);
    LED_setOff(sAppRedHandle);

    LED_Params_init(&ledParams); // default PWM LED
    sAppGreenHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    LED_setOff(sAppGreenHandle);
#endif // LED ENABLE

#if (BUTTON_ENABLE == 1)
    Button_Params buttonParams;

    // Initialize buttons
    PLAT_LOG("Initialize buttons");
    Button_init();

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    sAppLeftHandle                 = Button_open(CONFIG_BTN_LEFT, &buttonParams);
    Button_setCallback(sAppLeftHandle, ButtonLeftEventHandler);

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    sAppRightHandle                = Button_open(CONFIG_BTN_RIGHT, &buttonParams);
    Button_setCallback(sAppRightHandle, ButtonRightEventHandler);
#endif // BUTTON ENABLE
}
