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

#include <app/server/Server.h>

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include "SilabsDeviceDataProvider.h"
#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
#include <app/icd/server/ICDNotifier.h> // nogncheck
#endif
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/silabs/ThreadStackManagerImpl.h>
#endif // CHIP_ENABLE_OPENTHREAD

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#ifdef SL_WIFI
#include "wfx_host_events.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#endif // SL_WIFI

#ifdef DIC_ENABLE
#include "dic.h"
#include "dic_control.h"
#endif // DIC_ENABLE

#ifdef PERFORMANCE_TEST_ENABLED
#include <performance_test_commands.h>
#endif // PERFORMANCE_TEST_ENABLED

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE (4096)
#endif
#define APP_TASK_PRIORITY 2
#ifndef APP_EVENT_QUEUE_SIZE // Allow apps to define a different app queue size
#define APP_EVENT_QUEUE_SIZE 10
#endif
#define EXAMPLE_VENDOR_ID 0xcafe

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
#define SYSTEM_STATE_LED 0
#endif // ENABLE_WSTK_LEDS
#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {

/**********************************************************
 * Variable declarations
 *********************************************************/

osTimerId_t sFunctionTimer;
osTimerId_t sLightTimer;
osThreadId_t sAppTaskHandle;
osMessageQueueId_t sAppEventQueue;

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
LEDWidget sStatusLED;
#endif // ENABLE_WSTK_LEDS

#ifdef SL_WIFI
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif /* SL_WIFI */

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
bool sIsEnabled          = false;
bool sIsAttached         = false;
bool sHaveBLEConnections = false;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

constexpr uint32_t kLightTimerPeriod = static_cast<uint32_t>(pdMS_TO_TICKS(10));

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct; // TODO abstract type for static controlblock
constexpr osMessageQueueAttr_t appEventQueueAttr = { .cb_mem  = &sAppEventQueueStruct,
                                                     .cb_size = sizeof(sAppEventQueueBuffer),
                                                     .mq_mem  = sAppEventQueueBuffer,
                                                     .mq_size = sizeof(sAppEventQueueBuffer) };

uint8_t appStack[APP_TASK_STACK_SIZE];
StaticTask_t appTaskStruct; // TODO abstract type for static controlblock
constexpr osThreadAttr_t appTaskAttr = { .name       = APP_TASK_NAME,
                                         .attr_bits  = osThreadDetached,
                                         .cb_mem     = &appTaskStruct,
                                         .cb_size    = sizeof(appTaskStruct),
                                         .stack_mem  = appStack,
                                         .stack_size = APP_TASK_STACK_SIZE,
                                         .priority   = osPriorityNormal };

#ifdef DISPLAY_ENABLED
SilabsLCD slLCD;
#endif

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

Identify gIdentify = {
    chip::EndpointId{ 1 },
    BaseApplication::OnIdentifyStart,
    BaseApplication::OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    BaseApplication::OnTriggerIdentifyEffect,
};

#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER
} // namespace

bool BaseApplication::sIsProvisioned           = false;
bool BaseApplication::sIsFactoryResetTriggered = false;
LEDWidget * BaseApplication::sAppActionLed     = nullptr;
#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917
BaseApplicationDelegate BaseApplication::sAppDelegate = BaseApplicationDelegate();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917

#ifdef DIC_ENABLE
namespace {
void AppSpecificConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    SILABS_LOG("AppSpecificConnectivityEventCallback: call back for IPV4");
    if ((event->Type == DeviceEventType::kInternetConnectivityChange) &&
        (event->InternetConnectivityChange.IPv4 == kConnectivity_Established))
    {
        SILABS_LOG("Got IPv4 Address! Starting DIC module\n");
        if (DIC_OK != dic_init(dic::control::subscribeCB))
            SILABS_LOG("Failed to initialize DIC module\n");
    }
}
} // namespace
#endif // DIC_ENABLE

#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917
void BaseApplicationDelegate::OnCommissioningSessionStarted()
{
    isComissioningStarted = true;
}
void BaseApplicationDelegate::OnCommissioningSessionStopped()
{
    isComissioningStarted = false;
}
void BaseApplicationDelegate::OnCommissioningWindowClosed()
{
    if (!BaseApplication::GetProvisionStatus() && !isComissioningStarted)
    {
        int32_t status = wfx_power_save(RSI_SLEEP_MODE_8, STANDBY_POWER_SAVE_WITH_RAM_RETENTION);
        if (status != SL_STATUS_OK)
        {
            ChipLogError(DeviceLayer, "Failed to enable the TA Deep Sleep");
        }
    }
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI917

/**********************************************************
 * AppTask Definitions
 *********************************************************/

CHIP_ERROR BaseApplication::StartAppTask(osThreadFunc_t taskFunction)
{
    sAppEventQueue = osMessageQueueNew(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), &appEventQueueAttr);
    if (sAppEventQueue == NULL)
    {
        SILABS_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = osThreadNew(taskFunction, &sAppEventQueue, &appTaskAttr);
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
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    SILABS_LOG("APP: Done WiFi Init");
    /* We will init server when we get IP */

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    sWiFiNetworkCommissioningInstance.Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#endif

    // Create cmsis os sw timer for Function Selection.
    sFunctionTimer = osTimerNew(FunctionTimerEventHandler, // timer callback handler
                                osTimerOnce,               // no timer reload (one-shot timer)
                                (void *) this,             // pass the app task obj context
                                NULL                       // No osTimerAttr_t to provide.
    );
    if (sFunctionTimer == NULL)
    {
        SILABS_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    // Create cmsis os sw timer for LED Management.
    sLightTimer = osTimerNew(LightTimerEventHandler, // Timer callback handler"LightTmr",
                             osTimerPeriodic,        // timer repeats automatically
                             (void *) this,          // pass the app task obj context
                             NULL                    // No osTimerAttr_t to provide.
    );
    if (sLightTimer == NULL)
    {
        SILABS_LOG("Light Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    SILABS_LOG("Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    SILABS_LOG("Current Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);

#ifdef DIC_ENABLE
    chip::DeviceLayer::PlatformMgr().AddEventHandler(AppSpecificConnectivityEventCallback, reinterpret_cast<intptr_t>(nullptr));
#endif // DIC_ENABLE

    ConfigurationMgr().LogDeviceConfig();

    OutputQrCode(true /*refreshLCD at init*/);
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
#endif // ENABLE_WSTK_LEDS

#ifdef PERFORMANCE_TEST_ENABLED
    RegisterPerfTestCommands();
#endif // PERFORMANCE_TEST_ENABLED

    PlatformMgr().AddEventHandler(OnPlatformEvent, 0);
#ifdef SL_WIFI
    BaseApplication::sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
    BaseApplication::sIsProvisioned = ConnectivityMgr().IsThreadProvisioned();
#endif
    return err;
}

void BaseApplication::FunctionTimerEventHandler(void * timerCbArg)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = timerCbArg;
    event.Handler            = FunctionEventHandler;
    PostEvent(&event);
}

void BaseApplication::FunctionEventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_Timer);
    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    if (!sIsFactoryResetTriggered)
    {
        StartFactoryResetSequence();
    }
    else
    {
        // The factory reset sequence was in motion. The cancellation window expired.
        // Factory Reset the device now.
#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
        StopStatusLEDTimer();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

        ScheduleFactoryReset();
    }
}

bool BaseApplication::ActivateStatusLedPatterns()
{
    bool isPatternSet = false;
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
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
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    // Identify Patterns have priority over Status patterns
    if (!isPatternSet)
    {
        // Apply different status feedbacks
        if (BaseApplication::sIsProvisioned && sIsEnabled)
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

// TODO Move State Monitoring elsewhere
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
        BaseApplication::sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
        sIsEnabled                      = ConnectivityMgr().IsWiFiStationEnabled();
        sIsAttached                     = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
        sIsEnabled  = ConnectivityMgr().IsThreadEnabled();
        sIsAttached = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */
        sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);

        PlatformMgr().UnlockChipStack();
    }

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if defined(ENABLE_WSTK_LEDS)
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
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
    if (!sIsFactoryResetTriggered)
    {
        ActivateStatusLedPatterns();
    }

    sStatusLED.Animate();
#endif // SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    if (sAppActionLed)
    {
        sAppActionLed->Animate();
    }
#endif // ENABLE_WSTK_LEDS
}

void BaseApplication::ButtonHandler(AppEvent * aEvent)
{
    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        StartFunctionTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
    else
    {
        if (sIsFactoryResetTriggered)
        {
            CancelFactoryResetSequence();
        }
        else
        {
            // The factory reset sequence was not initiated,
            // Press and Release:
            // - Open the commissioning window and start BLE advertisement in fast mode when not  commissioned
            // - Output qr code in logs
            // - Cycle LCD screen
            CancelFunctionTimer();

            AppTask::GetAppTask().UpdateDisplay();

#ifdef SL_WIFI
            if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
            if (!BaseApplication::sIsProvisioned)
#endif /* !SL_WIFI */
            {
                // Open Basic CommissioningWindow. Will start BLE advertisements
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                CHIP_ERROR err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                if (err != CHIP_NO_ERROR)
                {
                    SILABS_LOG("Failed to open the Basic Commissioning Window");
                }
            }
            else
            {
                SILABS_LOG("Network is already provisioned, Ble advertisement not enabled");
#if CHIP_CONFIG_ENABLE_ICD_SERVER
                // Temporarily claim network activity, until we implement a "user trigger" reason for ICD wakeups.
                PlatformMgr().LockChipStack();
                ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
                PlatformMgr().UnlockChipStack();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
            }
        }
    }
}

void BaseApplication::UpdateDisplay()
{
    OutputQrCode(false);
#ifdef DISPLAY_ENABLED
    UpdateLCDStatusScreen();
    slLCD.CycleScreens();
#endif
}

void BaseApplication::CancelFunctionTimer()
{
    if (osTimerStop(sFunctionTimer) == osError)
    {
        SILABS_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void BaseApplication::StartFunctionTimer(uint32_t aTimeoutInMs)
{
    // Starts or restarts the function timer
    if (osTimerStart(sFunctionTimer, pdMS_TO_TICKS(aTimeoutInMs)) != osOK)
    {
        SILABS_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StartFactoryResetSequence()
{
    // Initiate the factory reset sequence
    SILABS_LOG("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

    // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
    // cancel, if required.
    StartFunctionTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

    sIsFactoryResetTriggered = true;
#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StartStatusLEDTimer();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    // Turn off all LEDs before starting blink to make sure blink is
    // co-ordinated.
    sStatusLED.Set(false);
    sStatusLED.Blink(500);
#endif // ENABLE_WSTK_LEDS
}

void BaseApplication::CancelFactoryResetSequence()
{
    CancelFunctionTimer();

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
    StopStatusLEDTimer();
#endif
    if (sIsFactoryResetTriggered)
    {
        sIsFactoryResetTriggered = false;
        SILABS_LOG("Factory Reset has been Canceled");
    }
}

void BaseApplication::StartStatusLEDTimer()
{
    if (osTimerStart(sLightTimer, kLightTimerPeriod) != osOK)
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StopStatusLEDTimer()
{
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    sStatusLED.Set(false);
#endif // ENABLE_WSTK_LEDS

    if (osTimerStop(sLightTimer) == osError)
    {
        SILABS_LOG("Light Time start failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
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
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

void BaseApplication::LightTimerEventHandler(void * timerCbArg)
{
    LightEventHandler();
}

#ifdef DISPLAY_ENABLED
SilabsLCD & BaseApplication::GetLCD(void)
{
    return slLCD;
}

void BaseApplication::UpdateLCDStatusScreen(void)
{
    SilabsLCD::DisplayStatus_t status;
    bool enabled, attached;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
#ifdef SL_WIFI
    enabled  = ConnectivityMgr().IsWiFiStationEnabled();
    attached = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
    enabled  = ConnectivityMgr().IsThreadEnabled();
    attached = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */
    status.connected   = enabled && attached;
    status.advertising = chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen();
    status.nbFabric    = chip::Server::GetInstance().GetFabricTable().FabricCount();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    status.icdMode = (ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT)
        ? SilabsLCD::ICDMode_e::SIT
        : SilabsLCD::ICDMode_e::LIT;
#endif
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    slLCD.SetStatus(status);
}
#endif

void BaseApplication::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != nullptr)
    {
        if (osMessageQueuePut(sAppEventQueue, aEvent, osPriorityNormal, 0) != osOK)
        {
            SILABS_LOG("Failed to post event to app task event queue");
        }
    }
    else
    {
        SILABS_LOG("App Event Queue is uninitialized");
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

void BaseApplication::ScheduleFactoryReset()
{
    PlatformMgr().ScheduleWork([](intptr_t) {
        PlatformMgr().HandleServerShuttingDown();
        ConfigurationMgr().InitiateFactoryReset();
    });
}

void BaseApplication::OnPlatformEvent(const ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceEventType::kServiceProvisioningChange)
    {
        BaseApplication::sIsProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
    }
}

void BaseApplication::OutputQrCode(bool refreshLCD)
{
    (void) refreshLCD; // could be unused

    // Create buffer for the Qr code setup payload that can fit max size and null terminator.
    char setupPayloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan setupPayload(setupPayloadBuffer);

    if (Silabs::SilabsDeviceDataProvider::GetDeviceDataProvider().GetSetupPayload(setupPayload) == CHIP_NO_ERROR)
    {
        // Print setup info on LCD if available
#ifdef QR_CODE_ENABLED
        if (refreshLCD)
        {
            slLCD.SetQRCode((uint8_t *) setupPayload.data(), setupPayload.size());
            slLCD.ShowQRCode(true);
        }
#endif // QR_CODE_ENABLED

        PrintQrCodeURL(setupPayload);
    }
    else
    {
        SILABS_LOG("Getting QR code failed!");
    }
}

bool BaseApplication::GetProvisionStatus()
{
    return BaseApplication::sIsProvisioned;
}
