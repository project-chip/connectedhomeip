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

#include "LEDWidget.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include "SilabsDeviceDataProvider.h"
#include "rsi_board.h"
#include "rsi_chip.h"
#include "silabs_utils.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#ifdef SL_WIFI
#include "wfx_host_events.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#endif // SL_WIFI

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define FACTORY_RESET_TRIGGER_TIMEOUT 7000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define FACTORY_RESET_LOOP_COUNT 5
#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE (4096)
#endif
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe
#define APP_STATE_LED 0

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {

/**********************************************************
 * Variable declarations
 *********************************************************/

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.
TimerHandle_t sLightTimer;

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;

#ifdef SL_WIFI
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif /* SL_WIFI */

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)

bool sIsProvisioned      = false;
bool sIsEnabled          = false;
bool sIsAttached         = false;
bool sHaveBLEConnections = false;

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

BaseApplication::Function_t mFunction;
bool mFunctionTimerActive;

#ifdef DISPLAY_ENABLED
SilabsLCD slLCD;
#endif

#ifdef EMBER_AF_PLUGIN_IDENTIFY_SERVER
Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

Identify gIdentify = {
    chip::EndpointId{ 1 },
    BaseApplication::OnIdentifyStart,
    BaseApplication::OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    BaseApplication::OnTriggerIdentifyEffect,
};

#endif // EMBER_AF_PLUGIN_IDENTIFY_SERVER
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

CHIP_ERROR BaseApplication::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    sWiFiNetworkCommissioningInstance.Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#endif

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",                  // Just a text name, not used by the RTOS kernel
                                  pdMS_TO_TICKS(1),         // == default timer period
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
                               pdMS_TO_TICKS(10),     // Default timer period
                               true,                  // reload timer
                               (void *) this,         // Timer Id
                               LightTimerEventHandler // Timer callback handler
    );
    if (sLightTimer == NULL)
    {
        SILABS_LOG("Light Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    SILABS_LOG("Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    SILABS_LOG("Current Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);

    sStatusLED.Init(APP_STATE_LED);

    ConfigurationMgr().LogDeviceConfig();

    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan QRCode(qrCodeBuffer);

    if (Silabs::SilabsDeviceDataProvider::GetDeviceDataProvider().GetSetupPayload(QRCode) == CHIP_NO_ERROR)
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
}

void BaseApplication::FunctionFactoryReset(void)
{
    SILABS_LOG("#################################################################");
    SILABS_LOG("################### Factory reset triggered #####################");
    SILABS_LOG("#################################################################");

    // Actually trigger Factory Reset
    mFunction = kFunction_NoneSelected;

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StopStatusLEDTimer();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    chip::Server::GetInstance().ScheduleFactoryReset();
}

bool BaseApplication::ActivateStatusLedPatterns()
{
    bool isPatternSet = false;
#if defined(ENABLE_WSTK_LEDS) && defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)
#ifdef EMBER_AF_PLUGIN_IDENTIFY_SERVER
    if (gIdentify.mActive)
    {
        // Identify in progress
        // Do a steady blink on the status led
        sStatusLED.Blink(250, 250);
        isPatternSet = true;
    }
    else if (sIdentifyEffect != Clusters::Identify::EffectIdentifierEnum::kStopEffect)
    {
        // Identify trigger effect received. Do some on/off patterns on the status led
        if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kBlink)
        {
            // Fast blink
            sStatusLED.Blink(50, 50);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kBreathe)
        {
            // Slow blink
            sStatusLED.Blink(1000, 1000);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kOkay)
        {
            // Pulse effect
            sStatusLED.Blink(300, 700);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kChannelChange)
        {
            // Alternate between Short and Long pulses effect
            static uint64_t mLastChangeTimeMS = 0;
            static bool alternatePattern      = false;
            uint32_t onTimeMS                 = alternatePattern ? 50 : 700;
            uint32_t offTimeMS                = alternatePattern ? 950 : 300;

            uint64_t nowMS = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
            if (nowMS >= mLastChangeTimeMS + 1000) // each pattern is done over a 1 second period
            {
                mLastChangeTimeMS = nowMS;
                alternatePattern  = !alternatePattern;
                sStatusLED.Blink(onTimeMS, offTimeMS);
            }
        }
        isPatternSet = true;
    }
#endif // EMBER_AF_PLUGIN_IDENTIFY_SERVER

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    // Identify Patterns have priority over Status patterns
    if (!isPatternSet)
    {
        // Apply different status feedbacks
        if (sIsProvisioned && sIsEnabled)
        {
            if (sIsAttached)
            {
                sStatusLED.Set(true);
            }
            else
            {
                sStatusLED.Blink(950, 50);
            }
        }
        else if (sHaveBLEConnections)
        {
            sStatusLED.Blink(100, 100);
        }
        else
        {
            sStatusLED.Blink(50, 950);
        }
        isPatternSet = true;
    }
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // ENABLE_WSTK_LEDS) && SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    return isPatternSet;
}

void BaseApplication::LightEventHandler()
{
    // Collect connectivity and configuration state from the CHIP stack. Because
    // the CHIP event loop is being run in a separate task, the stack must be
    // locked while these values are queried.  However we use a non-blocking
    // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
    // when the CHIP task is busy (e.g. with a long crypto operation).
#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
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
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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
    if (mFunction != kFunction_FactoryReset)
    {
        ActivateStatusLedPatterns();
    }
    sStatusLED.Animate();
}

void BaseApplication::ButtonHandler(AppEvent * aEvent)
{
    uint8_t count = FACTORY_RESET_LOOP_COUNT;

    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        if ((!mFunctionTimerActive) && (mFunction == kFunction_NoneSelected))
        {
            mFunction = kFunction_FactoryReset;

            // Wait for sometime to determine button is pressed for Factory reset
            // other functionality
            vTaskDelay(1000); // Delay of 1sec before we check the button status
        }
    }

    while (!(RSI_NPSSGPIO_GetPin(NPSS_GPIO_0)))
    {
        if (count == 0)
        {
            FunctionFactoryReset();
            break;
        }

        // Turn off status LED before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
        SILABS_LOG("Factory reset triggering in %d sec release button to cancel", count--);

        // Delay of 1sec before checking the button status again
        vTaskDelay(1000);
    }

    if (count > 0)
    {
        sStatusLED.Set(false);
        SILABS_LOG("Factory Reset has been Canceled"); // button held past Timeout wait till button is released
    }

    // If the button was released before factory reset got initiated, start BLE advertissement in fast mode
    if (mFunction == kFunction_FactoryReset)
    {
        mFunction = kFunction_NoneSelected;

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
        StopStatusLEDTimer();
#endif

        if (!ConnectivityMgr().IsWiFiStationProvisioned())
        {
            // Enable BLE advertisements
            ConnectivityMgr().SetBLEAdvertisingEnabled(true);
            ConnectivityMgr().SetBLEAdvertisingMode(ConnectivityMgr().kFastAdvertising);
        }
        else
        {
            SILABS_LOG("Network is already provisioned, Ble advertissement not enabled");
        }
    }
}

void BaseApplication::CancelFunctionTimer()
{
    if (xTimerStop(sFunctionTimer, pdMS_TO_TICKS(0)) == pdFAIL)
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
    // FreeRTOS- Block for a maximum of 100 ms if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, pdMS_TO_TICKS(aTimeoutInMs), pdMS_TO_TICKS(100)) != pdPASS)
    {
        SILABS_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void BaseApplication::StartStatusLEDTimer()
{
    if (pdPASS != xTimerStart(sLightTimer, pdMS_TO_TICKS(0)))
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StopStatusLEDTimer()
{
    sStatusLED.Set(false);

    if (xTimerStop(sLightTimer, pdMS_TO_TICKS(100)) != pdPASS)
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

#ifdef EMBER_AF_PLUGIN_IDENTIFY_SERVER
void BaseApplication::OnIdentifyStart(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStart");

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StartStatusLEDTimer();
#endif
}

void BaseApplication::OnIdentifyStop(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStop");

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StopStatusLEDTimer();
#endif
}

void BaseApplication::OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "Trigger Identify Complete");
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StopStatusLEDTimer();
#endif
}

void BaseApplication::OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mEffectVariant != Clusters::Identify::EffectVariantEnum::kDefault)
    {
        ChipLogDetail(AppServer, "Identify Effect Variant unsupported. Using default");
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StartStatusLEDTimer();
#endif

    switch (sIdentifyEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(10), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        break;
    default:
        sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        ChipLogProgress(Zcl, "No identifier effect");
    }
}
#endif // EMBER_AF_PLUGIN_IDENTIFY_SERVER

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
