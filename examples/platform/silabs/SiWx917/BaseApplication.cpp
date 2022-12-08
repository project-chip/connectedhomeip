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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include "SiWx917DeviceDataProvider.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#ifdef SL_WIFI
#include "wfx_host_events.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#endif // SL_WIFI

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE (4096)
#endif
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

using namespace chip;
using namespace ::chip::DeviceLayer;

namespace {

/**********************************************************
 * Variable declarations
 *********************************************************/

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.
TimerHandle_t sLightTimer;

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

#ifdef SL_WIFI
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif /* SL_WIFI */

#if !(defined(CHIP_DEVICE_CONFIG_ENABLE_SED) && CHIP_DEVICE_CONFIG_ENABLE_SED)

bool sIsProvisioned      = false;
bool sIsEnabled          = false;
bool sIsAttached         = false;
bool sHaveBLEConnections = false;

#endif // CHIP_DEVICE_CONFIG_ENABLE_SED

EmberAfIdentifyEffectIdentifier sIdentifyEffect = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

BaseApplication::Function_t mFunction;
bool mFunctionTimerActive;

Identify * gIdentifyptr = nullptr;

#ifdef DISPLAY_ENABLED
SilabsLCD slLCD;
#endif

} // namespace

/**********************************************************
 * AppTask Definitions
 *********************************************************/

CHIP_ERROR BaseApplication::StartAppTask(TaskFunction_t taskFunction)
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == NULL)
    {
        SILABS_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle =
        xTaskCreateStatic(taskFunction, APP_TASK_NAME, ArraySize(appStack), &sAppEventQueue, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        SILABS_LOG("Failed to create app task");
        appError(APP_ERROR_CREATE_TASK_FAILED);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BaseApplication::Init(Identify * identifyObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (identifyObj == nullptr)
    {
        SILABS_LOG("Invalid Identify Object!");
        appError(CHIP_ERROR_INVALID_ARGUMENT);
    }

    gIdentifyptr = identifyObj;

#ifdef SL_WIFI
    /*
     * Wait for the WiFi to be initialized
     */
    SILABS_LOG("APP: Wait WiFi Init");
    while (!wfx_hw_ready())
    {
        vTaskDelay(10);
    }
    SILABS_LOG("APP: Done WiFi Init");
    /* We will init server when we get IP */

    sWiFiNetworkCommissioningInstance.Init();
#endif

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",                  // Just a text name, not used by the RTOS kernel
                                  1,                        // == default timer period (mS)
                                  false,                    // no timer reload (==one-shot)
                                  (void *) this,            // init timer id = app task obj context
                                  FunctionTimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        SILABS_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    // Create FreeRTOS sw timer for LED Management.
    sLightTimer = xTimerCreate("LightTmr",            // Text Name
                               10,                    // Default timer period (mS)
                               true,                  // reload timer
                               (void *) this,         // Timer Id
                               LightTimerEventHandler // Timer callback handler
    );
    if (sLightTimer == NULL)
    {
        SILABS_LOG("Light Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    SILABS_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan QRCode(qrCodeBuffer);

    if (EFR32::EFR32DeviceDataProvider::GetDeviceDataProvider().GetSetupPayload(QRCode) == CHIP_NO_ERROR)
    {
        // Print setup info on LCD if available
#ifdef QR_CODE_ENABLED
        slLCD.SetQRCode((uint8_t *) QRCode.data(), QRCode.size());
        slLCD.ShowQRCode(true, true);
#else
        PrintQrCodeURL(QRCode);
#endif // QR_CODE_ENABLED
    }
    else
    {
        SILABS_LOG("Getting QR code failed!");
    }

    return err;
}

void BaseApplication::FunctionTimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = FunctionEventHandler;
    PostEvent(&event);
}

void BaseApplication::FunctionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (mFunctionTimerActive && mFunction == kFunction_StartBleAdv)
    {
        SILABS_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        StartFunctionTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
        StartStatusLEDTimer();
#endif // CHIP_DEVICE_CONFIG_ENABLE_SED

        mFunction = kFunction_FactoryReset;
    }
    else if (mFunctionTimerActive && mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        mFunction = kFunction_NoneSelected;

#if CHIP_DEVICE_CONFIG_ENABLE_SED == 1
        StopStatusLEDTimer();
#endif // CHIP_DEVICE_CONFIG_ENABLE_SED

        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void BaseApplication::LightEventHandler()
{
    // Collect connectivity and configuration state from the CHIP stack. Because
    // the CHIP event loop is being run in a separate task, the stack must be
    // locked while these values are queried.  However we use a non-blocking
    // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
    // when the CHIP task is busy (e.g. with a long crypto operation).
#if !(defined(CHIP_DEVICE_CONFIG_ENABLE_SED) && CHIP_DEVICE_CONFIG_ENABLE_SED)
    if (PlatformMgr().TryLockChipStack())
    {
#ifdef SL_WIFI
        sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
        sIsEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
        sIsAttached    = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
        sIsProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsEnabled     = ConnectivityMgr().IsThreadEnabled();
        sIsAttached    = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */
        sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
        PlatformMgr().UnlockChipStack();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_SED

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
}

void BaseApplication::CancelFunctionTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        SILABS_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void BaseApplication::StartFunctionTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        SILABS_LOG("app timer already started!");
        CancelFunctionTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        SILABS_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void BaseApplication::StartStatusLEDTimer()
{
    if (pdPASS != xTimerStart(sLightTimer, 0))
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StopStatusLEDTimer()
{

    if (xTimerStop(sLightTimer, 100) != pdPASS)
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::LightTimerEventHandler(TimerHandle_t xTimer)
{
    LightEventHandler();
}

#ifdef DISPLAY_ENABLED
SilabsLCD & BaseApplication::GetLCD(void)
{
    return slLCD;
}
#endif

void BaseApplication::PostEvent(const AppEvent * aEvent)
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
        {
            SILABS_LOG("Failed to post event to app task event queue");
        }
    }
    else
    {
        SILABS_LOG("Event Queue is NULL should never happen");
    }
}

void BaseApplication::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        SILABS_LOG("Event received with no handler. Dropping event.");
    }
}
