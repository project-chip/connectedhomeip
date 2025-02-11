/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <cy_wcm.h>
#include <data-model-providers/codegen/Instance.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Infineon/PSOC6/NetworkCommissioningDriver.h>
#include <static-supported-modes-manager.h>

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
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

#endif
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (4096)
#define APP_WAIT_LOOP 1000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 5000

LEDWidget sStatusLED;
LEDWidget sLightLED;
LEDWidget sClusterLED;

namespace {
TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;
chip::app::Clusters::ModeSelect::StaticSupportedModesManager sStaticSupportedModesManager;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
#endif

} // namespace

using namespace ::chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

AppTask AppTask::sAppTask;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::P6WiFiDriver::GetInstance()));

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    GetAppTask().InitOTARequestor();
#endif
    chip::app::Clusters::ModeSelect::setSupportedModesManager(&sStaticSupportedModesManager);
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == NULL)
    {
        PSOC6_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }
    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, appStack, &appTaskStruct);
    return (sAppTaskHandle == nullptr) ? APP_ERROR_CREATE_TASK_FAILED : CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    int rc = flash_area_boot_set_confirmed();
    if (rc != 0)
    {
        PSOC6_LOG("flash_area_boot_set_confirmed failed");
        appError(CHIP_ERROR_UNINITIALIZED);
    }
#endif
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
    NetWorkCommissioningInstInit();
    PSOC6_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);
    sLightLED.Init(LIGHT_LED);
    sClusterLED.RGB_init();

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        PSOC6_LOG("AppTask.Init() failed");
        appError(err);
    }

    PSOC6_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    /* ON/OFF Light Led based on Button interrupt */
    sLightLED.Invert();

    /* Update OnOff Cluster state */
    chip::DeviceLayer::PlatformMgr().ScheduleWork(OnOffUpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    if (btnIdx != APP_LIGHT_BUTTON_IDX && btnIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnAction;

    if (btnIdx == APP_LIGHT_BUTTON_IDX)
    {
        button_event.Handler = LightActionEventHandler;
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
            sLightLED.Set(false);

            sStatusLED.Blink(500);
            sLightLED.Blink(500);
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
        {

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
            PSOC6_LOG("Failed to post event to app task event queue");
    }
    else
    {
        PSOC6_LOG("Event Queue is NULL should never happen");
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
        PSOC6_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::OnOffUpdateClusterState(intptr_t context)
{
    uint8_t onoff = sLightLED.Get();

    // write the new on/off value
    Protocols::InteractionModel::Status status = app::Clusters::OnOff::Attributes::OnOff::Set(2, onoff);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        PSOC6_LOG("ERR: updating on/off %x", to_underlying(status));
    }
}

bool lowPowerClusterSleep()
{
    return true;
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
