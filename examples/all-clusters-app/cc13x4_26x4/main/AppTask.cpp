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
#include "Globals.h"
#include <app/util/endpoint-config-api.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <examples/platform/cc13x4_26x4/CC13X4_26X4DeviceAttestationCreds.h>

#include <DeviceInfoProviderImpl.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/cc13xx_26xx/OTAImageProcessorImpl.h>
#endif
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

#if AUTO_PRINT_METRICS
// #include <platform/cc13xx_26xx/DiagnosticDataProviderImpl.h>
#endif
#include <app/server/OnboardingCodesUtil.h>

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <src/platform/cc13xx_26xx/DefaultTestEventTriggerDelegate.h>

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* syscfg */
#include <ti_drivers_config.h>

#define APP_TASK_STACK_SIZE (5000)
#define APP_TASK_PRIORITY 4
#define APP_EVENT_QUEUE_SIZE 10
#define BUTTON_ENABLE 1

#define OTAREQUESTOR_INIT_TIMER_DELAY_MS 10000

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;
static Button_Handle sAppLeftHandle;
static Button_Handle sAppRightHandle;
static DeviceInfoProviderImpl sExampleDeviceInfoProvider;

AppTask AppTask::sAppTask;

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

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

    sRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sRequestorCore.Init(Server::GetInstance(), sRequestorStorage, sRequestorUser, sDownloader);
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

#ifdef AUTO_PRINT_METRICS
static void printMetrics(void)
{
    chip::DeviceLayer::ThreadMetrics *threadMetricsOut, *currThread;
    uint64_t heapFree, heapUsed;

    DiagnosticDataProviderImpl::GetDefaultInstance().GetCurrentHeapUsed(heapUsed);
    DiagnosticDataProviderImpl::GetDefaultInstance().GetCurrentHeapFree(heapFree);
    DiagnosticDataProviderImpl::GetDefaultInstance().GetThreadMetrics(&threadMetricsOut);

    PLAT_LOG("Heap Metrics\n Heap Free: %d Heap Used: %d", (uint32_t) heapFree, (uint32_t) heapUsed);

    PLAT_LOG("Thread Metrics\n");

    currThread = threadMetricsOut;
    while (currThread)
    {
        PLAT_LOG("Task Name: %s ID: %d Stack Free Min: %d", currThread->name, (uint32_t) currThread->id,
                 (uint32_t) currThread->stackFreeMinimum);

        currThread = currThread->Next;
    }

    DiagnosticDataProviderImpl::GetDefaultInstance().ReleaseThreadMetrics(threadMetricsOut);
}
#endif

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

#ifdef AUTO_PRINT_METRICS
    printMetrics();
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void OTAInitTimerEventHandler(TimerHandle_t xTimer)
{
    InitializeOTARequestor();
}
#endif

int AppTask::StartAppTask()
{
    int ret = 0;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PLAT_LOG("Failed to allocate app event queue");
        while (1)
            ;
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle) !=
        pdPASS)
    {
        PLAT_LOG("Failed to create app task");
        while (1)
            ;
    }
    return ret;
}

int AppTask::Init()
{
    cc13xx_26xxLogInit();

    uiInit();

    // Init Chip memory management before the stack
    Platform::MemoryInit();

    PLAT_LOG("Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PLAT_LOG("Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().InitChipStack() failed");
        while (1)
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
        while (1)
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
        while (1)
            ;
    }

    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("ThreadStackMgr().StartThreadTask() failed");
        while (1)
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

    // Initialize info provider
    sExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    SetDeviceInfoProvider(&sExampleDeviceInfoProvider);

    chip::Server::GetInstance().Init(initParams);

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        PLAT_LOG("PlatformMgr().StartEventLoopTask() failed");
        while (1)
            ;
    }

    ConfigurationMgr().LogDeviceConfig();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(DeviceEventCallback, reinterpret_cast<intptr_t>(nullptr));

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
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));

    return 0;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    sAppTask.Init();

    while (1)
    {
        /* Task pend until we have stuff to do */
        if (xQueueReceive(sAppEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
    }
}

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

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (xQueueSend(sAppEventQueue, aEvent, 0) != pdPASS)
    {
        /* Failed to post the message */
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ButtonLeft:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        break;

    case AppEvent::kEventType_ButtonRight:
        if (AppEvent::kAppEventButtonType_Clicked == aEvent->ButtonEvent.Type)
        {
        }
        else if (AppEvent::kAppEventButtonType_LongClicked == aEvent->ButtonEvent.Type)
        {
            // Enable BLE advertisements
            if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                if (Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
                {
                    PLAT_LOG("Enabled BLE Advertisements");
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
